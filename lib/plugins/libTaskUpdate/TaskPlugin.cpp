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
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    outcoming_string=QJsonDocument(QJsonObject{}).toJson().toStdString();
}
