#include <cmath>
#include <QUuid>
#include <QFile>
#include <QProcess>
#include <QtNetwork>
#include <QWebSocket>
#include <QEventLoop>
#include <QtGlobal>
#include <QSslCertificate>

#include "WebsocketClient.h"
#include "spdlog/spdlog.h"

std::vector<TaskItem> WebsocketClient::parse_incoming_json(const QJsonObject &incoming_json)
{
    std::vector<TaskItem> task_list {};

    if(!incoming_json.contains("items")){
        return task_list;
    }

    QJsonArray root_json_array=incoming_json.value("items").toArray();
    if(root_json_array.empty()){
        return task_list;
    }
    for(const QJsonValue& root_json_item: root_json_array){
        if(root_json_item.toObject().contains("id") && root_json_item.toObject().contains("name") && root_json_item.toObject().contains("additional_params")){

            TaskItem task_item_obj(root_json_item.toObject().value("id").toString(),root_json_item.toObject().value("name").toString());
            task_item_obj.setType(task_item_obj.m_name=="download" ? TaskItem::DOWNLOAD : task_item_obj.m_name=="executefile" ?
                                                               TaskItem::EXECUTE_FILE : TaskItem::EXECUTE_PROGRAMM);

            QJsonObject params_json=root_json_item.toObject().value("additional_params").toObject();
            TaskParamsItem task_params_item_obj(params_json.value("url").toString(),params_json.value("hash").toString(),
                                             params_json.value("filename").toString(),params_json.value("programm").toString());

            QJsonArray arguments_json=params_json.value("arguments").toArray();
            for(const QJsonValue& argument : arguments_json){
                task_params_item_obj.m_arguments.push_back(argument.toString());
            }

            task_item_obj.set_params_item(task_params_item_obj);
            task_list.push_back(task_item_obj);
        }
    }
    return task_list;
}

bool WebsocketClient::init_ssl_configuration()
{
    bool ok {true};
    QString clientCertPath {};
#ifdef Q_OS_WINDOWS
    clientCertPath=qEnvironmentVariable("programfiles(x86)") + "/USystem/agentServer/etc/usagent/cert";
#endif
#ifdef Q_OS_UNIX
#endif

    const QString caFileName {"cacert.pem"};
    const QString keyFileName {"clientPrivateKey.pem"};
    const QString certFileName {"clientCert.pem"};
    try{
        m_ssl_configuration_ptr.reset(new QSslConfiguration(QSslConfiguration::defaultConfiguration()));
        m_ssl_configuration_ptr->setProtocol(QSsl::AnyProtocol);
        m_ssl_configuration_ptr->setPeerVerifyMode(QSslSocket::VerifyPeer);

        //init certificates list
        QFile fileCa(clientCertPath + "/" + caFileName);
        if(fileCa.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&fileCa);
            m_ssl_configuration_ptr->setCaCertificates(listOfCert);
        }
        else{
            ok=false;
        }

        //init private key
        QFile fileKey(clientCertPath + "/" + keyFileName);
        if(fileKey.open(QIODevice::ReadOnly)){
            QSslKey key (&fileKey, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey,QByteArray());
            m_ssl_configuration_ptr->setPrivateKey(key);
        }
        else{
            ok=false;
        }

        //init client certificates list
        QFile fileCrt(clientCertPath + "/" + certFileName);
        if(fileCrt.open(QIODevice::ReadOnly)){
            QList<QSslCertificate> listOfCert=QSslCertificate::fromDevice(&fileCrt);
            m_ssl_configuration_ptr->setLocalCertificateChain(listOfCert);
        }
        else{
            ok=false;
        }

    }
    catch(std::exception& ex){
        //TODO log exception error here
        ok=false;
        m_error=QString("WsClient::initSslConfiguration() error, description: %1").arg(ex.what());
    }
    catch(...){
        //TODO log common error here
        ok=false;
        m_error="WsClient::initSslConfiguration() unknown error";
    }
    return ok;
}

