#include <QJsonObject>
#include <QSslCertificate>

#include "X509Csr.h"
#include "openssl/pem.h"

X509csr::X509csr(const std::string &log_name)
    :m_log_name{log_name}
{

}

bool X509csr::generate_private_key_and_csr_by_ca_cert(const QSslCertificate &ca_cert, const QJsonObject &params_json)
{
    const QString CN=params_json["CN"].toString();

    const auto O	{ ca_cert.subjectInfo(QSslCertificate::Organization).at(0) };
    const auto C  	{ ca_cert.subjectInfo(QSslCertificate::CountryName).at(0) };
    const auto L  	{ ca_cert.subjectInfo(QSslCertificate::LocalityName).at(0) };
    const auto OU 	{ ca_cert.subjectInfo(QSslCertificate::OrganizationalUnitName).at(0) };
    const auto ST 	{ ca_cert.subjectInfo(QSslCertificate::StateOrProvinceName).at(0) };

    int				ret = 0;
    RSA				*r = nullptr;
    BIGNUM			*bne = nullptr;

    int				nVersion = 1;
    int				bits = 2048;
    unsigned long	e = RSA_F4;

    X509_REQ		*x509_req = nullptr;
    X509_NAME		*x509_name = nullptr;
    EVP_PKEY		*pKey = nullptr;
    BIO				*csrout = nullptr
                    , *bio_private = nullptr;

    const auto pathToPrivateKeyQString 	{ params_json.value("pathToPrivateKey").toString() };
    const auto pathTopCsr 				{ params_json.value("pathToCsr").toString() };

    // 1. generate rsa key
    bne = BN_new();
    ret = BN_set_word(bne,e);
    if(ret != 1) { goto free_all; }

    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, nullptr);
    if(ret != 1) { goto free_all; }

    // 2. save private key
    bio_private = BIO_new_file(pathToPrivateKeyQString.toStdString().c_str(), "w");
    ret = PEM_write_bio_RSAPrivateKey(bio_private, r, NULL, NULL, 0, NULL, NULL);
    if (ret != 1) { goto free_all; }

    // 3. set version of x509 req
    x509_req = X509_REQ_new();
    ret = X509_REQ_set_version(x509_req, nVersion);
    if (ret != 1) { goto free_all; }

    // 4. set subject of x509 req
    x509_name = X509_REQ_get_subject_name(x509_req);

    ret = X509_NAME_add_entry_by_txt(x509_name,"C", MBSTRING_ASC, (const unsigned char*) C.toStdString().c_str(), -1, -1, 0);
//        if (ret != 1) { goto free_all; }

    ret = X509_NAME_add_entry_by_txt(x509_name,"ST", MBSTRING_ASC, (const unsigned char*) ST.toStdString().c_str(), -1, -1, 0);
//        if (ret != 1) { goto free_all; }

    ret = X509_NAME_add_entry_by_txt(x509_name,"L", MBSTRING_ASC, (const unsigned char*) L.toStdString().c_str(), -1, -1, 0);
//        if (ret != 1) { goto free_all; }

    ret = X509_NAME_add_entry_by_txt(x509_name,"O", MBSTRING_ASC, (const unsigned char*) O.toStdString().c_str(), -1, -1, 0);
//        if (ret != 1) {  goto free_all; }

    ret = X509_NAME_add_entry_by_txt(x509_name,"CN", MBSTRING_ASC
                                     , (const unsigned char*) CN.toStdString().c_str()
                                     , -1, -1, 0);
    if (ret != 1) { goto free_all; }

    // 5. set public key of x509 req
    pKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pKey, r);
    r = nullptr;	// will be free rsa when EVP_PKEY_free(pKey)

    ret = X509_REQ_set_pubkey(x509_req, pKey);
    if (ret != 1) { goto free_all; }

    // 6. set sign key of x509 req
    ret = X509_REQ_sign(x509_req, pKey, EVP_sha1());	// return x509_req->signature->length
    if (ret <= 0) { goto free_all; }

    csrout = BIO_new_file(pathTopCsr.toStdString().c_str(), "w");
    ret = PEM_write_bio_X509_REQ(csrout, x509_req);

    // 7. free
free_all:
    X509_REQ_free(x509_req);
    BIO_free_all(csrout);

    EVP_PKEY_free(pKey);
    BN_free(bne);

    BIO_free_all(bio_private);

    return (ret == 1);
}
