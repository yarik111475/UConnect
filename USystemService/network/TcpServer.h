#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QSslError>
#include <QProcess>
#include <QJsonObject>
#include <QNetworkReply>
#include <QSharedPointer>

#include "spdlog/spdlog.h"

class QTcpSocket;
class QTcpServer;
class QNetworkReply;
class QSslConfiguration;
class QTemporaryFile;
class QNetworkAccessManager;

class TcpServer : public QObject
{
    Q_OBJECT
private:
    QString     m_error {};
    const int   m_tcp_port {2957};
    QString     m_app_dir_path {};
    bool        m_downloads_exists {false};
    std::string m_log_name {};
    std::shared_ptr<spdlog::logger> m_logger_ptr {nullptr};

    QTcpServer*  m_tcp_server_ptr {nullptr};
    QTcpSocket*  m_tcp_socket_ptr {nullptr};

public:
    explicit TcpServer(const QString& app_dir_path, const std::string& log_name,  QObject *parent = nullptr);
    void start();
    void stop();

private:
    void send_response(const QJsonObject& outcoming_json, bool disconnect=true);

private slots:
    void slot_new_connection();
    void slot_ready_read();

signals:

};

#endif // TCPSERVER_H
