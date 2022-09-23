#include <set>
#include <list>
#include <array>
#include <future>
#include <QtGlobal>
#include <QTcpSocket>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>

#ifdef Q_OS_WINDOWS
#include <winsock2.h>
#include <iphlpapi.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef Q_OS_LINUX
#include "unix/Arp.h"
#endif

#include "TaskPlugin.h"
#include "snmp_pp/snmp_pp.h"

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

std::vector<TcpResultType> TaskPlugin::scan_tcp_ports(const std::string &host_address, const std::set<uint16_t> &ports)
{
    const int delay_interval {500};
    std::vector<TcpResultType> tcp_result_list{};
    std::unique_ptr<QTcpSocket> tcp_socket_ptr(new QTcpSocket);

    for(uint16_t port: ports){
        tcp_socket_ptr->connectToHost(QHostAddress(QString::fromStdString(host_address)), port);
        tcp_socket_ptr->waitForConnected(delay_interval);
        if(tcp_socket_ptr->state()!=QAbstractSocket::ConnectedState){
            continue;
        }
        else if(tcp_socket_ptr->state()==QAbstractSocket::ConnectedState){
            TcpResultType tcp_result_type {};
            tcp_result_type.port_=port;
            tcp_socket_ptr->waitForReadyRead(100);
            QByteArray content=tcp_socket_ptr->readAll();
            if(!content.isEmpty()){
                tcp_result_type.answer_=content.left(512).toStdString();
            }
            tcp_socket_ptr->disconnectFromHost();
            if(tcp_socket_ptr->state()!=QAbstractSocket::UnconnectedState){
                tcp_socket_ptr->waitForDisconnected(delay_interval);
            }

            /* not used in current implementation
            auto found=NetworkUtils::port_map_.find(port);
            tcp_result_type.type_=(found==NetworkUtils::port_map_.end()) ? "not defined" : found->second;
            */

            tcp_result_list.push_back(tcp_result_type);
        }
    }
    return tcp_result_list;
}

