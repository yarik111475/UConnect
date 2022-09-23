#include <QtCore>

#include "TaskPlugin.h"

std::string TaskPlugin::get_snmp_syntax_string(SmiUINT32 syntax)
{
    std::string syntax_string {"undefined"};

    std::map<SmiUINT32, std::string> syntax_map {};
    syntax_map.emplace(sNMP_SYNTAX_INT, "int");
    syntax_map.emplace(sNMP_SYNTAX_BITS, "bits");
    syntax_map.emplace(sNMP_SYNTAX_OCTETS, "octets");
    syntax_map.emplace(sNMP_SYNTAX_NULL, "null");
    syntax_map.emplace(sNMP_SYNTAX_OID, "oid");
    syntax_map.emplace(sNMP_SYNTAX_INT32, "int32");
    syntax_map.emplace(sNMP_SYNTAX_IPADDR, "ipaddress");
    syntax_map.emplace(sNMP_SYNTAX_CNTR32, "string");
    syntax_map.emplace(sNMP_SYNTAX_GAUGE32, "gauge32");
    syntax_map.emplace(sNMP_SYNTAX_TIMETICKS, "timeticks");
    syntax_map.emplace(sNMP_SYNTAX_OPAQUE, "opaque");
    syntax_map.emplace(sNMP_SYNTAX_CNTR64, "cntr64");
    syntax_map.emplace(sNMP_SYNTAX_UINT32, "uint32");

    auto found=syntax_map.find(syntax);
    return (found==syntax_map.end()) ? syntax_string : (*found).second;
}

TaskItem TaskPlugin::parse_incoming_string(const std::string& incoming_string)
{
    TaskItem task_item {};
    TaskItemParams task_item_params {};
    std::vector<TaskItemTarget> targets {};

    QJsonObject input_json=QJsonDocument::fromJson(QString(incoming_string.c_str()).toUtf8()).object();
    //root json item
    if(input_json.isEmpty() || !input_json.contains("name") || !input_json.contains("additional_params")){
        return task_item;
    }

    //additional params json item
    QJsonObject additional_json=input_json["additional_params"].toObject();
    if(!additional_json.contains("connection") || !additional_json.contains("targets")){
        return task_item;
    }

    //connection json item
    QJsonObject connection_json=additional_json["connection"].toObject();
    if(!connection_json.contains("version") || !connection_json.contains("port") || !connection_json.contains("timeout") ||
       !connection_json.contains("retry") || !connection_json.contains("security")){
        return task_item;
    }

    //security json item
    QJsonObject security_json=connection_json["security"].toObject();

    //targets json array
    QJsonArray targets_json=additional_json["targets"].toArray();
    if(targets_json.isEmpty()){
        return task_item;
    }

    //fill from 'targets' section
    for(const QJsonValue& target_json: targets_json){
        if(!target_json.toObject().contains("ip_address") || !target_json.toObject().contains("oids") || !target_json.toObject()["oids"].isArray()){
            continue;
        }
        TaskItemTarget target_item;
        target_item.ip_address_=target_json.toObject()["ip_address"].toString().toStdString();
        QJsonArray oids_json=target_json["oids"].toArray();
        for(const QJsonValue& oid_json: oids_json){
            target_item.oids_.push_back(oid_json.toString().toStdString());
        }
        targets.push_back(target_item);
    }

    //fill from common 'additional_params/connections' section

    task_item_params.port_=connection_json["port"].toInt();
    task_item_params.retry_=connection_json["retry"].toInt();
    task_item_params.timeout_=connection_json["timeout"].toInt();
    task_item_params.version_=(connection_json["version"].toString()=="v1") ? snmp_version::version1 :
                              (connection_json["version"].toString()=="v2c") ? snmp_version::version2c :
                                                                               snmp_version::version3;
    //case for snmpv1 or snmpv2c
    if(task_item_params.version_==snmp_version::version1 || task_item_params.version_==snmp_version::version2c){
        task_item_params.community_=security_json["community"].toString().toStdString();
    }
    else{
        //fill 'security'
        QString s_l=security_json["level"].toString();
        task_item_params.security_level_=(s_l=="noAuthNoPriv") ? SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV :
                                         (s_l=="noAuthPriv") ? SNMP_SECURITY_LEVEL_AUTH_NOPRIV :
                                                               SNMP_SECURITY_LEVEL_AUTH_PRIV;
        task_item_params.secirity_model_=SNMP_SECURITY_MODEL_USM;
        task_item_params.username_=security_json["username"].toString().toStdString();

        //fill 'auth'
        QString a_p=security_json["auth_protocol"].toString();
        task_item_params.auth_protocol_=(a_p=="sha") ? SNMP_AUTHPROTOCOL_HMACSHA : SNMP_AUTHPROTOCOL_HMACMD5;
        task_item_params.auth_password_=security_json["auth_password"].toString().toStdString();

        //fill 'privacy
        QString p_p=security_json["privacy_protocol"].toString();
        task_item_params.priv_protocol_=(p_p=="des") ? SNMP_PRIVPROTOCOL_DES :
                                        (p_p=="aes128") ? SNMP_PRIVPROTOCOL_AES128 :
                                        (p_p=="aes192") ? SNMP_PRIVPROTOCOL_AES192 :
                                                          SNMP_PRIVPROTOCOL_AES256;
        task_item_params.priv_password_=security_json["privacy_password"].toString().toStdString();
        task_item_params.context_name_=security_json["context_name"].toString().toStdString();
        task_item_params.context_engine_id_=security_json["context_engine_id"].toString().toStdString();
    }

    //fill 'root'
    task_item.name_=input_json["name"].toString();
    task_item.task_item_params_=task_item_params;
    task_item.targets_=targets;
    return task_item;
}

