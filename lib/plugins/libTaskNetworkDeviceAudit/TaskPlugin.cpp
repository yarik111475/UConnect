#include <QHostInfo>
#include <QSettings>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkInterface>

#ifdef Q_OS_WINDOWS
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef Q_OS_LINUX
#include "unix/Arp.h"
#endif

#include "TaskPlugin.h"


QString TaskPlugin::get_network_type(const QNetworkInterface::InterfaceType type){
    switch (type) {
    case QNetworkInterface::Loopback:
        return "loopback";
    case QNetworkInterface::Virtual:
        return "virtual";
    case QNetworkInterface::Ethernet:
        return "ethernet";
    case QNetworkInterface::Wifi:
        return "wifi";
    case QNetworkInterface::CanBus:
        return "canbus";
    case QNetworkInterface::Fddi:
        return "fddi";
    case QNetworkInterface::Ppp:
        return "ppp";
    case QNetworkInterface::Slip:
        return "slip";
    case QNetworkInterface::Phonet:
        return "phonet";
    case QNetworkInterface::Ieee802154:
        return "ieee802154";
    case QNetworkInterface::SixLoWPAN:
        return "sixlowpan";
    case QNetworkInterface::Ieee80216:
        return "ieee80216";
    case QNetworkInterface::Ieee1394:
        return "ieee1394";
    default:
        return "unknown";
    }
}