SnmpResultType TaskPlugin::scan_snmp_oid_values_simple(const std::string &host_address, const std::vector<std::string> &standard_oids)
{
    using namespace Snmp_pp;
    //set log profile to 'off' for snmp
    DefaultLog::log()->set_profile("off");
    //set log filter for snmp
    DefaultLog::log()->set_filter(ERROR_LOG, 0);
    DefaultLog::log()->set_filter(WARNING_LOG, 0);
    DefaultLog::log()->set_filter(EVENT_LOG, 0);
    DefaultLog::log()->set_filter(INFO_LOG, 0);
    DefaultLog::log()->set_filter(DEBUG_LOG, 0);

    SnmpResultType snmp_result {};
    const int retry {1};
    const int timeout {100};
    const int port {161};
    const OctetStr community {"public"};
    std::array<snmp_version, 2> snmp_versions {snmp_version::version2c, snmp_version::version1};


    Snmp::socket_startup();
    UdpAddress ip_address(host_address.c_str());
    if(!ip_address.valid()){
        return snmp_result;
        //throw std::runtime_error("invalid address or dns name!");
    }

    ip_address.set_port(port);
    int status {};

    Snmp snmp(status, 0, (ip_address.get_ip_version() == Address::version_ipv6));
    if (status != SNMP_CLASS_SUCCESS) {
        return snmp_result;
        //throw std::runtime_error("snmp_pp session create fail! " + std::string(snmp.error_msg(status)));
    }

    CTarget ctarget(ip_address);
    SnmpTarget *snmp_target {};
    Pdu pdu {};

    for(const std::string& oid: standard_oids){
        Vb vb {};
        vb.set_oid(oid.c_str());
        pdu+=vb;
    }

    ctarget.set_retry(retry);
    ctarget.set_timeout(timeout);
    ctarget.set_readcommunity(community);
    snmp_target=&ctarget;

    for(snmp_version& version: snmp_versions){
        ctarget.set_version(version);

        status = snmp.get(pdu, *snmp_target);
        if (status == SNMP_CLASS_SUCCESS){
            snmp_result.enabled_=true;
            snmp_result.version_=(version==snmp_version::version1) ? "v1" : "v2c";
            for(int i=0;i<pdu.get_vb_count();++i){
                Vb vb=pdu.get_vb(i);
                snmp_result.oids_values_.emplace(std::string(vb.get_printable_oid()),
                                                 std::string(vb.get_printable_value()));
            }
            break;
        }
    }
    Snmp_pp::Snmp::socket_cleanup();
    return snmp_result;
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
            std::stringstream ss {};
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

TaskItem TaskPlugin::parse_incoming_string(const std::string &incoming_string)
{
    TaskItem task_item {};
    const QJsonObject& incoming_json=QJsonDocument::fromJson(QString::fromStdString(incoming_string).toUtf8()).object();
    const QJsonObject& additional_params=incoming_json["additional_params"].toObject();

    //checks
    if(!additional_params["if_names"].isArray() || !additional_params["additional_tcp_ports"].isArray()){
        throw std::runtime_error("'if_names' or 'additional_tcp_ports' is empty or invalid!");
    }
    QJsonArray if_names_array_json=additional_params["if_names"].toArray();
    if(if_names_array_json.size() <=0){
        throw std::runtime_error("'if_names' is empty!");
    }

    for(const QJsonValue& if_name: if_names_array_json){
        task_item.if_names_.emplace(if_name.toString());
    }

    QJsonArray tcp_ports_array_json=additional_params["additional_tcp_ports"].toArray();
    for(const QJsonValue& tcp_port: tcp_ports_array_json){
        task_item.tcp_ports_.emplace(tcp_port.toInt());
    }
    return task_item;
}

TaskPlugin::TaskPlugin()
{
}

int TaskPlugin::pluginVersion() const
{
    return 0;
}

void TaskPlugin::handle_impl(const std::string &incoming_string, std::string &outcoming_string)
{
    TaskItem task_item=parse_incoming_string(incoming_string);
    std::vector<std::string> standard_oids {".1.3.6.1.2.1.1.1.0", ".1.3.6.1.2.1.1.4.0", ".1.3.6.1.2.1.1.5.0", ".1.3.6.1.2.1.1.6.0"};
    QJsonObject outcoming_json {};
    QJsonArray host_array_json {};

    for(const auto& itf_name: task_item.if_names_){
        QNetworkInterface itf=QNetworkInterface::interfaceFromName(itf_name);
        if(!itf.isValid()){
            continue;
        }

        outcoming_json.insert("ifName", itf_name);
        outcoming_json.insert("isUp", (itf.flags() & QNetworkInterface::IsUp) ? true : false);

        QList<QNetworkAddressEntry> all_entries=itf.addressEntries();
        for(const QNetworkAddressEntry& entry: all_entries){
            if(entry.ip().isNull() || entry.ip().protocol()!=QAbstractSocket::IPv4Protocol){
                continue;
            }

            const quint32 entry_ip=entry.ip().toIPv4Address();
            const quint32 entry_mask=entry.netmask().toIPv4Address();
            const quint32 network_address=entry_ip & entry_mask;
            const quint32 inverted_mask=~entry_mask;
            const quint32 broadcast_address=network_address | inverted_mask;

            std::list<std::future<HostResultType>> host_future_list {};
            std::launch launch_type=std::launch::async;
#ifdef Q_OS_LINUX
            launch_type=std::launch::deferred;
#endif
            for(quint32 i=network_address; i<broadcast_address; ++i){
                QHostAddress address( i);
                host_future_list.emplace_back(std::async(launch_type,
                                                     std::ref(TaskPlugin::send_arp),
                                                     address.toString(),
                                                     entry.ip().toString(),
                                                     itf_name));
            }

            for(std::future<HostResultType>& host_future: host_future_list){
                HostResultType host_result=host_future.get();
                if(host_result.is_up_){
                    QJsonObject host_info_json {};
                    QJsonArray ports_array_json {};
                    host_info_json["hostName"]=QString::fromStdString(host_result.host_name_);
                    host_info_json["isUp"] = host_result.is_up_;
                    host_info_json["ip"] = QString::fromStdString(host_result.dst_host_address_);
                    host_info_json["macAddress"] = QString::fromStdString(host_result.mac_address_);
                    host_info_json["ipVersion"] = "IPv4";

                    //tcp discovery
                    if(task_item.tcp_ports_.empty()){
                        host_info_json["ports"]=ports_array_json;
                    }
                    else{
                        std::vector<TcpResultType> tcp_result_list=TaskPlugin::scan_tcp_ports(host_result.dst_host_address_, task_item.tcp_ports_);
                        if(!tcp_result_list.empty()){
                            for(const TcpResultType& tcp_result: tcp_result_list){
                                QJsonObject port_json{};
                                port_json["port"]=QString::fromStdString(std::to_string(tcp_result.port_));
                                port_json["type"]=QString::fromStdString(tcp_result.type_);
                                port_json["answer"]=QString::fromStdString(tcp_result.answer_);

                                ports_array_json.append(port_json);
                            }
                        }

                        host_info_json["ports"]=ports_array_json;
                    }

                    //snmp dicovery
                    SnmpResultType snmp_result=TaskPlugin::scan_snmp_oid_values_simple(host_result.dst_host_address_, standard_oids);
                    if(!snmp_result.enabled_){
                        host_info_json["isSnmp"]="false";
                    }
                    else{
                        host_info_json["isSnmp"]="true";
                        host_info_json["isSnmpPublic"]="true";
                        host_info_json["snmpVer"]=QString::fromStdString(snmp_result.version_);

                        QJsonObject snmp_oids_json {};
                        for(const std::pair<std::string,std::string>& pair: snmp_result.oids_values_){
                            snmp_oids_json.insert(QString::fromStdString(pair.first), QString::fromStdString(pair.second));
                        }
                        host_info_json["snmpResults"]=snmp_oids_json;
                    }
                    host_array_json.append(host_info_json);
                }
            }
        }
    }
    outcoming_json.insert("hosts", host_array_json);
    outcoming_string=QJsonDocument(outcoming_json).toJson().toStdString();
}
