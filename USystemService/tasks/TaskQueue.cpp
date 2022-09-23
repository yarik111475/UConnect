#include <QDir>
#include <QFile>
#include <QThread>
#include <QJsonArray>
#include <QSharedPointer>
#include <QSslConfiguration>

#include "TaskQueue.h"
#include "tasks/TaskExecutor.h"
#include "network/HttpClient.h"
#include <spdlog/spdlog.h>

void TaskQueue::execute_task(const TaskItem& incoming_task)
{
    TaskExecutor* task_executor_ptr=new TaskExecutor(m_ssl_configuration_ptr, m_app_dir_path, m_log_name);
    QObject::connect(task_executor_ptr, &TaskExecutor::signal_task_executed, m_http_client_ptr.get(), &HttpClient::slot_task_executed);
    QThread* thread_ptr=new QThread;

    QObject::connect(thread_ptr, &QThread::finished, thread_ptr, &QThread::deleteLater);
    QObject::connect(thread_ptr, &QThread::finished, task_executor_ptr, &TaskExecutor::deleteLater);
    QObject::connect(thread_ptr, &QThread::started, task_executor_ptr, &TaskExecutor::start_execute);

    task_executor_ptr->set_incoming_task(incoming_task);
    task_executor_ptr->moveToThread(thread_ptr);
    thread_ptr->start();

    if(m_logger_ptr){
        m_logger_ptr->debug("start new thread to execute task with id: {} and name: {}",
                            incoming_task.m_id.toStdString(), incoming_task.m_name.toStdString());
    }
}

TaskQueue::TaskQueue(std::shared_ptr<QSslConfiguration> ssl_configuration_ptr, std::shared_ptr<HttpClient> http_client_ptr, const QString &app_dir_path, const std::string &log_name, QObject *parent)
    : QObject(parent),m_ssl_configuration_ptr{ssl_configuration_ptr}, m_http_client_ptr{http_client_ptr},  m_app_dir_path{app_dir_path}, m_log_name{log_name}
{
    m_logger_ptr=spdlog::get(m_log_name);
}

void TaskQueue::start()
{
    if(m_logger_ptr){
        m_logger_ptr->debug("start task queue event loop");
    }

    while(m_flag){
        m_mutex.lock();
        if(m_incoming_queue.empty()){

            if(m_logger_ptr){
                m_logger_ptr->debug("incoming task queue is empty, wait for incoming tasks...");
            }

            m_wait_condition.wait(&m_mutex);
        }
        m_mutex.unlock();

        TaskItem incoming_task=m_incoming_queue.dequeue();

        if(m_logger_ptr){
            m_logger_ptr->debug("dequeue task with id: {} and name: {} from task queue",
                               incoming_task.m_id.toStdString(), incoming_task.m_name.toStdString());
        }

        execute_task(incoming_task);
    }
}

void TaskQueue::stop()
{
    m_flag=false;
    if(m_logger_ptr){
        m_logger_ptr->debug("stop task queue execute loop");
    }
}

void TaskQueue::push_task_list(const std::vector<TaskItem> &incoming_task_list)
{
    //check if incoming task list is empty, logging and return
    if(incoming_task_list.empty()){
        if(m_logger_ptr){
            m_logger_ptr->debug("incoming task list is empty, nothing to execute");
        }
        return;
    }

    QMutexLocker locker(&m_mutex);
    for(const TaskItem& incoming_task: incoming_task_list){
        m_incoming_queue.enqueue(incoming_task);
    }
    m_wait_condition.wakeAll();

    if(m_logger_ptr){
        m_logger_ptr->debug("enqueue task list into task queue, count of enqueud tasks: {}", incoming_task_list.size());
    }
}
