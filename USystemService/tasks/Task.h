#ifndef TASKITEM_H
#define TASKITEM_H
#include <QString>

class TaskParamsItem{
public:
    QString     m_url {};
    QString     m_hash {};
    QString     m_filename {};
    QString     m_programm {};
    QStringList m_arguments {};

    TaskParamsItem()=default;
    TaskParamsItem(const QString& url, const QString& hash, const QString& filename,const QString& programm) :
        m_url(url),m_hash{hash},m_filename{filename},m_programm{programm}{
    }
};

class TaskItem{
public:
    enum TaskType{
        DOWNLOAD,
        EXECUTE_FILE,
        EXECUTE_PROGRAMM
    };

    TaskType  m_type;
    QString   m_id {};
    QString   m_name {};
    TaskParamsItem m_params_item {};

    TaskItem()=default;
    TaskItem(QString id,QString name): m_id{id},m_name{name}{
    }
    void setType(TaskType type){
        m_type=type;
    }
    void set_params_item(const TaskParamsItem& params_item){
        m_params_item=params_item;
    }
};

#endif // TASKITEM_H
