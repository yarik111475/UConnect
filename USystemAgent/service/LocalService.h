#ifndef LOCALSERVICE_H
#define LOCALSERVICE_H

#include <memory>
#include <QUrl>
#include <QTimer>
#include <QApplication>

#include "spdlog/spdlog.h"

class QSettings;
class Bootstrap;

class LocalService : public QApplication
{
    Q_OBJECT
private:
    std::string m_log_name {"agent_service"};

    QString     m_app_dir_path {QApplication::applicationDirPath()};
    QString     m_etc_path {m_app_dir_path + "/../etc"};
    QString     m_var_path {m_app_dir_path + "/../var"};
    QString     m_etc_usagent_path {m_etc_path + "/usagent"};
    QString     m_var_usagent_path {m_var_path + "/usagent"};
    QString     m_cert_path {m_etc_usagent_path + "/cert"};
    QString     m_downloaded_file_path {m_var_usagent_path + "/downloads"};

    int         m_interval {1000 * 10};
    QTimer      m_timer {};

    std::shared_ptr<spdlog::logger>    m_logger_ptr {nullptr};
    std::shared_ptr<Bootstrap>         m_bootstrap_ptr {nullptr};
    std::shared_ptr<QSettings>         m_app_settings_ptr {nullptr};

    void init_spdlog();
    void init_syslog();
    bool init_app_dirs();
    bool init_cert_and_key();
    bool init_app_settings();

public:
    explicit LocalService(int argc, char **argv);
    void start();
    ~LocalService()=default;

private slots:
    void slot_timeout();
};

#endif // LOCALSERVICE_H
