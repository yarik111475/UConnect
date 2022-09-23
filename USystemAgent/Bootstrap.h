#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <QUrl>
#include <QObject>
#include <QSharedPointer>
#include <QSslCertificate>

#include "task/Task.h"
#include "spdlog/spdlog.h"

class QSettings;
class TaskQueue;
class TaskExecutor;
class HttpClientTask;
class HttpClientCert;
class QSslConfiguration;

class Bootstrap : public QObject
{
    Q_OBJECT
private:
    std::string log_name_ {};
    QUrl    http_url_ {};
    int     http_port_{};
    QString api_path_ {};
    QString app_dir_path_ {};

    TaskQueue*        task_queue_ptr_ {nullptr};
    HttpClientTask*   http_client_task_ptr_ {nullptr};

    std::shared_ptr<spdlog::logger>    logger_ptr_ {nullptr};
    std::shared_ptr<QSettings>         app_settings_ptr_ {nullptr};
    std::shared_ptr<QSslConfiguration> ssl_configuration_ptr_ {nullptr};

    bool init_ssl_configuration();

public:
    explicit Bootstrap(std::string &log_name, const QString& app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent = nullptr);

    void run();

private slots:
    void slot_execute_task(const TaskItem &task_item);
    void slot_enqueue_task_list(const std::vector<TaskItem> &incoming_task_list);

signals:

};

#endif // BOOTSTRAP_H
