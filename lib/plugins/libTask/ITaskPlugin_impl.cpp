#include <thread>
#include <future>
#include <iostream>

#include "ITaskPlugin.h"

void ITaskPlugin::init_plugin(const std::string& incoming_string){
    QJsonObject incoming_json=QJsonDocument::fromJson(QString::fromStdString(incoming_string).toUtf8()).object();
    QJsonObject additional_params_json=incoming_json["additional_params"].toObject();
    timeout_=additional_params_json.contains("timeout") ? additional_params_json["timeout"].toInt() : 0;
    QJsonObject kernel_params=incoming_json["kernelParams"].toObject();
    machine_uid_=kernel_params.contains("machine_uid") ? kernel_params["machine_uid"].toString() : QString();
}

QJsonObject ITaskPlugin::success_result(const QJsonObject& data_json_obj){
    QJsonObject outcoming_json {};
    outcoming_json["result"]="Success";
    outcoming_json["data"]=data_json_obj;
    outcoming_json["machine_uid"]=machine_uid_;
    outcoming_json["processed_at"]=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    return outcoming_json;
}

QJsonObject ITaskPlugin::success_result(const QJsonArray &data_json_array)
{
    QJsonObject outcoming_json {};
    outcoming_json["result"]="Success";
    outcoming_json["data"]=data_json_array;
    outcoming_json["machine_uid"]=machine_uid_;
    outcoming_json["processed_at"]=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    return outcoming_json;
}

QJsonObject ITaskPlugin::error_result(int error_code, const QString& error_msg){
    QJsonObject outcoming_json {};
    outcoming_json["result"]="Error";
    outcoming_json["error_msg"]=error_msg;
    outcoming_json["error_code"]=error_code;
    outcoming_json["machine_uid"]=machine_uid_;
    outcoming_json["processed_at"]=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    return outcoming_json;
}

void ITaskPlugin::handle(const std::string &incoming_string, std::string &outcoming_string)
{
    init_plugin(incoming_string);
    std::string in_string {incoming_string};
    std::string out_string {};
    try{
        std::future<void> future=std::async(std::launch::deferred, [this, &in_string, &out_string]{
            handle_impl(in_string, out_string);
        });
        if(timeout_ && future.wait_for(std::chrono::milliseconds(timeout_))==std::future_status::timeout){
            outcoming_string=QJsonDocument(error_result(-1, "Task timeout expired")).toJson().toStdString();
        }
        else{
            future.wait();
            QJsonDocument json_doc=QJsonDocument::fromJson(out_string.c_str());
            if(json_doc.isArray()){
                outcoming_string=QJsonDocument(success_result(json_doc.array())).toJson().toStdString();
            }
            else{
                outcoming_string=QJsonDocument(success_result(json_doc.object())).toJson().toStdString();
            }
        }
    }
    catch(std::exception& ex){
        outcoming_string=QJsonDocument(error_result(-1, QString(ex.what()))).toJson().toStdString();
    }
    catch(...){
        outcoming_string=QJsonDocument(error_result(-1, "Unknown task error")).toJson().toStdString();
    }
}
