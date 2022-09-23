#include <thread>
#include "TaskPlugin.h"

TaskPlugin::TaskPlugin()
{
}

int TaskPlugin::pluginVersion() const
{
    return 0;
}

void TaskPlugin::handle_impl(const std::string &incoming_string, std::string &outcoming_string)
{
    outcoming_string=QJsonDocument(QJsonObject{}).toJson().toStdString();
}
