#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include <QJsonObject>
#include <QNetworkInterface>

#include "ITaskPlugin.h"

struct HostResultType{
    bool is_up_ {false};
    std::string dst_host_address_ {};
    std::string src_host_address_ {};
    std::string mac_address_ {};
    std::string host_name_ {};
};

class  TaskPlugin : public ITaskPlugin
{
private:
#ifdef Q_OS_UNIX
    QJsonObject nmcliDeviceInfo() const;
    QJsonObject nmcliConnections() const;
    QJsonObject nmcliConnectionByUuid(const QString& uuid);
    QJsonObject dhcpOptionFromCobnnection(const QJsonObject& connection, const QString keyFilter);
#endif
    QString get_network_type(const QNetworkInterface::InterfaceType type);
    QJsonObject get_data_from_win_registry(const QString& human_readable_name, const QJsonArray& ip_addresses_json_array);

    static QJsonArray string_list_to_json_array(const QStringList &string_list);
    static void append_json_object_to_json_object(const QJsonObject &source, QJsonObject &target);
    static HostResultType send_arp(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name={});
    static HostResultType send_arp_windows(const QString& dst_host_address, const QString& src_host_address);
    static HostResultType send_arp_linux(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name);

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
    return "networkDeviceAudit";
}

#endif // TASKPLUGIN_H
