#ifndef UTILS_H
#define UTILS_H

#include <QUuid>
#include <QString>

class Utils
{
private:
    static QString m_cn;
    static QString m_machine_uid;
public:
    explicit Utils()=default;
    ~Utils()=default;

    static QString get_cn();
    static QString get_machine_uid();

};

QString Utils::m_cn={};
QString Utils::m_machine_uid={};

#endif // UTILS_H
