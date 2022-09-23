#include "TaskQueue.h"

TaskQueue::TaskQueue(const std::string &log_name, QObject *parent)
    : QObject(parent), log_name_{log_name}
{
    logger_ptr_=spdlog::get(log_name_);
}

void TaskQueue::slot_start()
{
    if(logger_ptr_){
        logger_ptr_->info("start task queue event loop");
    }

    while(flag_){
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{
            return !incoming_queue_.empty();
        });

        TaskItem incoming_task=incoming_queue_.dequeue();
        lock.unlock();
        emit signal_dequeue_task(incoming_task);
    }
}

void TaskQueue::slot_enqueue_task_list(const std::vector<TaskItem> &incoming_task_list)
{
    //check if incoming task list is empty, logging and return
    if(incoming_task_list.empty()){
        if(logger_ptr_){
            logger_ptr_->info("incoming task list is empty, nothing to execute");
        }
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for(const TaskItem& incoming_task: incoming_task_list){
        incoming_queue_.enqueue(incoming_task);
    }
    cv_.notify_all();

    if(logger_ptr_){
        logger_ptr_->info("enqueue task list into task queue, count of enqued tasks: {}", incoming_task_list.size());
    }
}
