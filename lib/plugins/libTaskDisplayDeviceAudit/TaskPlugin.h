#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include "ITaskPlugin.h"

class  TaskPlugin : public ITaskPlugin
{
private:
    std::map<int, QString> orientation_map_{};
    QString get_orientation(Qt::ScreenOrientation orientation);

    QJsonArray get_displays_win();
    QJsonArray get_displays_common();

public:
    TaskPlugin();
    virtual ~TaskPlugin() = default;
    virtual int pluginVersion() const;
    virtual void handle_impl(const std::string& incoming_string, std::string& outcoming_string);
};

const char* minAgentVersion() {
    return "0.4.0";
}

ITaskPlugin *createPlugin() {
    return new TaskPlugin();
}

const char* taskName() {
    return "displayDeviceAudit";
}

#endif // TASKPLUGIN_H
