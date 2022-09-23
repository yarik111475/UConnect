#include <QNetworkReply>
#include <QSslConfiguration>
#include <QNetworkAccessManager>

#include "HttpClientBase.h"

HttpClientBase::HttpClientBase(const std::string &log_name, const QString &app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent)
    : QObject(parent),log_name_{log_name}, app_dir_path_{app_dir_path}, app_settings_ptr_{app_settings_ptr}
{
    logger_ptr_=spdlog::get(log_name);
    access_manager_ptr_=std::make_shared<QNetworkAccessManager>(this);
    access_manager_ptr_->setTransferTimeout(5000);
    QObject::connect(access_manager_ptr_.get(), &QNetworkAccessManager::sslErrors, this, &HttpClientBase::slot_ssl_errors);
}

void HttpClientBase::init(const QJsonObject& init_json)
{
    http_url_=init_json["http_url"].toString();
    http_port_=init_json["http_port"].toInt();
    api_path_=init_json["api_path"].toString();
}

void HttpClientBase::start()
{
    timer_.setInterval(interval_);
    QObject::connect(&timer_, &QTimer::timeout, this, &HttpClientBase::slot_timeout);
    timer_.start();
}

void HttpClientBase::stop()
{
    timer_.stop();
}

void HttpClientBase::slot_ssl_errors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(errors)
    try{
        reply->ignoreSslErrors();
    }
    catch(...){
    }
}
