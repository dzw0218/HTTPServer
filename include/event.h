/**
epoll事件接口
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "socketinterface.h"
#include <iostream>
#include <sys/epoll.h>

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
	EDEFULT = EIN | ECLOSE | EERR | EET
};

class Observer //虚基类
{
public:
    virtual ~Observer(){};

    virtual void epoll_handlein(int) = 0;
    virtual void epoll_handleout(int) = 0;
    virtual void epoll_handleclose(int) = 0;
    virtual void epoll_handleerr(int) = 0;

    int register_event(int fd, EventType type = EDEFAULT);
    int register_event(SocketInterface::SocketDest& socket, EventType type = EDEFAULT);
    int shutdown_event(int fd);
    int shutdown_event(SocketInterface::SocketDest& socket);
};



} // namespace EVENT_H_

#endif // EVENT_H_