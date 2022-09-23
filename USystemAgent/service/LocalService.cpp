#include <iostream>

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSslCertificate>

#include "Bootstrap.h"
#include "LocalService.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"

using settings_item=std::pair<QString, QVariant>;

void LocalService::init_spdlog()
{
    bool log_path_success=QDir(m_app_dir_path).mkpath("../var/log/usagent");
    const QString log_name=QString("%1.txt").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd hh_mm_ss"));
    const std::string path_to_log_file=log_path_success ? m_app_dir_path.toStdString() + std::string{"/../var/log/usagent/" + log_name.toStdString()}
                                                   : m_app_dir_path.toStdString()+ std::string{"/" + log_name.toStdString()};
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(path_to_log_file, 0, 0));

    m_logger_ptr=spdlog::get(m_log_name);
    if(!m_logger_ptr){
        m_logger_ptr=std::make_shared<spdlog::logger>(m_log_name, sinks.begin(),sinks.end());
        spdlog::register_logger(m_logger_ptr);
        //m_logger_ptr->set_pattern("[%H:%M:%S:%e.%f %z] [%o] [thread %t] [%n] %^[%L] %v%$");
        m_logger_ptr->set_level(spdlog::level::info);
        m_logger_ptr->flush_on(spdlog::level::info);
    }
}

bool LocalService::init_app_dirs()
{
    if(m_logger_ptr){
        m_logger_ptr->info("start to init app dirs");
    }

    bool success {false};
    success=QDir().mkpath(m_etc_path);
    success=QDir().mkpath(m_var_path);
    success=QDir().mkpath(m_etc_usagent_path);
    success=QDir().mkpath(m_var_usagent_path);
    success=QDir().mkpath(m_cert_path);
    success=QDir().mkpath(m_downloaded_file_path);

    if(m_logger_ptr){
        m_logger_ptr->info("end to init app dirs, result: {}", success);
    }
    return success;
}

bool LocalService::init_cert_and_key()
{
    if(m_logger_ptr){
        m_logger_ptr->info("start to init ssl cert and key");
    }

    bool success {true};
    bool is_need_to_get_client_cert {false};

    QSslCertificate ca_cert {};
    QString file_path_ca=m_app_settings_ptr->value("client/caCertPath").toString();
    QString file_path_pk=m_app_settings_ptr->value("client/clientPrivateKey").toString();
    QString file_path_client_cert=m_app_settings_ptr->value("client/clientCert").toString();

    //check does caCertPath file exist and contains valid data
    {
        QFile file_ca(file_path_ca);
        if (!file_ca.open(QIODevice::ReadOnly)) {
            if(m_logger_ptr){
                m_logger_ptr->warn("Failed to open file '{}' (for 'caCertPath')",
                                    file_path_ca.toStdString());
            }
            return false;
        }

        QList<QSslCertificate> list_of_cert=QSslCertificate::fromDevice(&file_ca);
        if (0 == list_of_cert.size()) {
            if(m_logger_ptr){
                m_logger_ptr->warn("'{}' ('caCertPath') doesn't contain valid certifaicate!",
                                    file_path_ca.toStdString());
            }
            return false;
        }

        ca_cert = list_of_cert[0];
    }

    //check 'ca_cert' expiry date
    {
        QDateTime expired(ca_cert.expiryDate());
        if (expired < QDateTime::currentDateTimeUtc()) {
            if(m_logger_ptr){
                m_logger_ptr->warn("'caCert' date is expired! Expired in '{}'",
                                    expired.toString().toStdString());
            }
            return false;
        }
    }

    //check client private key
    {
        QFile file(file_path_pk);
        if (!file.exists()) {
            is_need_to_get_client_cert = true;
        }
    }

    //check client cert
    {
        QFile file(file_path_client_cert);
        if (!file.exists()) {
            is_need_to_get_client_cert = true;
        }
    }

    //if need to get client cert, use httpclientcert to operate with server
    if(is_need_to_get_client_cert){
    }

    //set _CN
    {
        QString CN {};
        QString file_path_client_cert=m_app_settings_ptr->value("client/clientCert").toString();
        QFile file_client_cert(file_path_client_cert);
        if (!file_client_cert.open(QIODevice::ReadOnly)) {
            if(m_logger_ptr){
                m_logger_ptr->warn("Failed to open file '{}'",
                                    file_path_client_cert.toStdString());
            }
            return false;
        }

        QSslCertificate client_cert(&file_client_cert);
        QStringList subj_info=client_cert.subjectInfo(QSslCertificate::CommonName);

        if (subj_info.size() > 0 ) {
            CN = subj_info.at(0);
        }

        if (CN.isEmpty()) {
            if(m_logger_ptr){
                m_logger_ptr->warn("CN is empty. Cert path: '{}'",
                                    file_path_client_cert.toStdString());
            }
            return false;
        }
        if(m_logger_ptr){
            m_logger_ptr->info("CN is: '{0}'",CN.toStdString());
        }
    }

    if(m_logger_ptr){
        m_logger_ptr->info("end to init ssl cert and key, result: {}", success);
    }

    return true;
}

