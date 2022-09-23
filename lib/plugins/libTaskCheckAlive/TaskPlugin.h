#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include "ITaskPlugin.h"

class  TaskPlugin : public ITaskPlugin
{
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
    return "checkAlive";
}

#endif // TASKPLUGIN_H
