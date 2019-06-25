#include "mainclient.h"

namespace Socket
{

MainClient::MainClient()
    : m_sockfd(-1)
{

}

MainClient::MainClient(SOCKET fd)
    : sockfd(fd)
{

}

MainClient::MainClient(const MainClient& client)
    : m_sockfd(cleint->fd())
{

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
        return;

    m_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == m_sockfd)
    {
        Logger::log(Logger::All, "Client socket initialized failed.");
        return -1;
    }
    Logger::log(Logger::All, "Client socket initialized successfully.");

    return 0;
}

int MainClient::setNonBlock(bool block)
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
    }
}

bool MainClient::isRun()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    return m_sockfd != INVALID_SOCKET;
}

int MainClient::connect(const std::string &ip, unsigned int port)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    struct sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);
    _sin.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    int len = sizeof(_sin);
    
    int ret = connect(m_sockfd, (sockaddr*)&_sin, (socklen_t*)&len);
    if(ret < 0)
    {
        Logger::log(Logger::All, "Client connect to server failed.");
        return -1;
    }
    Logger::log(Logger::All, "Client connect to server successfully.");

    return 0;
}

ssize_t MainClient::recv(void *buf, size_t len, int flags)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    return recv(m_sockfd, buf, len, flags);
}

ssize_t MainClient::send(const void *buf, size_t len, int flags)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    if(INVALID_SOCKET == m_sockfd)
        return -1;

    size_t nLen = 0;
    ssize_t ret = 0;
    do
    {
        ret = send(m_sockfd, buf + nLen, len - nLen, flags);
        if(ret < 0)
        {
            if(errno != EINTR)
                return ret;
            else    
                continue;
        }
        nLen += ret;
    }while(nLen != len)

    return nLen;
}

} // namespace Socket