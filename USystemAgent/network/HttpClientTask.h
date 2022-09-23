#ifndef HTTPCLIENTTASK_H
#define HTTPCLIENTTASK_H

#include "network/HttpClientBase.h"

class QSslConfiguration;

class HttpClientTask : public HttpClientBase
{
    Q_OBJECT
private:
    std::vector<TaskItem> parse_incoming_json(const QJsonObject& incoming_json);

public:
    explicit HttpClientTask(const std::string& log_name, const QString& app_dir_path, std::shared_ptr<QSettings> app_settings_ptr, QObject *parent = nullptr);

    virtual ~HttpClientTask()=default;

    virtual void init(const QJsonObject& init_json) override;

public slots:
    virtual void slot_task_executed(const TaskItem& task_item, const QJsonObject& outcoming_json) override;

protected slots:
    virtual void slot_timeout() override;

};

#endif // HTTPCLIENTTASK_H