QJsonObject TaskPlugin::get_data_from_win_registry(const QString &human_readable_name, const QJsonArray &ip_addresses_json_array){
    QJsonObject json_object {}; //result
    const QString& interfaces_uuid_path="HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}";


    {//serach uuid of interface by human readable name
        const QSettings registry(interfaces_uuid_path, QSettings::NativeFormat);
        const QStringList& groups { registry.childGroups() };

        for (const QString& uuid_interface: groups) {
            const QString& interface_path="HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\"+
                    uuid_interface +
                    "\\Connection";

            const QSettings& registry2 { interface_path, QSettings::NativeFormat };

            if (registry2.value("name").toString()==human_readable_name) {
                {//ipv4
                    const QString& interface_info_path="HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\" + uuid_interface;
                    const QSettings iface(interface_info_path, QSettings::NativeFormat);

                    if (iface.value("EnableDHCP").toBool()) {
                        const QJsonArray& income_gatway_json_array=iface.value("DhcpDefaultGateway").toJsonArray();
                        QHash<QString, QString> map_gateway_and_mac;

                        for (const auto& item_gateway: income_gatway_json_array) {

                            for (const auto& item_ip_address: ip_addresses_json_array) {
                                QHostAddress src_addr { item_ip_address.toString() };
                                if (src_addr.protocol()!=QAbstractSocket::IPv4Protocol) {
                                    continue;
                                }

                                //1st insert empty-mac item
                                map_gateway_and_mac.insert(item_gateway.toString(), "");

                                const HostResultType host_result_type=TaskPlugin::send_arp(item_gateway.toString(), item_ip_address.toString());

                                if (host_result_type.is_up_) {
                                    const QString& mac_address=QString::fromStdString(host_result_type.mac_address_);

                                    //2nd replace
                                    map_gateway_and_mac.insert(item_gateway.toString(), mac_address);
                                    break;
                                }
                            }
                        }

                        QJsonArray result_gatetway_json_array;

                        for (auto it=map_gateway_and_mac.begin();it != map_gateway_and_mac.end(); it++) {
                            QJsonObject gateway_and_mac {};
                            if (!it.key().isEmpty()){
                                gateway_and_mac.insert("ipAddress", it.key());
                            }
                            if (!it.value().isEmpty()){
                                gateway_and_mac.insert("macAddress", it.value());
                            }
                            result_gatetway_json_array.append(gateway_and_mac);
                        }

                        json_object.insert("ipv4Gateway", result_gatetway_json_array);
                        json_object.insert("dhcp4Enabled", true);

                        QStringList server_name_list=iface.value("DhcpNameServer").toString().split(',');
                        QJsonArray server_name_json;
                        for(const QString& server_name: server_name_list){
                            server_name_json.append(server_name);
                        }
                        json_object.insert("ipv4Dns", server_name_json);
                        json_object.insert("dhcp4Server", iface.value("DhcpServer").toString());
                    }
                    else {
                        const QJsonArray& income_gateway_json_array=iface.value("DefaultGateway").toJsonArray();
                        QHash<QString, QString> map_gateway_and_mac;
                        for (const auto& item_gateway: income_gateway_json_array) {

                            for (const auto& item_ip_address: ip_addresses_json_array) {
                                QHostAddress src_addr(item_ip_address.toString());
                                if (QAbstractSocket::IPv4Protocol != src_addr.protocol()) {
                                    continue;
                                }

                                //1st insert empty-mac item
                                map_gateway_and_mac.insert(item_gateway.toString(), "");

                                const HostResultType& host_result_type=TaskPlugin::send_arp(item_gateway.toString(), item_ip_address.toString());

                                if (host_result_type.is_up_) {
                                    const auto& mac_address=QString::fromStdString(host_result_type.mac_address_);

                                    //2nd replace
                                    map_gateway_and_mac.insert(item_gateway.toString(), mac_address);
                                    break;
                                }
                            }
                        }

                        QJsonArray result_gatetway_json_array;

                        for (auto it=map_gateway_and_mac.begin();it != map_gateway_and_mac.end(); it++) {
                            QJsonObject gateway_and_mac;
                            if (!it.key().isEmpty()){
                                gateway_and_mac.insert("ipAddress", it.key());
                            }
                            if (!it.value().isEmpty()){
                                gateway_and_mac.insert("macAddress", it.value());
                            }
                            result_gatetway_json_array.append(gateway_and_mac);
                        }

                        json_object.insert("dhcp4Enabled", false);
                        json_object.insert("ipv4Gateway", result_gatetway_json_array);

                        QStringList server_name_list=iface.value("NameServer").toString().split(',');
                        QJsonArray server_name_json;
                        for(const QString& server_name: server_name_list){
                            server_name_json.append(server_name);
                        }
                        json_object.insert("ipv4Dns", server_name_json);
                    }
                }


                {//ipv6
                    const QString& interface_info_path="HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip6\\Parameters\\Interfaces\\" + uuid_interface;
                    const QSettings iface(interface_info_path, QSettings::NativeFormat);

                    if (iface.value("EnableDHCP").toBool()) {
                        const QString& defaultGateway { iface.value("DhcpDefaultGateway").toString() };
                        json_object.insert("dhcp6Enabled", true);

                        QJsonArray ipv6GatewayJA;
                        QJsonObject ipv6Gateway;
                        if (!defaultGateway.isEmpty()) ipv6Gateway.insert("ipAddress", defaultGateway);
                        //                        ipv6GatewayJson.insert("macAddress", "");
                        if (!ipv6Gateway.isEmpty()) ipv6GatewayJA.append(ipv6Gateway);
                        json_object.insert("ipv6Gateway", ipv6GatewayJA);
                        json_object.insert("ipv6Dns", TaskPlugin::string_list_to_json_array(iface.value("NameServer").toString().split(','))); //array
                        json_object.insert("dhcp6Server", iface.value("DhcpServer").toString());
                    }
                    else {
                        const QString& defaultGateway { iface.value("DefaultGateway").toString() };

                        json_object.insert("dhcp6Enabled", false);

                        QJsonArray ipv6GatewayJA;
                        QJsonObject ipv6Gateway;
                        if (!defaultGateway.isEmpty()) ipv6Gateway.insert("ipAddress", defaultGateway);
                        //                        ipv6GatewayJson.insert("macAddress", "");
                        if (!ipv6Gateway.isEmpty()) ipv6GatewayJA.append(ipv6Gateway);
                        json_object.insert("ipv6Gateway", ipv6GatewayJA);

                        json_object.insert("ipv6Dns",TaskPlugin::string_list_to_json_array(iface.value("NameServer").toString().split(',')));
                    }
                }
            }
        }
    }
    return json_object;
}

QJsonArray TaskPlugin::string_list_to_json_array(const QStringList &string_list)
{
    QJsonArray json_array;
    for (const auto& item: string_list) {
        json_array.append(item);
    }
    return json_array;
}

void TaskPlugin::append_json_object_to_json_object(const QJsonObject &source, QJsonObject &target)
{
    for (auto&& it=source.begin(); it != source.end(); it++  ) {
        if (!target.contains(it.key())) { //prevent replace exists item
            target.insert(it.key(), it.value());
        }
    }
}

HostResultType TaskPlugin::send_arp(const QString &dst_host_address, const QString &src_host_address, const QString &itf_name)
{
    HostResultType host_result_type {};
#if defined(Q_OS_WIN)
    host_result_type = send_arp_windows(dst_host_address, src_host_address);
#endif

#if defined(Q_OS_LINUX)
    host_result_type = send_arp_linux(dst_host_address, src_host_address, itf_name);
#endif
    return host_result_type;
}

