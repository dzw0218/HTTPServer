#include "mainserver.h"
#include "log.h"
#include <fcntl.h>

using Logger::log;

namespace Socket
{

MainServer::MainServer(const std::string &ip, unsigned int port)
    : m_sockfd(-1),
    m_ip(ip),
    m_port(port)
{
}

MainServer::~MainServer()
{
    
}

int MainServer::setNonBlocking(bool block)
{
    int flags = fcntl(m_sockfd, F_GETFL, 0);
    if(block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    return fcntl(m_sockfd, F_SETFL, flags);
}

SOCKET MainServer::fd()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return m_sockfd;
}

int MainServer::initSocket(size_t backlog)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET != m_sockfd)
        return -1;

    m_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == m_sockfd)
    {
        log(Logger::All, "Socket initialized failed.");
        return -1;
    }
    log(Logger::All, "Socket initialized successfully.");

    m_port = ((m_port < 1024) || (m_port > 5000)) ? 1234 : m_port;
    struct sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(m_port);
    _sin.sin_addr.s_addr = m_ip.empty() ? INADDR_ANY : inet_addr(m_ip.c_str());
    if(SOCKET_ERROR == bind(m_sockfd, (sockaddr*)&_sin, sizeof(sockaddr_in)))
    {
        std::string message;
        message.append("Bind server ");
        message.append(m_ip.c_str());
        message.push_back(m_port);
        message.append(" failed.");
        Logger::log(Logger::All, message.c_str());
        return -1;
    }
    log(Logger::All, "Bind server successfully.");

    if(listen(m_sockfd, backlog) < 0)
    {
        log(Logger::All, "Server listening failed.");
        return -1;
    }
    log(Logger::All, "Server listening successfully.");

    return 0;
}

int MainServer::closeSocket()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(m_sockfd != INVALID_SOCKET)
    {
        close(m_sockfd);
        return 0;
    }
    return -1;
}

bool MainServer::isRun()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return m_sockfd != INVALID_SOCKET;
}

SocketInterface::ClientInterface *MainServer::acceptSocket()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
    {
        log(Logger::All, "Server not running.");
        return nullptr;
    }

    struct sockaddr_in _clientsin = {};
    int len = sizeof(_clientsin);
    SOCKET cSock = accept(m_sockfd, (sockaddr*)&_clientsin, (socklen_t*)&len);
    if(INVALID_SOCKET == cSock)
    {
        log(Logger::All, "Client connection failed.");
        return nullptr;
    }
    log(Logger::All, "Client connection successfully.");

    SocketInterface::ClientInterface *client = new MainClient(cSock);
    return client;
}

} // namespace Socket
