#include "mainclient.h"
#include "log.h"
#include <fcntl.h>

using Logger::log;

namespace Socket
{

MainClient::MainClient()
    : m_sockfd(-1)
{

}

MainClient::MainClient(SOCKET fd)
    : m_sockfd(fd)
{

}

MainClient::MainClient(const MainClient& client)
{
    m_sockfd = client.m_sockfd;
}

MainClient::~MainClient()
{
    closeSocket();
}

SOCKET MainClient::fd()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return m_sockfd;
}

int MainClient::initSocket()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET != m_sockfd)
        return -1;

    m_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == m_sockfd)
    {
        log(Logger::All, "Client socket initialized failed.");
        return -1;
    }
    log(Logger::All, "Client socket initialized successfully.");

    return 0;
}

int MainClient::setNonBlocking(bool block)
{
    int flags = fcntl(m_sockfd, F_GETFL, 0);
    if(block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    return fcntl(m_sockfd, F_SETFL, flags);
}

int MainClient::closeSocket()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(m_sockfd != INVALID_SOCKET)
    {
        close(m_sockfd);
        return 0;
    }
    return -1;
}

bool MainClient::isRun()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return m_sockfd != INVALID_SOCKET;
}

int MainClient::connectSocket(const std::string &ip, unsigned int port)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    struct sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);
    _sin.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    
    int ret = connect(m_sockfd, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if(ret < 0)
    {
        log(Logger::All, "Client connect to server failed.");
        return -1;
    }
    log(Logger::All, "Client connect to server successfully.");

    return 0;
}

size_t MainClient::recv(void *buf, size_t len, int flags)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    return ::recv(m_sockfd, buf, len, flags);
}

size_t MainClient::send(const void *buf, size_t len, int flags)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    size_t nLen = 0;
    size_t ret = 0;
    do
    {
        ret = ::send(m_sockfd, buf + nLen, len - nLen, flags);
        if(ret < 0)
        {
            if(errno != EINTR)
                return ret;
            else    
                continue;
        }
        nLen += ret;
    }while(nLen != len);

    return nLen;
}

} // namespace Socket