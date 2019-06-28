#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace TaskQueue
{

template<class TYPE>
class QueueInterface //虚基类
{
public:
    ~QueueInterface(){};

    //返回队列的大小
    virtual size_t size() = 0;
    //清空队列
    virtual int clear() = 0;
    //阻塞式弹出队列中的队列项
    virtual TYPE& popTask() = 0;
    //非阻塞式弹出队列中的队列项
    virtual TYPE& popTaskNB() = 0;
    //阻塞式进入队列
    virtual int pushTask(TYPE &in) = 0;
    //非阻塞时进入队列
    virtual int pushTaskNB(TYPE &in) = 0;
};

template<class TYPE>
class MyQueue : public QueueInterface<TYPE>
{
public:
    MyQueue(size_t queuelen)
        : queue_size(queuelen),
        fill_flag(false)
    {}
    ~MyQueue()
    {}

    size_t size()
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        return (size_t)m_queue.size();
    }
    int clear()
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        while(!m_queue.empty())
            m_queue.pop();
        return 0;
    }
    TYPE& popTask()
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
    TYPE& popTaskNB()
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        if(m_queue.size() == 0)
        {
            TYPE *temp = nullptr;
            return *temp;
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
    int pushTask(TYPE &in)
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
    int pushTaskNB(TYPE &in)
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

private:
    typedef std::queue<TYPE> queue_t;
    
    std::mutex m_mutex;
    bool fill_flag;
    std::condition_variable cv_fill;
    bool free_flag;
    std::condition_variable cv_free;

    queue_t m_queue;
    size_t queue_size;
};

} // namespace TaskQueue

#endif