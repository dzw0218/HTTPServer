/**
epoll事件接口
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "socketinterface.h"
#include <iostream>
#include <sys/epoll.h>
#include <mutex>

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
public:
    virtual ~Observer(){};

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
public:
    MyObserver(Observer &observer, EventType);
    ~MyObserver();

    inline int increaseRef();
    inline int reduceRef();
    inline int countRef();
    inline bool release();
    inline EventType getEventType();
    inline const Observer *getHandle();


protected:
    void epoll_handlein(int);
    void epoll_handleout(int);
    void epoll_handleclose(int);
    void epoll_handleerror(int);

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

    virtual int register_event(int, ThreadHandle*, EventType) = 0;
    virtual int shutdown_event(int) = 0;
};

class MyEvent : public Event // 主事件类
{
public:
    MyEvent(size_t event_max);
    ~MyEvent();
    
};

} // namespace EVENT_H_

#endif // EVENT_H_