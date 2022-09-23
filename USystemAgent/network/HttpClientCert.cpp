#include <openssl/pem.h>
#include <QFile>
#include <QSettings>
#include <QEventLoop>
#include <QJsonDocument>
#include <QSslCertificate>

#include "HttpClientCert.h"

HttpClientCert::HttpClientCert(const std::string &log_name, const QString &app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent)
    :HttpClientBase(log_name, app_dir_path, app_settings_ptr, parent)
{

}

void HttpClientCert::init(const QJsonObject& init_json)
{
    HttpClientBase::init(init_json);
}

void HttpClientCert::slot_timeout()
{
}