bool WebsocketClient::init_machine_uid()
{
    QString client_cert_path {};
    QString client_cert_name {"clientCert.pem"};

#ifdef Q_OS_WINDOWS
    client_cert_path=qEnvironmentVariable("programfiles(x86)") + "/USystem/agentServer/etc/usagent/cert";
#endif
#ifdef Q_OS_UNIX
#endif

    QFile in(client_cert_path + "/" + client_cert_name);
    if(in.open(QIODevice::ReadOnly)){
        QSslCertificate client_cert(&in);
        in.close();
        if(!client_cert.isNull()){
            QStringList subject_info_list=client_cert.subjectInfo(QSslCertificate::CommonName);
            if(!subject_info_list.empty()){
                m_uid=QString("%1").arg(subject_info_list.at(0));

                QUrlQuery query;
                query.addQueryItem("id", m_uid);
                m_ws_url.setQuery(query);
                return true;
            }
        }
    }
    return false;
}

void WebsocketClient::execute_task(const TaskItem &incoming_task)
{
    if(m_logger_ptr){
        m_logger_ptr->debug("start execute task with id: {} and name: {}",
                           incoming_task.m_id.toStdString(),incoming_task.m_name.toStdString());
    }

    switch(incoming_task.m_type){
    case TaskItem::DOWNLOAD:
        download(incoming_task);
        break;
    case TaskItem::EXECUTE_FILE:
        execute_file(incoming_task);
        break;
    case TaskItem::EXECUTE_PROGRAMM:
        execute_programm(incoming_task);
        break;
    default:
        if(m_logger_ptr){
            m_logger_ptr->debug("found unknown task name: {} for task with id: {}",
                               incoming_task.m_name.toStdString(),incoming_task.m_id.toStdString());
        }
    }
}

void WebsocketClient::execute_programm(const TaskItem &incomingTask)
{
    /*
    QJsonArray args;
    QStringList listArg;
    QString programm;

    try{
        //set programm
        if(!incominJson.contains("programm")){
            sendResponse("incoming json field 'programm' is empty!");

            if(spdlog::get(m_logName)){
                spdlog::get(m_logName)->debug("incoming json field 'programm' is empty!");
            }
            return;
        }
        programm=incominJson["programm"].toString();

        //set arguments (if presents)
        if(incominJson.contains("arguments")){
            QJsonValue arguments=incominJson["arguments"];
            if(arguments.isArray()){
                args=arguments.toArray();
                for(int i=0;i<args.size();++i){
                    listArg.push_back(args.at(i).toString());
                }
            }
        }

        m_processPtr.reset(new QProcess{});
        QObject::connect(m_processPtr.data(), QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),this, &WebsocketClient::slotProcessFinished);
        m_processPtr->start(programm, listArg);
    }
    catch(...){
        sendResponse("execute programm error!");

        if(spdlog::get(m_logName)){
            spdlog::get(m_logName)->debug("execute programm error!");
        }
    }
    */
}

void WebsocketClient::execute_file(const TaskItem &incomingTask)
{
    /*
    QJsonArray  args;
    QStringList listArg;
    QString     filename;
    bool dirCreated=QDir(m_appDirPath + "/../download").exists();

    try{
        //set filename
        if(!incomingTask.contains("filename")){
            sendResponse("incoming json field 'filename' is empty!");

            if(spdlog::get(m_logName)){
                spdlog::get(m_logName)->debug("incoming json field 'filename' is empty!");
            }

            return;
        }
        filename=incomingTask["filename"].toString();

        //set arguments (if presents)
        if(incomingTask.contains("arguments")){
            QJsonValue arguments=incomingTask["arguments"];
            if(arguments.isArray()){
                args=arguments.toArray();
                for(int i=0;i<args.size();++i){
                    listArg.push_back(args.at(i).toString());
                }
            }
        }

        QString programm {dirCreated ? m_appDirPath + "/../download/" + filename : m_appDirPath + "/" + filename};

        m_processPtr.reset(new QProcess{});
        QObject::connect(m_processPtr.data(), QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),this, &WebsocketClient::slotProcessFinished);
        m_processPtr->start(programm, listArg);
    }
    catch(...){
        sendResponse("execute file error!");
        if(spdlog::get(m_logName)){
            spdlog::get(m_logName)->debug("execute file error!");
        }
    }
    */
}

