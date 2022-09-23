#include <iostream>
#include <QDir>
#include <QProcess>
#include <QtNetwork>
#include <QSslConfiguration>

#include "TcpServer.h"

TcpServer::TcpServer(const QString &app_dir_path, const std::string &log_name, QObject *parent)
    : QObject(parent),m_app_dir_path{app_dir_path}, m_log_name{log_name}
{
    m_logger_ptr=spdlog::get(m_log_name);
}

void TcpServer::send_response(const QJsonObject &outcoming_json, bool disconnect)
{
    m_tcp_socket_ptr->write(QJsonDocument(outcoming_json).toJson());
    m_tcp_socket_ptr->waitForBytesWritten(10000);
    if(disconnect){
        m_tcp_socket_ptr->disconnectFromHost();
        m_tcp_socket_ptr->deleteLater();
    }
}

void TcpServer::start()
{
    try{
        m_tcp_server_ptr=new QTcpServer(this);
        QObject::connect(m_tcp_server_ptr, &QTcpServer::newConnection, this, &TcpServer::slot_new_connection);
        m_tcp_server_ptr->listen(QHostAddress::LocalHost,m_tcp_port);

        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server start success");
        }
    }
    catch(std::exception& ex){
        if(m_logger_ptr){
            m_logger_ptr->debug("start tcp server with error: {}", ex.what());
        }
    }
    catch(...){
        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server start with unknown error");
        }
    }
}

void TcpServer::stop()
{
    if(m_tcp_server_ptr){
        m_tcp_server_ptr->close();
    }
}

void TcpServer::slot_new_connection()
{
    try{
        m_tcp_socket_ptr=m_tcp_server_ptr->nextPendingConnection();
        QObject::connect(m_tcp_socket_ptr ,&QTcpSocket::readyRead,this, &TcpServer::slot_ready_read);

        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server new incoming connection success");
        }
    }
    catch(std::exception& ex){
        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server new incoming connection error: {}", ex.what());
        }
    }
    catch(...){
        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server new incoming connection unknown error");
        }
    }
}

void TcpServer::slot_ready_read()
{
    QProcess* process_ptr=new QProcess();
    QJsonObject outcoming_json{};

    try{
        QByteArray content=m_tcp_socket_ptr->readAll();
        m_tcp_socket_ptr->deleteLater();

        if(content.isEmpty()){
            if(m_logger_ptr){
                m_logger_ptr->debug("inconing content is empty, nothing to execute");
            }
            outcoming_json["result"]="Error";
            outcoming_json["data"]="inconing content is empty, nothing to execute";
            return;
        }

        process_ptr->start(content, QStringList() << "/S");
        process_ptr->waitForFinished(-1);

        outcoming_json["result"]=process_ptr->error()==QProcess::UnknownError ? "Success" : "Error";
        outcoming_json["exitcode"]=process_ptr->exitCode();
        outcoming_json["exitstatus"]=process_ptr->exitStatus();
        outcoming_json["stdout"]=QString(process_ptr->readAllStandardOutput());
        outcoming_json["stderr"]=QString(process_ptr->readAllStandardError());
        outcoming_json["data"]=process_ptr->error()==QProcess::UnknownError ? "" : process_ptr->errorString();

        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server start process: {} success with result:\n {}",
                                          process_ptr->program().toStdString(), QJsonDocument(outcoming_json).toJson().toStdString());
        }
    }
    catch(std::exception& ex){
        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server start process {} with error {}",
                                          process_ptr->program().toStdString(), ex.what());
        }
    }
    catch(...){
        if(m_logger_ptr){
            m_logger_ptr->debug("tcp server start process {} with unknown error",
                                          process_ptr->program().toStdString());
        }
    }
    process_ptr->deleteLater();
}

