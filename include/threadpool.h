#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "taskqueue.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThreadPool
{

class ThreadHandle //线程池线程处理虚基类
{
public:
    friend class MyThreadPool;
    virtual ~ThreadHandle(){};

protected:
    virtual void taskHandle() = 0;
};

class ThreadPool //线程池虚基类
{
public:
    virtual ~ThreadPool(){};

    virtual int pushTask(ThreadHandle *task, bool block = false) = 0;
};

class MyThreadPool : public ThreadPool //线程池类
{
public:
    enum Config
    {
        ThreadNum = 64,
        TaskNum = 2018
    };
    typedef void*(m_threadpro)(void *);
    typedef std::vector<std::thread *> threadpool_t;
    typedef TaskQueue::MyQueue<ThreadHandle *> taskqueue_t;
    //typedef std::priority_queue<ThreadHandle *> taskpriorityqueue_t;

    MyThreadPool(size_t threadnum, size_t tasknum);
    ~MyThreadPool();
    int pushTask(ThreadHandle *task, bool block = false);

private:
    void createThreadpool();
    void destoryThreadpool();
    void promote_leader();
    void join_follower();
    static void *process_task(void *args);

    bool m_hasleader;
    std::mutex m_mutex;
    std::condition_variable cv;

    size_t m_threadnum;
    threadpool_t m_threadpool;
    taskqueue_t m_taskqueue;
    //taskpriorityqueue_t m_taskpriorityqueue;
};

class ThreadPoolProxy : public ThreadPool //线程池代理类
{
public:
    static ThreadPool* instance();
    int pushTask(ThreadHandle *task, bool block);

private:
    ThreadPoolProxy();
    ~ThreadPoolProxy();

    ThreadPool *m_threadpool;
};

} // namespace ThreadPool

#endif // THREADPOOL_H_