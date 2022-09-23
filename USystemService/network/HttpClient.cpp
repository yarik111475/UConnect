#include <QDir>
#include <QFile>
#include <QSslKey>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QUrlQuery>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QNetworkAccessManager>

#include "HttpClient.h"
#include "tasks/TaskQueue.h"
#include "spdlog/spdlog.h"

HttpClient::HttpClient(const QUrl &http_url, int http_port, const QString api_path, const QString &app_dir_path, std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const std::string &log_name, QObject *parent)
    :QObject{parent},m_http_url{http_url}, m_http_port{http_port}, m_api_path{api_path}, m_app_dir_path{app_dir_path}, m_ssl_configuration_ptr{ssl_configuration_ptr}, m_log_name{log_name}
{
    init_machine_uid();
    m_task_queue_ptr=new TaskQueue(m_ssl_configuration_ptr, std::shared_ptr<HttpClient>(this), m_app_dir_path, m_log_name);

    m_task_thread_ptr=new QThread;
    QObject::connect(m_task_thread_ptr, &QThread::started, m_task_queue_ptr, &TaskQueue::start);
    QObject::connect(m_task_thread_ptr, &QThread::finished, m_task_queue_ptr, &TaskQueue::deleteLater);

    m_task_queue_ptr->moveToThread(m_task_thread_ptr);
    m_task_thread_ptr->start();

    m_logger_ptr=spdlog::get(m_log_name);
}

bool HttpClient::init_machine_uid()
{
    QString client_cert_path {};
    QString client_cert_name {"clientCert.pem"};

#ifdef Q_OS_WINDOWS
    client_cert_path=qEnvironmentVariable("programfiles(x86)") + "/USystem/agentServer/etc/usagent/cert";
#endif
#ifdef Q_OS_UNIX
#endif

    QFile in(client_cert_path + "/" + client_cert_name);
    if(in.open(QIODevice::ReadOnly)){
        QSslCertificate client_cert(&in);
        in.close();
        if(!client_cert.isNull()){
            QStringList subject_info_list=client_cert.subjectInfo(QSslCertificate::CommonName);
            if(!subject_info_list.empty()){
                m_uid=QString("%1").arg(subject_info_list.at(0));

                /*
                QUrlQuery query;
                query.addQueryItem("id", m_uid);
                m_hostUrl.setQuery(query);
                */
                return true;
            }
        }
    }
    return false;
}

void HttpClient::start()
{
    m_timer.setInterval(m_interval);
    QObject::connect(&m_timer, &QTimer::timeout,this,&HttpClient::slot_timeout);
    m_timer.start();

    if(m_logger_ptr){
        m_logger_ptr->debug("http client started with uid: {}", m_uid.toStdString());
    }
}

void HttpClient::stop()
{
    m_timer.stop();
}

std::vector<TaskItem> HttpClient::parse_incoming_json(const QJsonObject &incoming_json)
{
    std::vector<TaskItem> task_list {};

    if(!incoming_json.contains("items")){
        return task_list;
    }

    QJsonArray root_json_array=incoming_json.value("items").toArray();
    if(root_json_array.empty()){
        return task_list;
    }
    for(const QJsonValue& root_json_item: root_json_array){
        if(root_json_item.toObject().contains("id") && root_json_item.toObject().contains("name") && root_json_item.toObject().contains("additional_params")){

            TaskItem task_item_obj(root_json_item.toObject().value("id").toString(),root_json_item.toObject().value("name").toString());
            task_item_obj.setType(task_item_obj.m_name=="download" ? TaskItem::DOWNLOAD : task_item_obj.m_name=="executefile" ?
                                                               TaskItem::EXECUTE_FILE : TaskItem::EXECUTE_PROGRAMM);

            QJsonObject params_json=root_json_item.toObject().value("additional_params").toObject();
            TaskParamsItem task_params_item_obj(params_json.value("url").toString(),params_json.value("hash").toString(),
                                             params_json.value("filename").toString(),params_json.value("programm").toString());

            QJsonArray arguments_json=params_json.value("arguments").toArray();
            for(const QJsonValue& argument : arguments_json){
                task_params_item_obj.m_arguments.push_back(argument.toString());
            }

            task_item_obj.set_params_item(task_params_item_obj);
            task_list.push_back(task_item_obj);
        }
    }
    return task_list;
}