bool LocalService::init_app_settings()
{
    if(m_logger_ptr){
        m_logger_ptr->info("start to init app settings");
    }

    QString cert_path {};
    QString app_settings_path {};
    const QString app_settings_file_name {"usagent.conf"};

#ifdef Q_OS_WINDOWS
    app_settings_path=m_app_dir_path + "/../etc/usagent";
    cert_path=app_settings_path + "/cert";
#endif
#ifdef Q_OS_UNIX
    app_settings_path=m_app_dir_path + "/../etc/usagent";
    cert_path=app_settings_path + "/cert";
#endif

    std::shared_ptr<QSettings> app_settings_ptr {nullptr};

    //default settings list
    std::initializer_list<settings_item> setting_items {
          //"trace", "debug", "info", "warning", "error", "critical", "off"
          { "log/level", 	"off" }

        , { "mainSettings/urlPostCsrAndReceiveClientCert", 	"" } //may be populated by install
        , { "mainSettings/agentMode", 	"user" } //'user' or 'system'

        //,{ "remote_server/wsaddress",  "wss://dev.u-system.tech/sub" }

        , { "server/port", 56987 }

        , { "remoteAccess/enabled", 	false }
        //,{ "remoteAccess/globalAddress",  "dev.u-system.tech" }
        //,{ "remoteAccess/globalPort",  4444 }

        , { "client/caCertPath"				, cert_path + "/cacert.pem" }
        , { "client/clientPrivateKey"		, cert_path + "/clientPrivateKey.pem" }
        , { "client/clientCert"				, cert_path + "/clientCert.pem" }
        , { "client/privateKeyPassPhrase"	, "" } //maby used in future

        , { "sentry/httpLogPath", "https://0229dea89cb84318a272e552318b0d03@sentry.it2c.ru/5" }

          //"trace", "debug", "info", "warning", "error", "critical", "off"
        , { "syslog/level", 	"off" }
        , { "syslog/host", 	"" }
        , { "syslog/port", 	10514 }
    };

    //check if app config file exists
    if(!QFile::exists(app_settings_path + "/" + app_settings_file_name)){
        if(m_logger_ptr){
            m_logger_ptr->warn("app settings file: {} on path: {} not exists!",
                                app_settings_file_name.toStdString(), app_settings_path.toStdString());
        }
        return false;
    }

    //check if settings no error
    app_settings_ptr=std::make_shared<QSettings>(app_settings_path + "/" + app_settings_file_name, QSettings::IniFormat);
    if(app_settings_ptr->status()!=QSettings::NoError){
        if(m_logger_ptr){
            m_logger_ptr->warn("error init QSettings object!");
        }
        return false;
    }

    //fill settings default values if is empty
    for(const std::pair<QString, QVariant>& item: setting_items){
        if(app_settings_ptr->value(item.first).isNull()){
            app_settings_ptr->setValue(item.first, item.second);
        }
    }

    m_app_settings_ptr=std::move(app_settings_ptr);
    app_settings_ptr.reset();

    if(m_logger_ptr){
        m_logger_ptr->info("end to init app settings, result: {}", m_app_settings_ptr!=nullptr);
    }

    return true;
}

LocalService::LocalService(int argc, char **argv):QApplication(argc, argv)
{
    m_app_dir_path=QApplication::applicationDirPath();
    init_spdlog();

    m_timer.setInterval(m_interval);
    QObject::connect(&m_timer, &QTimer::timeout, this, &LocalService::slot_timeout);
}

void LocalService::start()
{
    if(m_logger_ptr){
        m_logger_ptr->info("start to init service");
    }

    if(init_app_dirs() && init_app_settings()){
        m_timer.start();
    }
}

void LocalService::slot_timeout()
{
    if(init_cert_and_key()){
        m_timer.stop();
        m_bootstrap_ptr=std::make_shared<Bootstrap>(m_log_name, m_app_dir_path, m_app_settings_ptr);
        m_bootstrap_ptr->run();

        if(m_logger_ptr){
            m_logger_ptr->info("service init success, create and run Bootstrap");
        }
    }
}
