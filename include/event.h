/**
epoll事件接口
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "socketinterface.h"
#include "threadpool.h"
#include <iostream>
#include <sys/epoll.h>
#include <mutex>
#include <map>


namespace Event
{

#define EVENT_MAX 1024

enum EventType
{
    EIN = EPOLLIN,		  // 读事件
	EOUT = EPOLLOUT,	  // 写事件
	ECLOSE = EPOLLRDHUP,  // 对端关闭连接或者写半部
	EPRI = EPOLLPRI,	  // 紧急数据到达
	EERR = EPOLLERR,	  // 错误事件
	EET = EPOLLET, 		  // 边缘触发
	EDEFAULT = EIN | ECLOSE | EERR | EET
};

class Observer //虚基类
{
    friend class MyObserver;
public:
    virtual ~Observer(){};

protected:
    virtual void epoll_handlein(int) = 0;
    virtual void epoll_handleout(int) = 0;
    virtual void epoll_handleclose(int) = 0;
    virtual void epoll_handleerr(int) = 0;
};

class Handler : public Observer //处理事件类
{
public:
    int register_event(int fd, EventType type = EDEFAULT);
    int register_event(SocketInterface::SocketDest& socket, EventType type = EDEFAULT);
    int shutdown_event(int fd);
    int shutdown_event(SocketInterface::SocketDest& socket);
};

class MyObserver : public Observer //主要观察者
{
    friend class MyEvent;
public:
    MyObserver(Observer &observer, EventType);
    ~MyObserver();

    inline int increaseRef();
    inline int reduceRef();
    inline bool countRef();
    inline void release();
    inline EventType getEventType();
    inline const Observer *getHandle();


protected:
    void epoll_handlein(int);
    void epoll_handleout(int);
    void epoll_handleclose(int);
    void epoll_handleerr(int);

private:
    std::mutex m_mutex;

    EventType m_eventtype;
    Observer &m_observer;

    int m_refcount;
};

class Event //事件虚基类
{
public:
    ~Event(){};

    virtual int register_event(int, Handler *, EventType) = 0;
    virtual int shutdown_event(int) = 0;
};

class MyEvent : public Event, public ThreadPool::ThreadHandle // 主事件类
{
public:
    MyEvent(size_t event_max);
    ~MyEvent();

    int register_event(int, Handler *, EventType);
    int shutdown_event(int);

protected:
    void taskHandle();

private:
    enum Exist{
        NotExist, Modify, HandleModify, TypeModify, Existed
    };
    enum Config{
        EventLen = 1024,
    };
    typedef std::map<int, MyObserver*> EventObserver_t;
    typedef std::map<int, EventType> EventType_t;

    Exist isExist(int fd, EventType type, Handler *handler);
    int record(int fd, EventType type, Handler *handler);
    int detach(int fd, bool release = false);
    MyObserver *getObserver(int fd);
    int pushTask(int fd, EventType type);
    int popTask(int &fd, EventType &type);
    size_t taskSize();
    int cleartask(int fd);
    int unregister_event(int);
    static void *eventwait(void *args);

private:
    int m_epollfd;
    EventObserver_t m_eventobserver;
    std::mutex m_eventobserver_mutex;
    EventType_t m_eventtype;
    std::mutex m_eventtype_mutex;

    struct epoll_event m_event[EventLen];
    ThreadPool::ThreadPool *m_threadpool;
    std::thread m_thread;
};

class EventProxy : public Event //事件类代理
{
public:
    static EventProxy *instance();
    int register_event(int, Handler *, EventType);
    int register_event(SocketInterface::SocketDest &, Handler*, EventType);
    int shutdown_event(int);
    int shutdown_event(SocketInterface::SocketDest &);

private:
    EventProxy(int event_max);
    ~EventProxy();

    Event *m_event;
};

} // namespace EVENT_H_

#endif // EVENT_H_