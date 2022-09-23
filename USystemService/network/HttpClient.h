#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QSslError>
#include <QProcess>
#include <QJsonObject>
#include <QSharedPointer>

#include "tasks/Task.h"
#include "spdlog/spdlog.h"

class QTimer;
class QThread;
class QProcess;
class QNetworkReply;
class QNetworkRequest;
class QSslConfiguration;
class QNetworkAccessManager;

class TaskQueue;

class HttpClient : public QObject
{
    Q_OBJECT
private:
    QString m_uid {};
    QUrl    m_http_url {};
    int     m_http_port {443};
    QString m_api_path {};
    QString m_app_dir_path {};

    int         m_interval {1000 * 30};
    std::string m_log_name {};
    std::shared_ptr<spdlog::logger> m_logger_ptr {nullptr};

    QTimer m_timer;
    std::shared_ptr<QSslConfiguration>    m_ssl_configuration_ptr {nullptr};

    QThread*   m_task_thread_ptr {nullptr};
    TaskQueue* m_task_queue_ptr {nullptr};

public:
    HttpClient(const QUrl& http_url, int http_port, const QString api_path, const QString& app_dir_path,
               std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const std::string& log_name, QObject* parent=nullptr);
    ~HttpClient()=default;

    bool init_machine_uid();
    void start();
    void stop();

private:
    std::vector<TaskItem> parse_incoming_json(const QJsonObject& incoming_json);

private slots:
    void slot_timeout();

public slots:
    //task executor callback slot
    void slot_task_executed(const QString task_id,const QJsonObject& outcoming_json);
};

#endif // HTTPCLIENT_H
