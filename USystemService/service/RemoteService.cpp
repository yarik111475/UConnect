#include <QDir>
#include <QFile>
#include <QTimer>
#include <QSslKey>
#include <QSettings>
#include <QSslConfiguration>

#include "RemoteService.h"
#include "network/TcpServer.h"
#include "network/HttpClient.h"
#include "network/WebsocketClient.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

void RemoteService::init_spdlog()
{
    bool log_path_success=QDir(m_app_dir_path).mkpath("../logs");
    const std::string pathToLogFile=log_path_success ? m_app_dir_path.toStdString() + std::string{"/../logs/log.txt"}
                                                   : m_app_dir_path.toStdString()+ std::string{"/log.txt"};
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(pathToLogFile, 0, 0));

    m_logger_ptr=spdlog::get(m_log_name);
    if(!m_logger_ptr){
        m_logger_ptr=std::make_shared<spdlog::logger>(m_log_name, sinks.begin(),sinks.end());
        spdlog::register_logger(m_logger_ptr);
        m_logger_ptr->set_level(spdlog::level::debug);
        m_logger_ptr->flush_on(spdlog::level::debug);
    }

}

bool RemoteService::init_ssl_configuration()
{
    bool success {true};
    QString complete_cert_path {};
#ifdef Q_OS_WINDOWS
    complete_cert_path=qEnvironmentVariable("programfiles(x86)") + "/" + m_agent_cert_path;
#endif
#ifdef Q_OS_UNIX
#endif

    const QString ca_file_name {"cacert.pem"};
    const QString key_file_name {"clientPrivateKey.pem"};
    const QString cert_file_name {"clientCert.pem"};
    try{
        m_ssl_configuration_ptr=std::make_shared<QSslConfiguration>(QSslConfiguration::defaultConfiguration());
        m_ssl_configuration_ptr->setProtocol(QSsl::AnyProtocol);
        m_ssl_configuration_ptr->setPeerVerifyMode(QSslSocket::VerifyPeer);

        //init certificates list
        QFile file_ca(complete_cert_path + "/" + ca_file_name);
        if(file_ca.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&file_ca);
            m_ssl_configuration_ptr->setCaCertificates(listOfCert);
        }
        else{
            success=false;
        }

        //init private key
        QFile file_key(complete_cert_path + "/" + key_file_name);
        if(file_key.open(QIODevice::ReadOnly)){
            QSslKey key (&file_key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey,QByteArray());
            m_ssl_configuration_ptr->setPrivateKey(key);
        }
        else{
            success=false;
        }

        //init client certificates list
        QFile file_crt(complete_cert_path + "/" + cert_file_name);
        if(file_crt.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&file_crt);
            m_ssl_configuration_ptr->setLocalCertificateChain(listOfCert);
        }
        else{
            success=false;
        }

    }
    catch(std::exception& ex){
        success=false;
    }
    catch(...){
        success=false;
    }
    return success;
}

bool RemoteService::init_addresses()
{
    bool success {true};
    QString complete_config_path {};
    const QString config_file_name {"usagent.conf"};
#ifdef Q_OS_WINDOWS
    complete_config_path=qEnvironmentVariable("programfiles(x86)") + "/" + m_agent_config_path;
#endif
#ifdef Q_OS_UNIX
#endif

    try{
        QSettings settings(complete_config_path + "/" + config_file_name, QSettings::IniFormat);
        settings.beginGroup("remote_server");
        m_addresses_json["address"]=settings.value("address", "").toString();
        m_addresses_json["wsaddress"]=settings.value("wsaddress", "").toString();
        settings.endGroup();
        success=QUrl(m_addresses_json["address"].toString()).isValid() &&
                QUrl(m_addresses_json["wsaddress"].toString()).isValid();
    }
    catch(std::exception& ex){
        success=false;
    }
    catch(...){
        success=false;
    }
    return success;
}

void RemoteService::start()
{
    //set app directory
    m_app_dir_path=QCoreApplication::applicationDirPath();

    //init logger
    init_spdlog();
    spdlog::get(m_log_name)->debug("service started success");

    m_timer.setInterval(m_interval);
    QObject::connect(&m_timer, &QTimer::timeout,this, &RemoteService::slot_timeout);
    m_timer.start();
}

RemoteService::RemoteService(int argc, char **argv, const QString &service_name)
    : QtService<QCoreApplication>(argc, argv, service_name)
{
}

RemoteService::~RemoteService()
{
}

void RemoteService::slot_timeout()
{
    bool success=(init_ssl_configuration() & init_addresses());
    if(success){
        m_logger_ptr->debug("certificates and config files found success, stop timer, start tcp server and http client");
        m_timer.stop();

        m_tcp_server_ptr=new TcpServer(m_app_dir_path, m_log_name, this);
        m_tcp_server_ptr->start();

        m_http_client_ptr=new HttpClient(QUrl(m_addresses_json["address"].toString()),443,m_api_path, m_app_dir_path, m_ssl_configuration_ptr, m_log_name, this);
        m_http_client_ptr->start();
    }
}