void WebsocketClient::download(const TaskItem& incomingTask)
{
    /*
    QString urlStr {};
    QDir appDir(m_appDirPath);
    appDir.mkdir("../download");

    try{
        if(!incominJson.contains("url")){
            sendResponse("incoming json field 'url' is empty!");

            if(spdlog::get(m_logName)){
                spdlog::get(m_logName)->debug("шncoming json field 'url' is empty!");
            }
            return;
        }
        urlStr=incominJson["url"].toString();

        if(!incominJson.contains("filename")){
            sendResponse("incoming json field 'filename' is empty!");

            if(spdlog::get(m_logName)){
                spdlog::get(m_logName)->debug("incoming json field 'filename' is empty!");
            }
            return;
        }
        m_fileName=incominJson["filename"].toString();

        QUrl url(urlStr);
        m_accessManagerPtr.reset(new QNetworkAccessManager);
        m_accessManagerPtr->setAutoDeleteReplies(true);
        m_accessManagerPtr->setTransferTimeout(2000);
        QObject::connect(m_accessManagerPtr.get(), &QNetworkAccessManager::sslErrors,this, &WebsocketClient::slotHttpSslErrors);

        QNetworkRequest request;
        request.setUrl(url);
        request.setSslConfiguration(*m_sslConfigurationPtr.get());
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        request.setHeader(QNetworkRequest::UserAgentHeader, "UcAgent");

        if(spdlog::get(m_logName)){
            spdlog::get(m_logName)->debug("start download binary data");
        }

        QNetworkReply* reply=m_accessManagerPtr->get(request);
        QObject::connect(reply, &QNetworkReply::downloadProgress, this, &WebsocketClient::slotDownloadProgress);
        QObject::connect(reply, &QNetworkReply::finished,this, &WebsocketClient::slotFinished);
    }
    catch(...){
        sendResponse("download binary error!");
        if(spdlog::get(m_logName)){
            spdlog::get(m_logName)->debug("download binary error!");
        }
    }
    */
}

void WebsocketClient::send_response(const QString &result)
{

}

WebsocketClient::WebsocketClient(const QUrl &ws_url, int ws_port, const QString &app_dir_path, std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, const std::string &log_name, QObject *parent)
    : QObject(parent), m_ws_url{ws_url}, m_ws_port{ws_port}, m_app_dir_path{app_dir_path}, m_ssl_configuration_ptr{ssl_configuration_ptr},m_log_name{log_name}
{
    init_machine_uid();
    m_logger_ptr=spdlog::get(m_log_name);
}

WebsocketClient::~WebsocketClient()
{
    if(m_command_socket_ptr && m_command_socket_ptr->state()==QAbstractSocket::ConnectedState){
        m_command_socket_ptr->close();
    }
}

void WebsocketClient::start()
{
    m_command_socket_ptr.reset(new QWebSocket);
    m_command_socket_ptr->setSslConfiguration(*m_ssl_configuration_ptr.get());
    QObject::connect(m_command_socket_ptr.get(), &QWebSocket::sslErrors,this, &WebsocketClient::slot_ws_ssl_errors);
    QObject::connect(m_command_socket_ptr.get(), &QWebSocket::connected, this, &WebsocketClient::slot_connected);
    QObject::connect(m_command_socket_ptr.get(), &QWebSocket::disconnected, this, &WebsocketClient::slot_disconnected);
    QObject::connect(m_command_socket_ptr.get(), &QWebSocket::textMessageReceived,this, &WebsocketClient::slot_text_message_received);
    QObject::connect(m_command_socket_ptr.get(), &QWebSocket::pong, this, &WebsocketClient::slot_pong);

    m_ping_timer.setInterval(m_interval);
    QObject::connect(&m_ping_timer, &QTimer::timeout,this, &WebsocketClient::slot_ping_timeout);

    m_connection_timer.setInterval(m_interval);
    QObject::connect(&m_connection_timer, &QTimer::timeout,this, &WebsocketClient::slot_connect_timeout);
    m_connection_timer.start();

    if(m_logger_ptr){
        m_logger_ptr->debug("websocket client started with uid: {}", m_uid.toStdString());
    }
    m_command_socket_ptr->open(m_ws_url);
}

