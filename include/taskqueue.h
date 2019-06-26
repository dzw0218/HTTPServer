#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_

#include <iostream>
#include <queue>
#include <mutex>

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
class MyQueue : public QueueInterface
{
public:
    MyQueue(size_t queuelen);
    ~MyQueue();

    size_t size();
    int clear();
    TYPE& popTask();
    TYPE& popTaskNB();
    int pushTask(TYPE &in);
    int pushTaskNB(TYPE &in);

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