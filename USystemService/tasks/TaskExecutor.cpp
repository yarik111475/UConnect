#include <QDir>
#include <QFile>
#include <QUrl>
#include <QProcess>
#include <QSslError>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSharedPointer>
#include <QSslConfiguration>
#include <QNetworkAccessManager>
#include <QCryptographicHash>

#include "TaskExecutor.h"
#include "spdlog/spdlog.h"

void TaskExecutor::download(const TaskItem &incoming_task)
{
    QJsonObject outcoming_json {};
    bool dir_created=QDir{m_app_dir_path}.mkpath("../downloads");

    try{
        if(incoming_task.m_params_item.m_url.isEmpty()){
            if(m_logger_ptr){
                m_logger_ptr->debug("field 'url' for task with id: {} is empty!",
                                   incoming_task.m_id.toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("field 'url' for task with id: %1 is empty!").arg(incoming_task.m_id);
            emit signal_task_executed(incoming_task.m_id, outcoming_json);
            return;
        }

        if(incoming_task.m_params_item.m_filename.isEmpty()){
            if(m_logger_ptr){
                m_logger_ptr->debug("field 'filename' for task with id: {} is empty!",
                                   incoming_task.m_id.toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("field 'filename' for task with id: %1 is empty!").arg(incoming_task.m_id);
            emit signal_task_executed(incoming_task.m_id, outcoming_json);
            return;
        }

        m_file_name=incoming_task.m_params_item.m_filename;
        m_hash=incoming_task.m_params_item.m_hash;

        QUrl url(incoming_task.m_params_item.m_url);

        QEventLoop event_loop{};
        QNetworkAccessManager* accessmanager_ptr=new QNetworkAccessManager;
        QObject::connect(accessmanager_ptr, &QNetworkAccessManager::sslErrors,[this](QNetworkReply* reply, const QList<QSslError>& errors){
            Q_UNUSED(errors)
            try{
                reply->ignoreSslErrors();
            }
            catch(...){
            }
        });
        QObject::connect(accessmanager_ptr, &QNetworkAccessManager::finished, &event_loop, &QEventLoop::quit);
        QObject::connect(accessmanager_ptr, &QNetworkAccessManager::finished, accessmanager_ptr, &QNetworkAccessManager::deleteLater);

        QNetworkRequest request;
        request.setUrl(url);
        request.setSslConfiguration(*m_ssl_configuration_ptr.get());

        QNetworkReply* reply=accessmanager_ptr->get(request);
        event_loop.exec();

        if(reply->error()==QNetworkReply::NoError){
            QByteArray content=reply->readAll();

            //check if hash for incoming content and real hash success
            QString content_hash =QString(QCryptographicHash::hash(content,QCryptographicHash::Md5).toHex());
            if(content_hash!=incoming_task.m_params_item.m_hash){
                if(m_logger_ptr){
                    m_logger_ptr->debug("hashes not equal, incoming content hash: {}, real hash: {}",
                                       content_hash.toStdString(), incoming_task.m_params_item.m_hash.toStdString());
                }
                outcoming_json["result"]="Error";
                outcoming_json["data"]=QString("hashes not equal, incoming content hash: %1, real hash: %2").
                                               arg(content_hash).arg(incoming_task.m_params_item.m_hash);
                emit signal_task_executed(incoming_task.m_id, outcoming_json);
                return;
            }

            QFile out(dir_created ? m_app_dir_path + "/../downloads/" + m_file_name : m_app_dir_path + "/" + m_file_name);
            if(out.open(QIODevice::WriteOnly)){
                out.write(content.data(),content.size());
                out.close();

                //file save success
                if(m_logger_ptr){
                    m_logger_ptr->debug("save content as file with name: {} success", m_file_name.toStdString());
                }
                outcoming_json["result"]="Success";
                outcoming_json["data"]=QString("save content as file with name:: %1 success").arg(m_file_name);
                emit signal_task_executed(incoming_task.m_id, outcoming_json);
                return;

            }
            else{//file save error
                if(m_logger_ptr){
                    m_logger_ptr->debug("save content as file with name: {} error with result: {}",
                                       m_file_name.toStdString(), out.errorString().toStdString());
                }
                outcoming_json["result"]="Error";
                outcoming_json["data"]=QString("save content as file with name: %1 error with result: %2").
                        arg(m_file_name).arg(out.errorString());
                emit signal_task_executed(incoming_task.m_id, outcoming_json);
                return;
            }
        }
        else{//network reply error
            if(m_logger_ptr){
                m_logger_ptr->debug("network reply error with result: {}", reply->errorString().toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("network reply error with result: %1").arg(reply->errorString());
        }
    }
    catch(std::exception& ex){//catch std::exception
        if(m_logger_ptr){
            m_logger_ptr->debug("task with id: {} for content file with name: {} finished with error: {}",
                               incoming_task.m_id.toStdString(), m_file_name.toStdString(), ex.what());
        }
        outcoming_json["result"]="Error";
        outcoming_json["data"]=QString("task with id: %1 for content file with name: %2 finished with error: %3").
                arg(incoming_task.m_id, m_file_name, ex.what());
    }
    catch(...){//catch all exceptions
        if(m_logger_ptr){
            m_logger_ptr->debug("task with id: {} for content file with name: {} finished with unlnown error",
                               incoming_task.m_id.toStdString(), m_file_name.toStdString());
        }
        outcoming_json["result"]="Error";
        outcoming_json["data"]=QString("task with id: %1 for content file with name: %2 finished with unlnown error").
                arg(incoming_task.m_id, m_file_name);
    }
    emit signal_task_executed(incoming_task.m_id, outcoming_json);
}

void TaskExecutor::execute_file(const TaskItem &incoming_task)
{
    QStringList list_arg;
    QString     filename;
    QJsonObject outcoming_json {};
    bool dir_created=QDir{m_app_dir_path}.mkpath("../downloads");

    try{
        //set filename
        if(incoming_task.m_params_item.m_filename.isEmpty()){
            if(m_logger_ptr){
                m_logger_ptr->debug("field 'filename' for task with id: {} is empty!",
                                   incoming_task.m_id.toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("field 'programm' for task with id: %1 is empty!").arg(incoming_task.m_id);
            emit signal_task_executed(incoming_task.m_id, outcoming_json);
            return;
        }
        filename=incoming_task.m_params_item.m_filename;

        //set arguments (if presents)
        if(!incoming_task.m_params_item.m_arguments.empty()){
            list_arg.append(incoming_task.m_params_item.m_arguments);
        }

        QString programm {dir_created ? m_app_dir_path + "/../downloads/" + filename : m_app_dir_path + "/" + filename};

        m_process_ptr=new QProcess{};
        m_process_ptr->start(programm, list_arg);
        m_process_ptr->waitForFinished(-1);

        outcoming_json["result"]=m_process_ptr->error()==QProcess::UnknownError ? "Success" : "Error";
        outcoming_json["exitcode"]=m_process_ptr->exitCode();
        outcoming_json["exitstatus"]=m_process_ptr->exitStatus();
        outcoming_json["stdout"]=QString(m_process_ptr->readAllStandardOutput());
        outcoming_json["stderr"]=QString(m_process_ptr->readAllStandardError());
        outcoming_json["data"]=m_process_ptr->error()==QProcess::UnknownError ? "" : m_process_ptr->errorString();

        if(m_logger_ptr){
            m_logger_ptr->debug("process {} for task with id: {} finished with results:\n {}",
                                m_process_ptr->program().toStdString(), incoming_task.m_id.toStdString(),
                                QString(QJsonDocument(outcoming_json).toJson()).toStdString());
        }

      emit signal_task_executed(incoming_task.m_id, outcoming_json);
      m_process_ptr->deleteLater();
      return;
    }
    catch(std::exception& ex){//catch std::exception
            if(m_logger_ptr){
                m_logger_ptr->debug("task with id: {} for process: {} finished with error: {}",
                                   incoming_task.m_id.toStdString(), m_process_ptr->program().toStdString(),ex.what());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("task with id: %1 for process: %2 finished with error: %3").
                    arg(incoming_task.m_id,m_process_ptr->program(),ex.what());
    }
    catch(...){//catch all exceptions
            if(m_logger_ptr){
                m_logger_ptr->debug("task with id: {} for process: {} finished with unknown error",
                                   incoming_task.m_id.toStdString(), m_process_ptr->program().toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("task with id: %1 for process: %2 finished with unknown error").
                    arg(incoming_task.m_id,m_process_ptr->program());
    }
    emit signal_task_executed(incoming_task.m_id, outcoming_json);
    m_process_ptr->deleteLater();
}

void TaskExecutor::execute_programm(const TaskItem &incoming_task)
{
    QStringList list_arg;
    QString     programm;
    QJsonObject outcoming_json {};

    try{
        //set programm
        if(incoming_task.m_params_item.m_programm.isEmpty()){
            if(m_logger_ptr){
                m_logger_ptr->debug("field 'programm' for task with id: {} is empty!",
                              incoming_task.m_id.toStdString());
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]=QString("field 'programm' for task with id: %1 is empty!").arg(incoming_task.m_id);
            emit signal_task_executed(incoming_task.m_id, outcoming_json);
            return;
        }
        programm=incoming_task.m_params_item.m_programm;

        //set arguments (if presents)
        if(!incoming_task.m_params_item.m_arguments.empty()){
            list_arg.append(incoming_task.m_params_item.m_arguments);
        }


        m_process_ptr=new QProcess{};
        m_process_ptr->start(programm, list_arg);
        m_process_ptr->waitForFinished(-1);

        outcoming_json["result"]=m_process_ptr->error()==QProcess::UnknownError ? "Success" : "Error";
        outcoming_json["exitcode"]=m_process_ptr->exitCode();
        outcoming_json["exitstatus"]=m_process_ptr->exitStatus();
        outcoming_json["stdout"]=QString(m_process_ptr->readAllStandardOutput());
        outcoming_json["stderr"]=QString(m_process_ptr->readAllStandardError());
        outcoming_json["data"]=m_process_ptr->error()==QProcess::UnknownError ? "" : m_process_ptr->errorString();

        if(m_logger_ptr){
            m_logger_ptr->debug("process {} for task with id: {} finished with results:\n {}",
                                m_process_ptr->program().toStdString(), incoming_task.m_id.toStdString(),
                                QString(QJsonDocument(outcoming_json).toJson()).toStdString());
        }

        emit signal_task_executed(incoming_task.m_id, outcoming_json);
        m_process_ptr->deleteLater();
        return;
    }
    catch(std::exception& ex){//catch std::exception
        if(m_logger_ptr){
            m_logger_ptr->debug("task with id: {} for process: {} finished with error: {}",
                          incoming_task.m_id.toStdString(), m_process_ptr->program().toStdString(),ex.what());
        }
        outcoming_json["result"]="Error";
        outcoming_json["data"]=QString("task with id: %1 for process: %2 finished with error: %3").
                                       arg(incoming_task.m_id,m_process_ptr->program(),ex.what());
    }
    catch(...){//catch all exceptions
        if(m_logger_ptr){
            m_logger_ptr->debug("task with id: {} for process: {} finished with unknown error",
                          incoming_task.m_id.toStdString(), m_process_ptr->program().toStdString());
        }
        outcoming_json["result"]="Error";
        outcoming_json["data"]=QString("task with id: %1 for process: %2 finished with unknown error").
                                       arg(incoming_task.m_id,m_process_ptr->program());
    }
    m_process_ptr->deleteLater();
    emit signal_task_executed(incoming_task.m_id, outcoming_json);
}

TaskExecutor::TaskExecutor(std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const QString &app_dir_path, const std::string &log_name, QObject *parent)
    : QObject(parent), m_ssl_configuration_ptr{ssl_configuration_ptr}, m_app_dir_path{app_dir_path}, m_log_name{log_name}
{
    m_logger_ptr=spdlog::get(m_log_name);
}

void TaskExecutor::set_incoming_task(const TaskItem &incoming_task)
{
    m_incoming_task=incoming_task;
}

void TaskExecutor::start_execute()
{
    if(m_logger_ptr){
        m_logger_ptr->debug("start execute task with id: {} and name: {}",
                           m_incoming_task.m_id.toStdString(),m_incoming_task.m_name.toStdString());
    }

    switch(m_incoming_task.m_type){
    case TaskItem::DOWNLOAD:
        download(m_incoming_task);
        break;
    case TaskItem::EXECUTE_FILE:
        execute_file(m_incoming_task);
        break;
    case TaskItem::EXECUTE_PROGRAMM:
        execute_programm(m_incoming_task);
        break;
    default:
        if(m_logger_ptr){
            m_logger_ptr->debug("found unknown task name: {} for task with id: {}",
                               m_incoming_task.m_name.toStdString(),m_incoming_task.m_id.toStdString());
        }
    }
}
