#ifndef MAINCLIENT_H_
#define MAINCLIENT_H_

#include "socketinterface.h"
#include <iostream>
#include <mutex>
#include <string>

namespace Socket
{

class MainClient : public SocketInterface::ClientInterface
{
public:
    MainClient();
    MainClient(SOCKET fd);
    MainClient(const MainClient& client);

    SOCKET fd();
    int initSocket();
    int setNonBlocking(bool block);
    int closeSocket();
    bool isRun();
    int connect(const std::string& ip, unsigned int port);
    size_t recv(void *buf, size_t len, int flags = 0);
    size_t send(const void *buf, size_t len, int flags = 0);

private:
    SOCKET m_sockfd;
    std::mutex m_mutex;
};

}

#endif // MAINCLIENT_H_