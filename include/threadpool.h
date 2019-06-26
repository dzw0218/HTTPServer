#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <iostream>
#include <queue>
#include <vector>

namespace ThreadPool
{

class ThreadHandle
{
public:
    virtual ~ThreadHandle(){};

protected:
    virtual void taskHandle() = 0;
};

class ThreadPool
{
public:
    virtual ~ThreadPool(){};

protected:
    virtual int pushTask(ThreadHandle *task, bool block = false) = 0;
};

class MyThreadPool : public ThreadPool
{
public:
    enum Config
    {
        ThreadNum = 64;
        TaskNum = 2018;
    };
    typedef void*(m_threadpro)(void *);
    typedef std::vector<std::thread> threadpool_t;
    typedef std::queue<ThreadHandle *> taskqueue_t;
    typedef std::priority_queue<ThreadHandle *> taskpriorityqueue_t;

    MyThreadPool(size_t threadnum, size_t tasknum);
    ~MyThreadPool();
    int pushTask(ThreadHandle *task, bool block = false);

private:
    void createThreadpool(size_t threadnum);
    void destoryThreadpool();
    void promote_leader();
    void join_follower();
    static void *process_task(void *args);

    bool m_hasleader;
    std::mutex m_mutex;
    std::contidition_variable cv;

    size_t m_threadnum;
    threadpool_t m_threadpool;
    taskqueue_t m_taskqueue;
    taskpriorityqueue_t m_taskpriorityqueue;
};

class ThreadPoolProxy : public ThreadPool
{

};

} // namespace ThreadPool

#endif // THREADPOOL_H_