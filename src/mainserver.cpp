#include "mainserver.h"

namespace Socket
{

MainServer::MainServer(const std::string &ip, unsigned int port)
    : m_sockfd(-1),
    m_ip(ip),
    m_port(port)
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
        return;

    m_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == m_sockfd)
    {
        Logger::log(Logger::All, "Socket initialized failed.");
        return -1;
    }
    Logger::log(Logger::All, "Socket initialized successfully.");

    m_port = ((m_port < 1024) || (m_port > 5000)) ? 1234 : m_port;
    struct sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(m_port);
    _sin.sin_addr.s_addr = m_ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    if(SOCKET_ERROR == bind(m_sockfd, (sockaddr*)&_sin, sizeof(sockaddr_in)))
    {
        std::string message;
        message.append("Bind server ").append(ip.c_str()).append(m_port).append(" failed.");
        Logger::log(Logger::All, message);
        return -1;
    }
    Logger::log(Logger::All, "Bind server successfully.");

    if(listen(m_sockfd, backlog) < 0)
    {
        Logger::log(Logger::All, "Server listening failed.");
        return -1;
    }
    Logger::log(Logger::All, "Server listening successfully.");

    return 0;
}

int MainServer::closeSocket()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(m_sockfd != INVALID_SOCKET)
    {
        close(m_sockfd);
    }
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
        Logger::log(Logger::All, "Server not running.");
        return -1;
    }

    struct sockaddr_in _clientsin = {};
    int len = sizeof(_clientsin);
    SOCKET cSock = accept(m_sockfd, (sockaddr*)&_clientsin, (socklen_t*)&len);
    if(INVALID_SOCKET == cSock)
    {
        Logger::log(Logger::All, "Client connection failed.");
        return -1;
    }
    Logger::log(Logger::All, "Client connection successfully.");

    //return client
}

} // namespace Socket