#ifdef Q_OS_WINDOWS
HostResultType TaskPlugin::send_arp_windows(const QString &dst_host_address, const QString &src_host_address)
{
    HostResultType host_result_type {};
    DWORD dw_ret_val;
    ULONG mac_addr[2];          /* for 6-byte hardware addresses */
    ULONG phys_addr_len { 6 };  /* default to length of six bytes */

    const auto& dest_inet_addr_host_address=inet_addr(dst_host_address.toLocal8Bit().data());
    const auto& src_inet_addr_host_address=inet_addr(src_host_address.toLocal8Bit().data());

    memset(&mac_addr, 0xff, sizeof (mac_addr));

    dw_ret_val = SendARP(dest_inet_addr_host_address,
                         src_inet_addr_host_address,
                         &mac_addr, &phys_addr_len);

    if (dw_ret_val == NO_ERROR) {
        QString result_mac_address;
        BYTE *b_phys_addr;
        b_phys_addr = (BYTE *) & mac_addr;
        if (phys_addr_len) {
            QByteArray ba {};
            for (int i{0}; i < static_cast<int>(phys_addr_len); i++ ) {
                ba.append(b_phys_addr[i]);
            }

            result_mac_address = std::move(QString { ba.toHex(':') });
        }
        host_result_type.is_up_=true;
        host_result_type.dst_host_address_=dst_host_address.toStdString();
        host_result_type.src_host_address_=src_host_address.toStdString();
        host_result_type.mac_address_=result_mac_address.toStdString();
        QHostInfo host_info=QHostInfo::fromName(dst_host_address);
        host_result_type.host_name_=(host_info.hostName()!=dst_host_address) ?
                                    host_info.hostName().toStdString() :
                                    std::string{"null"};
    }
    return host_result_type;
}
#endif

#ifdef Q_OS_LINUX
HostResultType TaskPlugin::send_arp_linux(const QString &dst_host_address, const QString &src_host_address, const QString &itf_name)
{
    HostResultType host_result_type {};

    char mac[256] { 0 };

    const auto& res = arpPing(itf_name.toStdString().c_str(), dst_host_address.toStdString().c_str(), mac);
    if (0 == res) {
        host_result_type.is_up_=true;
        host_result_type.dst_host_address_=dst_host_address.toStdString();
        host_result_type.src_host_address_=src_host_address.toStdString();
        host_result_type.mac_address_=QString(mac).toStdString();
    }
    return host_result_type;
}
#endif

TaskPlugin::TaskPlugin()
{
}

int TaskPlugin::pluginVersion() const
{
    return 0;
}

void TaskPlugin::handle_impl(const std::string &incoming_string, std::string &outcoming_string)
{
    Q_UNUSED(incoming_string)
    QJsonArray outcoming_json_array {};
    const QList<QNetworkInterface> all_interfaces=QNetworkInterface::allInterfaces();
    for(const QNetworkInterface& itf: all_interfaces){
        if(itf.type()==QNetworkInterface::Loopback){
            continue;
        }

        const QString itf_name{itf.name()};
        const QString human_readable_name {itf.humanReadableName()};

        QJsonArray ip_addresses_json_array {};
        QJsonArray ip_addresses_without_netmask_json_array {};

        for(const QNetworkAddressEntry& address_entry: itf.addressEntries()){
            ip_addresses_json_array.append(address_entry.ip().toString() + "/" +QString::number(address_entry.prefixLength()));
            ip_addresses_without_netmask_json_array.append(address_entry.ip().toString());
        }

        QJsonObject additional_info_json {};
#ifdef Q_OS_WINDOWS
        additional_info_json=get_data_from_win_registry(human_readable_name, ip_addresses_without_netmask_json_array);
#endif
#ifdef Q_OS_LINUX
#endif
        QJsonObject top_json_object {};
        top_json_object["ipAddresses"]=ip_addresses_json_array;
        top_json_object["macAddress"]=itf.hardwareAddress();
        top_json_object["name"]=itf_name;
        top_json_object["humanReadableName"]=human_readable_name;
        top_json_object["type"]=get_network_type(itf.type());
        TaskPlugin::append_json_object_to_json_object(additional_info_json, top_json_object);
        outcoming_json_array.append(top_json_object);
    }

    outcoming_string=QJsonDocument(outcoming_json_array).toJson().toStdString();
}
