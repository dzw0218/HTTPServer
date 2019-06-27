/**
* socket接口类
 */

#ifndef SOCKETINTERFACE_H_
#define SOCKETINTERFACE_H_

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR -1

namespace SocketInterface
{

class SocketDest
{
public:
    virtual SOCKET fd() = 0;
    virtual int setNonBlocking(bool block) = 0;
    virtual int closeSocket() = 0;
};

//HTTP连接客户端接口
class ClientInterface : public SocketDest
{
public:
    virtual ~ClientInterface()
    {
    }

    //初始化套接字，0/成功，-1/失败
    virtual int initSocket() = 0;
    //virtual int connect(const std::string &ip, unsigned int port) = 0;
    virtual size_t recv(void *buf, size_t len, int flags) = 0;
    virtual size_t send(const void *buf, size_t len, int flags) = 0;
};

//HTTP服务器接口
class ServerInterface : public SocketDest
{
public:
    virtual ~ServerInterface()
    {
    }

    virtual int initSocket(size_t backlog) = 0;
    //设置该套接字是否阻塞，0/成功，-1/失败
    virtual bool isRun() = 0;
    virtual ClientInterface *acceptSocket() = 0;
};

}

 #endif // SOCKETINTERFACE_H_