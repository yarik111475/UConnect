#include <array>
#include <future>
#include <memory>
#include <QHostInfo>
#include <QByteArray>
#include <QTcpSocket>
#include <QHostAddress>

#ifdef Q_OS_WINDOWS
#include <winsock2.h>
#include <iphlpapi.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#endif

#include "NetworkUtils.h"
#include "snmp_pp/snmp_pp.h"

NetworkUtils::NetworkUtils()
{
    /* not used in current implementation
    port_map_.emplace(21,   "tcp");
    port_map_.emplace(22,   "ssh");
    port_map_.emplace(23,   "telnet");
    port_map_.emplace(25,   "smtp");
    port_map_.emplace(53,   "domain");
    port_map_.emplace(80,   "http");
    port_map_.emplace(81,   "hosts2-ns");
    port_map_.emplace(110,  "pop3");
    port_map_.emplace(111,  "rpcbind");
    port_map_.emplace(113,  "ident");
    port_map_.emplace(135,  "msrpc");
    port_map_.emplace(139,  "netbios-ssn");
    port_map_.emplace(143,  "imap");
    port_map_.emplace(179,   "bgp");
    port_map_.emplace(199,   "smux");
    port_map_.emplace(443,   "https");
    port_map_.emplace(445,   "microsoft-ds");
    port_map_.emplace(465,   "smtps");
    port_map_.emplace(514,   "shell");
    port_map_.emplace(548,   "afp");
    port_map_.emplace(587,   "submission");
    port_map_.emplace(993,   "imaps");
    port_map_.emplace(995,   "pop3s");
    port_map_.emplace(1025,  "NFS-or-IIS");
    port_map_.emplace(1026,  "LSA-or-nterm");
    port_map_.emplace(1720,  "h323q931");
    port_map_.emplace(1723,  "pptp");
    port_map_.emplace(2000,  "cisco-sccp");
    port_map_.emplace(3306,  "mysql");
    port_map_.emplace(3389,  "ms-wbt-server");
    port_map_.emplace(5060,  "sip");
    port_map_.emplace(5900,  "vnc");
    port_map_.emplace(6001,  "X11:1");
    port_map_.emplace(8080,  "http-proxy");
    port_map_.emplace(8443,  "https-alt");
    port_map_.emplace(8888,  "sun-answerbook");
    port_map_.emplace(10000, "snet-sensor-mgmt");
    */
}

HostResultType NetworkUtils::send_arp(const QString &dst_host_address, const QString &src_host_address, const QString &itf_name)
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

std::vector<TcpResultType> NetworkUtils::scan_tcp_ports(const std::string &host_address, const std::set<uint16_t> &ports)
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

SnmpResultType NetworkUtils::scan_snmp_oid_values_simple(const std::string &host_address, const std::vector<std::string> &standard_oids)
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
HostResultType NetworkUtils::send_arp_windows(const QString &dst_host_address, const QString &src_host_address)
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
HostResultType NetworkUtils::send_arp_linux(const QString &dst_host_address, const QString &src_host_address, const QString &itf_name)
{
    const auto& destInetAddrHostAddress {
        inet_addr(dst_host_address.toLocal8Bit().data())
    };
    const auto& srcInetAddrHostAddress {
        inet_addr(src_host_address.toLocal8Bit().data())
    };

//    const char *ifname = "enp0s5";
//    const char *ip = "192.168.31.101";

    char mac[256] { 0 };

    const auto& res = arpPing(ifName.toStdString().c_str()
                        , dstHostAddress.toStdString().c_str()
                        , mac);
    if (0 == res) {
//        qDebug() << "====================" << dstHostAddress << mac;
        return std::make_tuple (true
                            , dstHostAddress.toStdString()
                            , srcHostAddress.toStdString()
                            ,  QString {mac}.toStdString());
    }

    return std::make_tuple(false
                           , dstHostAddress.toStdString()
                           , srcHostAddress.toStdString()
                           , "");
}
#endif

std::map<uint16_t, std::string> NetworkUtils::port_map_;
