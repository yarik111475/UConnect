#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include <map>
#include <QList>
#include <QHash>
#include <QJsonObject>
#include <QNetworkAddressEntry>

#include "TaskItem.h"
#include "ITaskPlugin.h"

struct HostResultType{
    bool is_up_ {false};
    std::string dst_host_address_ {};
    std::string src_host_address_ {};
    std::string mac_address_ {};
    std::string host_name_ {};
};

struct TcpResultType{
    uint16_t port_ {};
    std::string type_ {"tcp"};
    std::string answer_ {};
    inline bool empty()const{
        return port_==0 || type_.empty();
    }
};

struct SnmpResultType{
    bool enabled_ {false};
    std::string version_ {};
    std::map<std::string,std::string> oids_values_ {};
};

class  TaskPlugin : public ITaskPlugin
{
private:
    TaskItem parse_incoming_string(const std::string &incoming_string);
    static HostResultType send_arp(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name={});
    static HostResultType send_arp_windows(const QString& dst_host_address, const QString& src_host_address);
    static HostResultType send_arp_linux(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name);

    static std::vector<TcpResultType> scan_tcp_ports(const std::string &host_address, const std::set<uint16_t>& ports);
    static SnmpResultType scan_snmp_oid_values_simple(const std::string &host_address, const std::vector<std::string> &standard_oids);

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
    return "localNetworkAudit";
}

#endif // TASKPLUGIN_H
