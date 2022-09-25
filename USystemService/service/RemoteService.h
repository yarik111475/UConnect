#ifndef SERVICE_H
#define SERVICE_H

#include <QTimer>
#include <QJsonObject>
#include <QSharedPointer>
#include <QCoreApplication>

#include "qtservice.h"
#include "spdlog/spdlog.h"

class TcpServer;
class HttpClient;
class QSslConfiguration;

class RemoteService : public QObject, public QtService<QCoreApplication>
{
    Q_OBJECT
private:
    const int         m_interval {1000 * 20};
    QString           m_app_dir_path {};
    const QString     m_agent_cert_path {"USystem/agentServer/etc/usagent/cert"};
    const QString     m_agent_config_path {"USystem/agentServer/etc/usagent"};
    QJsonObject       m_addresses_json {};
    QString           m_api_path {"/api/v1/tasks/agent-service/tasks"};

    const std::string                 m_log_name {"service_logger"};
    std::shared_ptr<spdlog::logger>   m_logger_ptr {nullptr};

    QTimer             m_timer;
    TcpServer*         m_tcp_server_ptr {nullptr};
    HttpClient*        m_http_client_ptr {nullptr};

    std::shared_ptr<QSslConfiguration> m_ssl_configuration_ptr {nullptr};

//temporary
public:
    void init_spdlog();
    bool init_ssl_configuration();
    bool init_addresses();

protected:
    void start();

public:
    explicit RemoteService(int argc, char **argv, const QString& service_name);
    ~RemoteService();

private slots:
    void slot_timeout();

signals:

};

#endif // SERVICE_H
