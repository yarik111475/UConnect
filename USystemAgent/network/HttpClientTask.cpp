#include <QFile>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QNetworkAccessManager>

#include "HttpClientTask.h"

std::vector<TaskItem> HttpClientTask::parse_incoming_json(const QJsonObject &incoming_json)
{
    std::vector<TaskItem> task_list {};

    if(!incoming_json.contains("items")){
        return task_list;
    }

    QJsonArray root_json_array=incoming_json.value("items").toArray();
    if(root_json_array.empty()){
        return task_list;
    }
    QJsonObject kernel_params {};
    kernel_params["machine_uid"]="a7fa3c16-7f9c-4f65-a28e-ae671180ae79";

    for(const QJsonValue& root_json_item: root_json_array){
        if(root_json_item.toObject().contains("id") && root_json_item.toObject().contains("name") && root_json_item.toObject().contains("additional_params")){
            QJsonObject root_json(root_json_item.toObject());

            TaskItem task_item {};
            task_item.id_=root_json["id"].toString();
            task_item.name_=root_json["name"].toString();

            root_json.insert("kernelParams", kernel_params);

            task_item.incoming_string_=QJsonDocument(root_json).toJson().toStdString();
            task_list.push_back(task_item);
        }
    }
    return task_list;
}

HttpClientTask::HttpClientTask(const std::string &log_name, const QString &app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent)
    :HttpClientBase(log_name, app_dir_path, app_settings_ptr, parent)
{

}

void HttpClientTask::init(const QJsonObject& init_json)
{
    HttpClientBase::init(init_json);
}

void HttpClientTask::slot_task_executed(const TaskItem &task_item, const QJsonObject &outcoming_json)
{
    try{
        QUrl url {http_url_};
        url.setPort(http_port_);
        QString response_api_path {QString(api_path_ + "%1/result").arg(task_item.id_)};
        url.setPath(response_api_path);

        QEventLoop eventLoop {};
        QObject::connect(access_manager_ptr_.get(), &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

        QNetworkRequest request;
        request.setUrl(url);
        request.setSslConfiguration(*ssl_configuration_ptr_.get());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if(logger_ptr_){
            logger_ptr_->info("SEND PUT REQUEST, 'url': {}, 'data': \n{}",
                               url.toString().toStdString(),QJsonDocument(outcoming_json).toJson().toStdString());
        }

        QNetworkReply* reply=access_manager_ptr_->put(request, QJsonDocument(outcoming_json).toJson());
        eventLoop.exec();

        if(reply->error()==QNetworkReply::NoError){
            if(logger_ptr_){
                logger_ptr_->info("RECIEVED RESPONSE: 'true', status for task with id: {} changed to: {}",
                                   task_item.id_.toStdString(), outcoming_json["result"].toString().toStdString());
            }
        }
        else{
            if(logger_ptr_){
                logger_ptr_->warn("RECIEVED RESPONSE: 'error', reply error: {} in method 'HttpClientTask::slotTaskExecuted()'",
                                   reply->errorString().toStdString());
            }
        }
    }
    catch(std::exception& ex){

    }
    catch(...){
    }
}

void HttpClientTask::slot_timeout()
{
    try{
        QUrl url{http_url_};
        url.setPort(http_port_);
        url.setPath(api_path_);

        QEventLoop eventLoop {};
        QObject::connect(access_manager_ptr_.get(), &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

        QNetworkRequest request;
        request.setUrl(url);
        request.setSslConfiguration(*ssl_configuration_ptr_.get());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply=access_manager_ptr_->get(request);
        eventLoop.exec();

        if(reply->error()==QNetworkReply::NoError){
            //get all tasks from backend
            QByteArray content=reply->readAll();
            QJsonObject incoming_json=QJsonDocument::fromJson(content).object();
            std::vector<TaskItem> incoming_task_list=parse_incoming_json(incoming_json);

            //emit signal with incoming task list
            emit signal_incoming_task_list(incoming_task_list);
        }
        else{
            if(logger_ptr_){
                logger_ptr_->warn("RECIEVED RESPONSE: 'error', reply error: {} in method 'HttpClientTask::slot_timeout()'",
                                   reply->errorString().toStdString());
            }
        }
    }
    catch(std::exception& ex){
    }
    catch(...){
    }
}