void HttpClient::slot_timeout()
{
    QUrl url {m_http_url};
    url.setPort(m_http_port);
    url.setPath(m_api_path);

    QEventLoop eventLoop {};
    QNetworkAccessManager* accessmanager_ptr{new QNetworkAccessManager()};
    QObject::connect(accessmanager_ptr, &QNetworkAccessManager::sslErrors,[this](QNetworkReply* reply, const QList<QSslError>& errors){
        Q_UNUSED(errors)
        try{
            reply->ignoreSslErrors();
        }
        catch(...){}
    });
    QObject::connect(accessmanager_ptr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    QObject::connect(accessmanager_ptr,&QNetworkAccessManager::finished, accessmanager_ptr,&QNetworkAccessManager::deleteLater);

    QNetworkRequest request;
    request.setUrl(url);
    request.setSslConfiguration(*m_ssl_configuration_ptr.get());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply=accessmanager_ptr->get(request);
    eventLoop.exec();

    if(reply->error()==QNetworkReply::NoError){
        //get all tasks from backend
        QByteArray content=reply->readAll();
        QJsonObject incoming_json=QJsonDocument::fromJson(content).object();
        std::vector<TaskItem> incoming_task_list=parse_incoming_json(incoming_json);

        //change status for all tasks from 'Created' to 'Progress'"
        for(const TaskItem& task_item: incoming_task_list){
            QJsonObject outcoming_json{};
            outcoming_json["result"]="Progress";
            slot_task_executed(task_item.m_id, outcoming_json);
        }

        //push all tasks from incoming json into task queue
        m_task_queue_ptr->push_task_list(incoming_task_list);
    }
    else{
        if(m_logger_ptr){
            m_logger_ptr->debug("http client network reply error: {} in method slotTimeout()",
                               reply->errorString().toStdString());
        }
    }
}

void HttpClient::slot_task_executed(const QString task_id, const QJsonObject &outcoming_json)
{
    QUrl url {m_http_url};
    url.setPort(m_http_port);
    QString response_api_path {QString(m_api_path + "/%1/result").arg(task_id)};
    url.setPath(response_api_path);

    QEventLoop eventLoop {};
    QNetworkAccessManager* accessmanager_ptr(new QNetworkAccessManager());
    QObject::connect(accessmanager_ptr, &QNetworkAccessManager::sslErrors,[this](QNetworkReply* reply, const QList<QSslError>& errors){
        Q_UNUSED(errors)
        try{
            reply->ignoreSslErrors();
        }
        catch(...){}
    });
    QObject::connect(accessmanager_ptr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    QObject::connect(accessmanager_ptr,&QNetworkAccessManager::finished, accessmanager_ptr,&QNetworkAccessManager::deleteLater);

    QNetworkRequest request;
    request.setUrl(url);
    request.setSslConfiguration(*m_ssl_configuration_ptr.get());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply=accessmanager_ptr->put(request, QJsonDocument(outcoming_json).toJson());
    eventLoop.exec();

    if(reply->error()==QNetworkReply::NoError){
        if(m_logger_ptr){
            m_logger_ptr->debug("status for task with id: {} changed to: {}",
                               task_id.toStdString(), outcoming_json["result"].toString().toStdString());
        }
    }
    else{
        if(m_logger_ptr){
            m_logger_ptr->debug("http client network reply error: {} in method 'slotTaskExecuted()'",
                               reply->errorString().toStdString());
        }
    }
}
