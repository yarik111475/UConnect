#ifndef TASKITEM_H
#define TASKITEM_H

#include <set>
#include <string>
#include <QString>

class TaskItem{
public:
    std::set<QString> if_names_ {};
    std::set<uint16_t> tcp_ports_ {/*22, 80, 443, 993, 999, 56987*/}; //that ports may be used in future

    TaskItem()=default;
    ~TaskItem()=default;
    inline bool empty()const{
        return if_names_.empty() || tcp_ports_.empty();
    }
};

#endif // TASKITEM_H
