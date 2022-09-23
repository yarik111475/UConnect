#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include <QJsonArray>
#include <QJsonObject>
#include "TaskItem.h"
#include <ITaskPlugin.h>
#include "snmp_pp/snmp_pp.h"

class  TaskPlugin : public ITaskPlugin
{
private:
    std::string get_snmp_syntax_string(SmiUINT32 syntax);
    TaskItem parse_incoming_string(const std::string &incoming_string);
    QJsonArray snmp_get_request(const TaskItem& task_item);

public:
    TaskPlugin()=default;
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
    return "snmpMonitoring";
}

#endif // TASKPLUGIN_H
