#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <QMutex>
#include <QQueue>
#include <QObject>
#include <QProcess>
#include <QSslError>
#include <QJsonObject>
#include <QWaitCondition>

#include "Task.h"
#include "spdlog/spdlog.h"

class QThread;
class HttpClient;
class QSslConfiguration;

class TaskQueue : public QObject
{
    Q_OBJECT
private:
    bool           m_flag {true};
    QQueue<TaskItem>   m_incoming_queue {};
    QMutex         m_mutex {};
    QWaitCondition m_wait_condition {};

    QString        m_app_dir_path {};
    std::string    m_log_name {};

    std::shared_ptr<spdlog::logger>     m_logger_ptr {nullptr};
    std::shared_ptr<HttpClient>         m_http_client_ptr {nullptr};
    std::shared_ptr<QSslConfiguration>  m_ssl_configuration_ptr {nullptr};

    void execute_task(const TaskItem &incoming_task);

public:
    explicit TaskQueue(std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, std::shared_ptr<HttpClient> http_client_ptr,
                       const QString& app_dir_path, const std::string& log_name, QObject *parent = nullptr);

public slots:
    void start();
    void stop();
    void push_task_list(const std::vector<TaskItem>& incoming_task_list);
};

#endif // TASKQUEUE_H
