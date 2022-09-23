#ifndef X509CSR_H
#define X509CSR_H

#include <string>
#include <memory>
#include <QString>
#include <QSettings>

class QJsonObject;
class QSslCertificate;

class X509csr
{
private:
    std::string     m_log_name {};
    QSslCertificate m_ca_cert {};

public:
    X509csr(const std::string& log_name);
    ~X509csr()=default;

    bool generate_private_key_and_csr_by_ca_cert(const QSslCertificate& ca_cert, const QJsonObject& params_json);
};

#endif // X509CSR_H
