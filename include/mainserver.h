#ifndef MAINSERVER_H_
#define MAINSERVER_H_

#include "socketinterface.h"
#include <mutex>
#include <string>

namespace Socket
{

class MainServer : public SocketInterface::ServerInterface
{
public:
    MainServer(const std::string &ip, unsigned int port);
    ~MainServer();

    SOCKET fd();
    int initSocket(size_t backlog);
    int setNonBlocking(bool block);
    int closeSocket();
    bool isRun();
    SocketInterface::ClientInterface *acceptSocket();

private:
    std::string m_ip;
    unsigned int m_port;

    SOCKET m_sockfd;
    std::mutex m_mutex;
};

} // namespace Socket

#endif // MAINSERVER_H_