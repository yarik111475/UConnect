#ifndef TASKITEM_H
#define TASKITEM_H

#include <string>
#include <QtCore>
#include <QString>
#include <QJsonObject>

class TaskItem{
public:
    TaskItem()=default;
    ~TaskItem()=default;

    QString id_ {};
    QString name_ {};
    std::string incoming_string_ {};
};

Q_DECLARE_METATYPE(TaskItem)


#endif // TASKITEM_H
