#ifndef HTTPCLIENTBASE_H
#define HTTPCLIENTBASE_H
#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QSslError>
#include <QJsonObject>
#include <QSharedPointer>

#include "task/Task.h"
#include "spdlog/spdlog.h"

class QSettings;
class QNetworkReply;
class QSslConfiguration;
class QNetworkAccessManager;

class HttpClientBase : public QObject
{
    Q_OBJECT
protected:
    int         interval_ {1000 * 10};
    QTimer      timer_ {};
    std::string log_name_ {};
    QString     app_dir_path_ {};

    QUrl        http_url_ {};
    int         http_port_ {};
    QString     api_path_ {};
    QJsonObject init_json_ {};

    std::shared_ptr<spdlog::logger>        logger_ptr_ {nullptr};

    std::shared_ptr<QSettings>             app_settings_ptr_ {nullptr};
    std::shared_ptr<QSslConfiguration>     ssl_configuration_ptr_ {nullptr};
    std::shared_ptr<QNetworkAccessManager> access_manager_ptr_ {nullptr};

public:
    explicit HttpClientBase(const std::string& log_name, const QString& app_dir_path, std::shared_ptr<QSettings> app_settings_ptr,  QObject *parent = nullptr);
    virtual ~HttpClientBase()=default;

    virtual void init(const QJsonObject& init_json);
    virtual void start();
    virtual void stop();

    inline void set_interval(int interval){
        interval_=interval;
    }

    inline void set_ssl_configuration(std::shared_ptr<QSslConfiguration> ssl_configuration_ptr){
        ssl_configuration_ptr_=ssl_configuration_ptr;
    }

public slots:
    virtual void slot_task_executed(const TaskItem&, const QJsonObject&){}

protected slots:
    virtual void slot_timeout()=0;

private slots:
    void slot_ssl_errors(QNetworkReply *reply, const QList<QSslError> &errors);

signals:
    void signal_incoming_task_list(const std::vector<TaskItem>& task_list);
    void signal_cert_created_success();
};

#endif // HTTPCLIENTBASE_H
