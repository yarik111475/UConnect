#ifndef TASKITEM_H
#define TASKITEM_H

#include <vector>
#include <string>
#include <QString>
#include "snmp_pp/snmp_pp.h"

/* input json format
{
  "agent_id": "c5699db8-1654-44c5-bd40-b5b88fe74dbd",
  "name": "processMonitoring",
  "additional_params": {
    "connection": {
      "version": "v3",  //v1/v2c/v3
      "port": 161,
      "timeout": 30,
      "retry": 5,
      "security": {
        "level": "noAuthNoPriv",  // noAuthNoPriv/noAuthPriv/authPriv for version=v3
        "username": "any_str",
        "auth_protocol": "md5",    //md5/sha, // optional for level=noAuthNoPriv/noAuthPriv
        "auth_password": "any_str",  // optional for level=noAuthNoPriv/noAuthPriv
        "privacy_protocol": "des",   //des/aes128/aes192/aes256  // optional for level=noAuthNoPriv
        "privacy_password": "any_str",  // optional for level=noAuthNoPriv
        "context_name": "any_str",  // optional
        "context_engine_id": "any_str", // optional
        "community": "any_str"
      }
    },
    "targets": [
      {
        "ip_address": "127.0.0.1",
        "oids": ["1.3.6.1.4.1.641.2.1.5.2", "1.3.6.1.4.1.641.2.1.5.3"]
      }
    ]
  }
}

    OctetStr priv_password {"test1q2w3e"};
    OctetStr auth_password {"test1q2w3e"};

    int security_model {SNMP_SECURITY_MODEL_USM};
    int security_level {SNMP_SECURITY_LEVEL_AUTH_PRIV};

    OctetStr community {"public"};
    OctetStr context_name {"Jetdirect"};
    OctetStr context_engine_id {};
    OctetStr security_name {"admin"};

    long auth_protocol=SNMP_AUTHPROTOCOL_HMACMD5;
    long priv_protocol=SNMP_PRIVPROTOCOL_DES;

*/

using namespace Snmp_pp;

class TaskItemTarget{
public:
    std::string ip_address_ {};
    std::vector<std::string> oids_ {};

    TaskItemTarget()=default;
    ~TaskItemTarget()=default;

    inline bool empty()const{
        return ip_address_.empty() || oids_.empty();
    }
};

class TaskItemParams{
public:
    //common params
    Snmp_pp::snmp_version version_ {};
    int port_ {};
    int timeout_;
    int retry_ {};
    std::string community_ {};

    //securyty params
    int security_level_ {};
    int secirity_model_ {};
    std::string username_ {};

    //auth params
    long auth_protocol_ {};
    std::string auth_password_ {};

    //privacy params
    long priv_protocol_ {};
    std::string priv_password_ {};

    //optional params
    std::string context_name_ {};
    std::string context_engine_id_ {};

    inline bool empty()const{
        if(version_==version1 || version_==version2c){
            return port_==0 || community_.empty();
        }
        else{
            return port_==0 || username_.empty() || auth_password_.empty();
        }
    }
};

class TaskItem{
public:
    QString name_ {};
    TaskItemParams task_item_params_ {};
    std::vector<TaskItemTarget> targets_ {};

    TaskItem()=default;
    ~TaskItem()=default;
    inline bool empty()const{
        return  name_.isEmpty() ||
                task_item_params_.empty() || targets_.empty();
    }
};

#endif // TASKITEM_H