QJsonArray TaskPlugin::snmp_get_request(const TaskItem &task_item)
{
    QJsonArray outcoming_array_data_json {};

    for(const TaskItemTarget& target: task_item.targets_){
        QJsonObject outcoming_data_json {};
        QJsonObject outcoming_response_json {};

        Snmp::socket_startup();
        int retry=task_item.task_item_params_.retry_;
        int timeout=task_item.task_item_params_.timeout_;
        snmp_version version=task_item.task_item_params_.version_;

        UdpAddress ip_address(target.ip_address_.c_str());
        if(!ip_address.valid()){
            outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
            outcoming_response_json["message"]="invalid address or dns name!";
            outcoming_data_json["response"]=outcoming_response_json;
            outcoming_array_data_json.push_back(outcoming_data_json);
            continue;
        }

        ip_address.set_port(task_item.task_item_params_.port_);
        int status {};

        Snmp snmp(status, 0, (ip_address.get_ip_version() == Address::version_ipv6));
        if (status != SNMP_CLASS_SUCCESS) {
            outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
            outcoming_response_json["message"]=QString("%1, %2").arg("snmp_pp session create fail!").arg(snmp.error_msg(status));
            outcoming_data_json["response"]=outcoming_response_json;
            outcoming_array_data_json.push_back(outcoming_data_json);
            continue;
        }

        CTarget ctarget(ip_address);
        UTarget utarget(ip_address);
        SnmpTarget *snmp_target {};
        v3MP *v3_MP {};
        Pdu pdu {};

        for(const std::string& oid: target.oids_){
            Vb vb {};
            vb.set_oid(oid.c_str());
            pdu+=vb;
        }

        //set params for snmpv1/snmpv2c
        if(task_item.task_item_params_.version_==version1 || task_item.task_item_params_.version_==version2c){
            OctetStr community {task_item.task_item_params_.community_.c_str()};
            // MUST create a dummy v3MP object if _SNMPv3 is enabled!
            int construct_status;
            v3_MP = new v3MP("dummy", 0, construct_status);

            ctarget.set_version(version);
            ctarget.set_retry(retry);
            ctarget.set_timeout(timeout);
            ctarget.set_readcommunity(community);
            snmp_target=&ctarget;
        }
        //set params for snmpv3
        else{
            OctetStr priv_password {task_item.task_item_params_.priv_password_.c_str()};
            OctetStr auth_password {task_item.task_item_params_.auth_password_.c_str()};

            int security_model {task_item.task_item_params_.secirity_model_};
            int security_level {task_item.task_item_params_.security_level_};


            OctetStr context_name {task_item.task_item_params_.context_name_.c_str()};
            OctetStr context_engine_id {task_item.task_item_params_.context_engine_id_.c_str()};
            OctetStr security_name {task_item.task_item_params_.username_.c_str()};

            long auth_protocol=task_item.task_item_params_.auth_protocol_;
            long priv_protocol=task_item.task_item_params_.priv_protocol_;

            //init snmpv3
            {
                const char *engineId = "snmpGet";
                const char *filename = "snmpv3_boot_counter";
                unsigned int snmpEngineBoots = 0;
                int status{};

                status = getBootCounter(filename, engineId, snmpEngineBoots);
                if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
                {
                    outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
                    outcoming_response_json["message"]=QString("%1: %2").arg("error loading snmp_engine_boots counter").arg(status);
                    outcoming_data_json["response"]=outcoming_response_json;
                    outcoming_array_data_json.push_back(outcoming_data_json);
                    continue;
                }
                snmpEngineBoots++;
                status = saveBootCounter(filename, engineId, snmpEngineBoots);
                if (status != SNMPv3_OK)
                {
                    outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
                    outcoming_response_json["message"]=QString("%1: %2").arg("error saving snmp_engine_boots counter").arg(status);
                    outcoming_data_json["response"]=outcoming_response_json;
                    outcoming_array_data_json.push_back(outcoming_data_json);
                    continue;
                }

                int construct_status{};
                v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);
                if (construct_status != SNMPv3_MP_OK)
                {
                    outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
                    outcoming_response_json["message"]=QString("%1: %2").arg("error initializing snmpv3").arg(construct_status);
                    outcoming_data_json["response"]=outcoming_response_json;
                    outcoming_array_data_json.push_back(outcoming_data_json);
                    continue;
                }

                USM *usm = v3_MP->get_usm();
                usm->add_usm_user(security_name,auth_protocol, priv_protocol,auth_password, priv_password);
            }

            utarget.set_version(version);
            utarget.set_retry(retry);
            utarget.set_timeout(timeout);
            utarget.set_security_model(security_model);
            utarget.set_security_name(security_name);
            pdu.set_security_level(security_level);
            pdu.set_context_name (context_name);
            pdu.set_context_engine_id(context_engine_id);
            snmp_target=&utarget;
        }

        snmp.set_mpv3(v3_MP);
        status = snmp.get(pdu, *snmp_target);
        if (status == SNMP_CLASS_SUCCESS){
            for(int i=0;i<pdu.get_vb_count();++i){
                Vb vb=pdu.get_vb(i);

                outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
                outcoming_response_json["oid"]=QString(vb.get_printable_oid());
                outcoming_response_json["type"]=QString(get_snmp_syntax_string(vb.get_syntax()).c_str());
                outcoming_response_json["value"]=QString(vb.get_printable_value());
                outcoming_data_json["response"]=outcoming_response_json;
                outcoming_array_data_json.push_back(outcoming_data_json);
            }
        }
        else{
            outcoming_data_json["ip_address"]=QString(target.ip_address_.c_str());
            outcoming_response_json["message"]=QString(snmp.error_msg( status));
            outcoming_data_json["response"]=outcoming_response_json;
            outcoming_array_data_json.push_back(outcoming_data_json);
        }
        Snmp_pp::Snmp::socket_cleanup();
    }
    return outcoming_array_data_json;
}

int TaskPlugin::pluginVersion() const
{
    return 0;
}

void TaskPlugin::handle_impl(const std::string &incoming_string, std::string &outcoming_string)
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

    TaskItem task_item=parse_incoming_string(incoming_string);
    if(task_item.empty()){
        throw std::runtime_error("Incoming parameters emty or invalid!");
    }
    else{
        outcoming_string=QJsonDocument(snmp_get_request(task_item)).toJson().toStdString();
    }
}