void WebsocketClient::stop()
{
    try{
        m_ping_timer.stop();
        m_connection_timer.stop();
        //disconnect objects to prevent timers start
        QObject::disconnect(m_command_socket_ptr.get(), &QWebSocket::disconnected, this, &WebsocketClient::slot_disconnected);
        m_command_socket_ptr->close();
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_ping_timeout()
{
    try{
        m_command_socket_ptr->ping(m_uid.toUtf8());
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_connect_timeout()
{
    try{
        m_command_socket_ptr->close();
        m_command_socket_ptr->open(m_ws_url);
        if(m_logger_ptr){
            m_logger_ptr->debug("try to connect to host: {}", m_ws_url.toString().toStdString());
        }
    }
    catch(...){
    }
}

void WebsocketClient::slot_ws_ssl_errors(const QList<QSslError> &errors)
{
    try{
        m_command_socket_ptr->ignoreSslErrors();
        if(!errors.empty()){
            for(const auto& error: errors){
                if(m_logger_ptr){
                    m_logger_ptr->debug("websocket ssl error found: {}", error.errorString().toStdString());
                }
            }
        }
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_connected()
{
    try{
        m_connected=true;
        m_connection_timer.stop();
        m_ping_timer.start();

        if(m_logger_ptr){
            m_logger_ptr->debug("websocket connected to host");
        }
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_disconnected()
{
    try{
        m_connected=false;
        m_ping_timer.stop();
        m_connection_timer.start();
        if(m_logger_ptr){
            m_logger_ptr->debug("websocket disconnected from host");
        }
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_text_message_received(const QString &message)
{
    /*
    QJsonObject incomingJson=QJsonDocument::fromJson(message.toUtf8()).object();

    if(incomingJson.contains("method")){
        const auto command=incomingJson.value("method").toString();
        if(command=="executeprogramm"){
            executeProgramm(incomingJson);
        }
        else if(command=="executefile"){
            executeFile(incomingJson);
        }
        else if(command=="download"){
            downloadBinary(incomingJson);
        }
    }
    else{
        if(spdlog::get(m_logName)){
            spdlog::get(m_logName)->debug("incoming json does not contains 'method' field");
        }
    }
    */
}

void WebsocketClient::slot_pong(quint64 elapsed_time, const QByteArray &payload)
{
    Q_UNUSED(elapsed_time)
    try{
        if(!std::equal(payload.begin(), payload.end(), m_uid.begin())){
            slot_disconnected();
        }
    }
    catch(...){
        //TODO log errors here
    }
}

void WebsocketClient::slot_finished()
{
    bool dir_created=QDir(m_app_dir_path + "/../download").exists();
    QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());

      if (reply->error() == QNetworkReply::NoError){
        QByteArray content= reply->readAll();
        if(!content.isEmpty()){
            QFile out(dir_created     ? m_app_dir_path + "/../download/" + m_file_name : m_app_dir_path + "/" + m_file_name);
            if(out.open(QIODevice::WriteOnly)){
                out.write(content);
                out.close();

                send_response(QString("file %1 downloaded sucess").arg(m_file_name));
                if(m_logger_ptr){
                    m_logger_ptr->debug("file {} downloaded sucess", m_file_name.toStdString());
                }
                return;
            }
        }
      }

      else{
          send_response(QString("network reply error: %1").arg(reply->errorString()));
          if(m_logger_ptr){
              m_logger_ptr->debug("network reply error: {}", reply->errorString().toStdString());
          }
      }

      reply->deleteLater();
}

void WebsocketClient::slot_download_progress(qint64 bytes_received, qint64 bytes_total)
{
    int percents_now=round(bytes_received / (bytes_total/100));
    if((percents_now % 20)==0){
        if(m_logger_ptr){
            m_logger_ptr->debug("download progress: {} percents", percents_now);
        }
    }

}

void WebsocketClient::slot_http_ssl_errors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    try{
        reply->ignoreSslErrors();
        if(!errors.empty()){
            for(const auto& error: errors){
                if(m_logger_ptr){
                    m_logger_ptr->debug("http ssl error found: {}", error.errorString().toStdString());
                }
            }
        }
    }
    catch(...){
    }
}

void WebsocketClient::slot_process_finished(int exit_code, QProcess::ExitStatus exit_status)
{ 
    QProcess* process=static_cast<QProcess*>(sender());
    QByteArray std_out=process->readAllStandardOutput();
    QByteArray std_err=process->readAllStandardError();
    process->deleteLater();

    QString status=exit_status==QProcess::NormalExit ? "NormalExit" : "CrashExit";
    send_response(QString("process %1 fifnished with status %2").arg(process->program()).arg(status));

    if(m_logger_ptr){
        m_logger_ptr->debug("process {} fifnished with status: {}", process->program().toStdString(), status.toStdString());
    }
}
