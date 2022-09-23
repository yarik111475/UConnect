#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <set>
#include <map>
#include <vector>
#include <QString>
#include "include/Defines.h"

class NetworkUtils
{
private:
    static std::map<uint16_t, std::string> port_map_;
public:
    NetworkUtils();
    ~NetworkUtils()=default;

    static HostResultType send_arp(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name={});
    static HostResultType send_arp_windows(const QString& dst_host_address, const QString& src_host_address);
    static HostResultType send_arp_linux(const QString& dst_host_address, const QString& src_host_address, const QString& itf_name);

    static std::vector<TcpResultType> scan_tcp_ports(const std::string &host_address, const std::set<uint16_t>& ports);
    static SnmpResultType scan_snmp_oid_values_simple(const std::string &host_address, const std::vector<std::string> &standard_oids);
};


#endif // NETWORKUTILS_H
