#include "threadpool.h"

namespace ThreadPool
{

MyThreadPool::MyThreadPool(size_t threadnum, size_t tasknum)
    : m_taskqueue(tasknum), 
    m_hasleader(false)
{
    m_threadnum = (threadnum > ThreadNum) ? ThreadNum : threadnum;
    createThreadpool();
}

MyThreadPool::~MyThreadPool()
{
    destoryThreadpool();
}

int MyThreadPool::pushTask(ThreadHandle *task, bool block)
{
    if(block)
        return m_taskqueue.pushTask(task);
    return m_taskqueue.pushTaskNB(task);
}

void MyThreadPool::createThreadpool()
{
    for(size_t index = 0; index < m_threadnum; ++index)
    {
        m_threadpool.push_back(new std::thread(process_task, (void*)this));
    }
}

void MyThreadPool::destoryThreadpool()
{
    threadpool_t::iterator iter = m_threadpool.begin();
    while(iter != m_threadpool.end())
    {
        (*iter)->join();
        delete *iter;
        ++iter;
    }
    m_threadpool.clear();
}

void MyThreadPool::promote_leader()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    while(m_hasleader)
    {
        cv.wait(lck);
    }
    m_hasleader = true;
}

void MyThreadPool::join_follower()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_hasleader = false;
    cv.notify_one();
}

void* MyThreadPool::process_task(void *args)
{
    MyThreadPool &currentThread = *(MyThreadPool*)args;
    while(true)
    {
        currentThread.promote_leader();
        ThreadHandle *handler = currentThread.m_taskqueue.popTask();
        currentThread.join_follower();

        if(handler)
            handler->taskHandle();
    }
}

ThreadPoolProxy::ThreadPoolProxy()
{
    m_threadpool = new MyThreadPool(4, 1024);
}

ThreadPoolProxy::~ThreadPoolProxy()
{
    delete m_threadpool;
}

ThreadPool* ThreadPoolProxy::instance()
{
    static ThreadPoolProxy threadpoolproxy;
    return &threadpoolproxy;
}

int ThreadPoolProxy::pushTask(ThreadHandle *task, bool block)
{
    return m_threadpool->pushTask(task, block);
}

} // namespace ThreadPool