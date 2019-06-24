#include "mainserver.h"

namespace Socket
{

MainServer::MainServer(const std::string &ip, unsigned int port)
    : m_sockfd(-1),
    m_ip(ip),
    m_port(port)
{
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
        //log
        return -1;
    }
    //log
    return 0;

}

} // namespace Socket