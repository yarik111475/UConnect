#ifndef UTILS_H
#define UTILS_H

#include <QJsonArray>

class  Utils
{
public:
    Utils()=default;
    ~Utils()=default;

    static bool isElevated();
    static QJsonArray string_list_to_json_array(const QStringList &string_list);
    static void append_json_object_to_json_object(const QJsonObject &source, QJsonObject &target);
};

#endif // UTILS_H
