#ifndef DEFINES_H
#define DEFINES_H

#include <map>
#include <string>
#include <QString>

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

#endif // DEFINES_H
