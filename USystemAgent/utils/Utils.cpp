#include <QFile>
#include <QUuid>

#include "Utils.h"

QString Utils::get_cn()
{
    if(m_cn.isEmpty()){
        m_cn=QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    return m_cn;
}

QString Utils::get_machine_uid()
{
    //case readed before
    if(!m_machine_uid.isEmpty()){
        return m_machine_uid;
    }

    QString machine_uid_file_path {};
    const QString machine_uid_file_name {"machine_uid.txt"};

#ifdef Q_OS_WINDOWS
    machine_uid_file_path=qEnvironmentVariable("ProgramData") + "/USystem/etc/" + machine_uid_file_name;
#endif
#ifdef Q_OS_UNIX
#endif

    //case file exists, read existing
    QFile in(machine_uid_file_path);
    if(in.open(QIODevice::ReadOnly)){
        m_machine_uid=in.readLine();
        in.close();

        //check if readed machine_uid is not empty, if so return it
        if(!m_machine_uid.isEmpty()){
            return m_machine_uid;
        }
    }

    //case file not exists or readed machine uid is empty, create new
    QFile out(machine_uid_file_path);
    if(out.open(QIODevice::WriteOnly)){
        m_machine_uid=QUuid::createUuid().toString(QUuid::WithoutBraces);
        out.write(m_machine_uid.toUtf8());
        out.close();
    }
    return m_machine_uid;
}
