#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <QQueue>
#include <QObject>
#include <mutex>
#include <condition_variable>

#include "task/Task.h"
#include "spdlog/spdlog.h"

class TaskItem;

class TaskQueue : public QObject
{
    Q_OBJECT
private:
    std::string log_name_ {};
    bool        flag_ {true};
    QQueue<TaskItem> incoming_queue_ {};

    std::mutex mutex_ {};
    std::condition_variable cv_ {};

    std::shared_ptr<spdlog::logger> logger_ptr_ {nullptr};

public:
    explicit TaskQueue(const std::string& log_name, QObject *parent = nullptr);

public slots:
    void slot_start();
    void slot_enqueue_task_list(const std::vector<TaskItem> &incoming_task_list);

signals:
    void signal_dequeue_task(const TaskItem& task_item);

};

#endif // TASKQUEUE_H
