#include <string>
#include <thread>
#include <future>

#include <QUuid>
#include <QFile>
#include <QSslKey>
#include <QSettings>
#include <QSslCertificate>
#include <QSslConfiguration>

#include "Bootstrap.h"
#include "task/Task.h"
#include "task/TaskQueue.h"
#include "task/TaskExecutor.h"

#include "network/HttpClientTask.h"

bool Bootstrap::init_ssl_configuration()
{
    if(logger_ptr_){
        logger_ptr_->info("start to init ssl configuration");
    }

    bool success {false};
    std::shared_ptr<QSslConfiguration> ssl_configuration_ptr=std::make_shared<QSslConfiguration>(QSslConfiguration::defaultConfiguration());
    const QString ca_file_name=app_settings_ptr_->value("client/caCertPath").toString();
    const QString key_file_name=app_settings_ptr_->value("client/clientPrivateKey").toString();
    const QString cert_file_name=app_settings_ptr_->value("client/clientCert").toString();

    try{
        ssl_configuration_ptr->setProtocol(QSsl::AnyProtocol);
        ssl_configuration_ptr->setPeerVerifyMode(QSslSocket::VerifyPeer);

        //init certificates list
        QFile file_ca(ca_file_name);
        if(file_ca.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&file_ca);
            ssl_configuration_ptr->setCaCertificates(listOfCert);
        }

        //init private key
        QFile file_key(key_file_name);
        if(file_key.open(QIODevice::ReadOnly)){
            QSslKey key (&file_key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey,QByteArray());
            ssl_configuration_ptr->setPrivateKey(key);
        }

        //init client certificates list
        QFile file_crt(cert_file_name);
        if(file_crt.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&file_crt);
            ssl_configuration_ptr->setLocalCertificateChain(listOfCert);
            success=true;
        }
    }
    catch(std::exception& ex){
        ssl_configuration_ptr_.reset();
        if(logger_ptr_){
            logger_ptr_->warn("init ssl configuration error: {}", ex.what());
        }
    }
    catch(...){
        ssl_configuration_ptr_.reset();
        if(logger_ptr_){
            logger_ptr_->warn("init ssl configuration unknown error");
        }
    }
    if(logger_ptr_){
        logger_ptr_->info("end to init ssl configuration, result: {}", ssl_configuration_ptr!=nullptr);
    }

    ssl_configuration_ptr_=std::move(ssl_configuration_ptr);
    ssl_configuration_ptr.reset();
    return success;
}

Bootstrap::Bootstrap(std::string& log_name, const QString &app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent)
    : QObject(parent), log_name_{log_name}, app_dir_path_{app_dir_path}, app_settings_ptr_{app_settings_ptr}
{
    logger_ptr_=spdlog::get(log_name_);
}

void Bootstrap::run()
{
    if(!init_ssl_configuration()){
        if(logger_ptr_){
            logger_ptr_->warn("fail to init ssl configuration");
        }
        return;
    }

    QJsonObject init_json{};
    init_json["http_url"]=app_settings_ptr_->value("remote_server/address", "").toString();
    init_json["http_port"]=app_settings_ptr_->value("remote_server/port", 443).toInt();
    init_json["api_path"]="/api/v1/tasks/";

    task_queue_ptr_=new TaskQueue(log_name_);
    http_client_task_ptr_=new HttpClientTask(log_name_, app_dir_path_, app_settings_ptr_);
    http_client_task_ptr_->init(init_json);
    http_client_task_ptr_->set_ssl_configuration(ssl_configuration_ptr_);

    //TaskQueue <> this signal/slot connections (for start task executor in new thread)
    QObject::connect(task_queue_ptr_, &TaskQueue::signal_dequeue_task, this, &Bootstrap::slot_execute_task);

    //HttpClient <> this signal/slot connections (make for translate signal into TaskQueue)
    QObject::connect(http_client_task_ptr_, &HttpClientTask::signal_incoming_task_list, this, &Bootstrap::slot_enqueue_task_list);

    //init new thread for task queue and make signal <> slot connections
    std::thread thread(&TaskQueue::slot_start, task_queue_ptr_);
    thread.detach();

    http_client_task_ptr_->start();
}

void Bootstrap::slot_execute_task(const TaskItem &task_item)
{
    TaskExecutor* task_executor_ptr=new TaskExecutor(log_name_);
    task_executor_ptr->init_executor(task_item);
//    std::future<QJsonObject> future=std::async(std::launch::async,&TaskExecutor::execute_task, task_executor_ptr, task_item);
//    future.wait();
//    QJsonObject outcoming_json=future.get();
//    m_http_client_task_ptr->slot_task_executed(task_item,outcoming_json);


    //TaskExecutor <> HttpClient signal/slot connections
    QObject::connect(task_executor_ptr, &TaskExecutor::signal_task_executed, http_client_task_ptr_, &HttpClientTask::slot_task_executed);
    std::thread thread(&TaskExecutor::slot_execute_task, task_executor_ptr);
    thread.detach();
}

void Bootstrap::slot_enqueue_task_list(const std::vector<TaskItem> &incoming_task_list)
{
    task_queue_ptr_->slot_enqueue_task_list(incoming_task_list);
}
