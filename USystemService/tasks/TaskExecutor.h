#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H

#include <memory>
#include <functional>
#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>

#include "Task.h"
#include "spdlog/spdlog.h"

class QProcess;
class QJsonObject;
class QSslConfiguration;

class TaskExecutor : public QObject
{
    Q_OBJECT
private:
    std::string m_log_name {};
    QString     m_app_dir_path {};
    QString     m_file_name {};
    QString     m_hash {};
    TaskItem    m_incoming_task {};

    std::shared_ptr<spdlog::logger> m_logger_ptr {nullptr};
    QProcess* m_process_ptr {nullptr};
    std::shared_ptr<QSslConfiguration> m_ssl_configuration_ptr {nullptr};

private:
    void download(const TaskItem& incoming_task);
    void execute_file(const TaskItem &incoming_task);
    void execute_programm(const TaskItem& incoming_task);

public:
    explicit TaskExecutor(std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const QString& app_dir_path, const std::string& log_name,QObject *parent = nullptr);
    void set_incoming_task(const TaskItem& incoming_task);

public slots:
    void start_execute();

signals:
    void signal_task_executed(const QString task_id,const QJsonObject& outcoming_json);

};

#endif // TASKEXECUTOR_H
