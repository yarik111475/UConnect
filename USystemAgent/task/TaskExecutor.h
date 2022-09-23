#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H

#include <map>
#include <functional>
#include <QObject>
#include <QCoreApplication>

#include "Task.h"
#include "ITaskPlugin.h"
#include "spdlog/spdlog.h"

class TaskItem;
class QJsonObject;

using task_name_func_type=const char*(*)();
using create_plugin_func_type = ITaskPlugin*(*)();

class TaskExecutor : public QObject
{
    Q_OBJECT
private:
    std::string log_name_ {};
    std::shared_ptr<spdlog::logger> logger_ptr_ {nullptr};
    TaskItem task_item_ {};

    const QString plugin_path_ {QCoreApplication::applicationDirPath() + "/../plugins"};
    std::map<QString , ITaskPlugin*> plugin_map_ {};
    void resolve_plugin_list();

public:
    explicit TaskExecutor(const std::string& log_name, QObject *parent = nullptr);
    void init_executor(const TaskItem& task_item);
    QJsonObject execute_task(const TaskItem& task_item);

public slots:
    void slot_execute_task();

signals:
    void signal_task_executed(const TaskItem& task_item, const QJsonObject& outcoming_json);
};

#endif // TASKEXECUTOR_H
