#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "http.h"
#include "event.h"
#include "mainclient.h"
#include "mainserver.h"
#include <iostream>

namespace HTTPSERVER
{

#define HTTP_ROOT "Html"
#define HTTP_DEFAULTFILE 	"index.html"
#define HTTP_SLASH 			"/"
#define HTTP_CURRENT_DIR	"."
#define HTTP_ABOVE_DIR 		".."

std::string httpPathHandle(const std::string& dname, const std::string& bname);
void splitUrl(const std::string& url, std::string& dir, std::string& base);

class HTTPStream : public Event::Handler
{
public:
    HTTPStream(SocketInterface::ClientInterface* client);
    ~HTTPStream();
    int close();

protected:
    void epoll_handlein(int);
    void epoll_handleout(int);
    void epoll_handleclose(int);
    void epoll_handleerr(int);

private:
    HTTP::HTTPResponse* handle_request(HTTP::HTTPRequest&);
    enum CONFIG
    {
        READBUFFLEN = 1024,
    };
    SocketInterface::ClientInterface* m_client;
    std::mutex m_mutex;
    char* m_readbuf;
}; // HTTPStream

class HTTPServer : public Event::Handler
{
public:
    HTTPServer(const std::string& ip, unsigned int port);
    ~HTTPServer();

    int start(int backlog);
    int close();

protected:
    void epoll_handlein(int);
    void epoll_handleout(int);
    void epoll_handleclose(int);
    void epoll_handleerr(int);

private:
    std::mutex m_mutex;
    const std::string m_ip;
    unsigned int m_port;
    Socket::MainServer m_server;
}; // HTTPServer

} // namespace HTTPServer

#endif // HTTPSERVER_H_