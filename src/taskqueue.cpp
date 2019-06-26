#include "taskqueue.h"

namespace TaskQueue
{

MyQueue::MyQueue(size_t queuelen)
    : queue_size(queuelen),
    free_flag(true),
    fill_flag(false)
{

}

MyQueue::~MyQueue()
{

}

size_t MyQueue::size()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return (size_t)m_queue.size();
}

int MyQueue::clear()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    while(!m_queue.empty())
        m_queue.pop();
    return 0;
}

TYPE& MyQueue::popTask()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    while(0 == m_queue.size())
    {
        fill_flag = true;
        cv_fill.wait(lck);
    }

    TYPE out = m_queue.front();
    m_queue.pop();

    if(free_flag)
    {
        cv_free.notify_one();
        free_flag = false;
    }
    return out;
}

TYPE& MyQueue::popTaskNB()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(0 == m_queue.size())
        return (TYPE)0;

    TYPE out = m_queue.front();
    m_queue.pop();

    if(free_flag)
    {
        cv_free.notify_one();
        free_flag = false;
    }

    return out;
}

int MyQueue::pushTask(TYPE& in)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    while(m_queue.size() == queue_size)
    {
        free_flag = true;
        cv_free.wait(lck);
    }

    m_queue.push(in);

    if(fill_flag)
    {
        cv_fill.notify_one();
        fill_flag = false;
    }
    
    return 0;
}

int MyQueue::pushTaskNB(TYPE& in)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(m_queue.size() == queue_size)
        return -1;

    m_queue.push(in);

    if(fill_flag)
    {
        cv_fill.notify_one();
        fill_flag = false;
    }

    return 0;
}

} // namespace TaskQueue