#include <thread>
#include <future>
#include <string>
#include <functional>
#include <QFileInfo>
#include <QLibrary>
#include <QJsonObject>
#include <QDirIterator>

#include "Task.h"
#include "ITaskPlugin.h"
#include "TaskExecutor.h"

void TaskExecutor::resolve_plugin_list()
{
    QString task_name {};
    QDir plugin_dir{plugin_path_};
    QList<QFileInfo> entry_list=plugin_dir.entryInfoList(QStringList() << "*.dll" << "*.so", QDir::Files);
    for(const QFileInfo& entry: entry_list){
        QLibrary library(entry.absoluteFilePath());
        if(!library.load()){
            continue;
        }

        task_name_func_type plugin_name_func=(task_name_func_type)library.resolve("taskName");
        if(plugin_name_func){
            task_name=QString(plugin_name_func());
            if(!task_name.isEmpty()){
                create_plugin_func_type create_plugin_func=(create_plugin_func_type)library.resolve("createPlugin");
                if(create_plugin_func){
                    plugin_map_.emplace(task_name, create_plugin_func());
                }
            }
        }
    }
}

TaskExecutor::TaskExecutor(const std::string &log_name, QObject *parent)
    : QObject(parent),log_name_{log_name}
{
    logger_ptr_=spdlog::get(log_name_);
    resolve_plugin_list();
}

void TaskExecutor::init_executor(const TaskItem &task_item)
{
    task_item_=task_item;
}

QJsonObject TaskExecutor::execute_task(const TaskItem &task_item)
{
    QJsonObject outcoming_json {};
    auto found=plugin_map_.find(task_item_.name_);
    if(found!=plugin_map_.end()){
        if(logger_ptr_){
            logger_ptr_->info("execute task with id: {} and name: {}",
                              task_item_.id_.toStdString(), task_item_.name_.toStdString());
        }

        std::string outcoming_string {};
        found->second->handle(task_item_.incoming_string_, outcoming_string);
        outcoming_json=QJsonDocument::fromJson(QString::fromStdString(outcoming_string).toUtf8()).object();
    }
    return outcoming_json;
}

void TaskExecutor::slot_execute_task()
{
    QJsonObject outcoming_json {};
    auto found=plugin_map_.find(task_item_.name_);
    if(found!=plugin_map_.end()){
        if(logger_ptr_){
            logger_ptr_->info("execute task with id: {} and name: {}",
                              task_item_.id_.toStdString(), task_item_.name_.toStdString());
        }

        std::string outcoming_string {};
        found->second->handle(task_item_.incoming_string_, outcoming_string);
        outcoming_json=QJsonDocument::fromJson(QString::fromStdString(outcoming_string).toUtf8()).object();
    }

    emit signal_task_executed(task_item_, outcoming_json);
}
