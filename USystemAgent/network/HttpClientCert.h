#ifndef HTTPCLIENTCERT_H
#define HTTPCLIENTCERT_H

#include <QHash>
#include <QVariant>
#include <QSslCertificate>

#include "HttpClientBase.h"

class HttpClientCert : public HttpClientBase
{
    Q_OBJECT
public:
    explicit HttpClientCert(const std::string& log_name, const QString& app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent = nullptr);

    ~HttpClientCert()=default;

    virtual void init(const QJsonObject& init_json) override;

protected slots:
    virtual void slot_timeout() override;
};

#endif // HTTPCLIENTCERT_H
