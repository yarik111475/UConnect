#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <QSslError>
#include <QSharedPointer>

#include "tasks/Task.h"
#include "spdlog/spdlog.h"

class QWebSocket;
class QNetworkReply;
class QSslConfiguration;
class QNetworkAccessManager;

class WebsocketClient : public QObject
{
    Q_OBJECT
private:
    bool        m_connected {false};
    QString     m_error {};
    QUrl        m_ws_url {};
    int         m_ws_port {};
    QString     m_app_dir_path {};
    QString     m_file_name {};
    std::string m_log_name {};

    QString m_uid {};
    int     m_interval {5000};
    QTimer  m_ping_timer, m_connection_timer;

    std::shared_ptr<spdlog::logger> m_logger_ptr {nullptr};

    QSharedPointer<QProcess>   m_process_ptr {nullptr};
    QSharedPointer<QWebSocket> m_command_socket_ptr {nullptr};
    QSharedPointer<QWebSocket> m_result_socket_ptr {nullptr};
    QSharedPointer<QNetworkAccessManager> m_accessmanager_ptr {nullptr};
    std::shared_ptr<QSslConfiguration>     m_ssl_configuration_ptr {nullptr};

    std::vector<TaskItem> parse_incoming_json(const QJsonObject& incoming_json);

public:
    bool init_ssl_configuration();
    bool init_machine_uid();

    void execute_task(const TaskItem& incoming_task);

    void download(const TaskItem& incoming_task);
    void execute_programm(const TaskItem& incoming_task);
    void execute_file(const TaskItem& incoming_task);

    void send_response(const QString& result);

public:
    explicit WebsocketClient(const QUrl& ws_url, int ws_port, const QString& app_dir_path,
                             std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const std::string& log_name,  QObject *parent = nullptr);
    ~WebsocketClient();
    void start();
    void stop();

private slots:
    //websocket slots
    void slot_ping_timeout();
    void slot_connect_timeout();
    void slot_ws_ssl_errors(const QList<QSslError> &errors);
    void slot_connected();
    void slot_disconnected();
    void slot_text_message_received(const QString &message);
    void slot_pong(quint64 elapsed_time, const QByteArray &payload);

    //http(s) slots
    void slot_finished();
    void slot_download_progress(qint64 bytes_received, qint64 bytes_total);
    void slot_http_ssl_errors(QNetworkReply *reply, const QList<QSslError> &errors);

    //process slots
    void slot_process_finished(int exit_code, QProcess::ExitStatus exit_status);

signals:

};

#endif // WEBSOCKETCLIENT_H
