#include "httpserver.h"
#include "log.h"
#include <cstring>
#include <sys/stat.h>
#include <libgen.h>

namespace HTTPSERVER
{

std::string httpPathHandle(const std::string& dname, const std::string& bname)
{
    std::string filepath(HTTP_ROOT);
    if(bname == HTTP_CURRENT_DIR || bname == HTTP_SLASH || bname == HTTP_ABOVE_DIR)
    {
        filepath += HTTP_SLASH;
        filepath += HTTP_DEFAULTFILE;
    }
    else if(dname == HTTP_CURRENT_DIR)
    {
        filepath += HTTP_SLASH;
        filepath += bname;
    }
    else if(dname == HTTP_SLASH)
    {
        filepath += dname;
        filepath += bname;
    }
    else
    {
        filepath += dname;
        filepath += HTTP_SLASH;
        filepath += bname;
    }

    return filepath;
}

void splitUrl(const std::string& url, std::string& dir, std::string& base)
{
    char* cUrl = strdup(url.c_str());
    dir = dirname(cUrl);
    delete cUrl;

    char* basec = strdup(url.c_str());
    base = basename(basec);
    delete basec;
}

//HTTPStream实现
HTTPStream::HTTPStream(SocketInterface::ClientInterface* client)
    : m_client(client)
{
    m_readbuf = new char[READBUFFLEN + 1];
    m_client->setNonBlocking(true);
    register_event(*m_client);
}

HTTPStream::~HTTPStream()
{
    delete m_client;
    m_client = nullptr;

    delete m_readbuf;
    m_readbuf = nullptr;
}

int HTTPStream::close()
{
    return shutdown_event(*m_client);
}

void HTTPStream::epoll_handlein(int fd)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    size_t nread = m_client->recv(m_readbuf, READBUFFLEN, MSG_DONTWAIT);
    if((nread < 0 && nread != EAGAIN) || nread == 0)
    {
        close();
        return;
    }
    else if(nread < 0 && nread != EAGAIN)
    {
        return;
    }

    m_readbuf[nread] = 0;

    HTTP::MyHTTPRequest httprequest;
    if(httprequest.loadPacket(m_readbuf, nread) < 0)
    {
        return;
    }

    HTTP::HTTPResponse *response = handle_request(httprequest);
    if(response != nullptr)
    {
        m_client->send(response->serialize(), response->size(), 0);
        delete response;
    }
}

void HTTPStream::epoll_handleout(int fd)
{

}

void HTTPStream::epoll_handleclose(int fd)
{
    delete this;
}

void HTTPStream::epoll_handleerr(int fd)
{
    close();
}

HTTP::HTTPResponse* HTTPStream::handle_request(HTTP::HTTPRequest& request)
{
    const std::string& method = request.getMethod();
    const std::string& url = request.getUrl();

    std::string dname, bname;
    splitUrl(url, dname, bname);

    HTTP::MyHTTPResponse* response = new HTTP::MyHTTPResponse();
    std::string filepath = httpPathHandle(dname, bname);
    if(method == "GET")
    {
        std::string extention = HTTP::extention_name(filepath);
        if(extention.empty() || access(filepath.c_str(), R_OK) < 0)
        {
            response->setVersion(HTTP_VERSION);
            response->setStatus("404", "Not Found");
            response->addHeader(HTTP_HEAD_CONNECTION, "close");
            return response;
        }

        struct stat filestat;
        stat(filepath.c_str(), &filestat);
        const size_t filesize = filestat.st_size;

        char* fbuff = new char[filesize];

        FILE* fp = fopen(filepath.c_str(), "rb");
        if(fp == nullptr || fread(fbuff, filesize, 1, fp) != 1)
        {
            delete fbuff;

            response->setVersion(HTTP_VERSION);
            response->setStatus("500", "Internal Server Error");
            response->addHeader(HTTP_HEAD_CONNECTION, "close");
            return response;
        }

        fclose(fp);

        char sfilesize[16] = {0};
        snprintf(sfilesize, sizeof(sfilesize), "%ld", filesize);

        response->setVersion(HTTP_VERSION);
        response->setStatus("200", "OK");
        response->addHeader(HTTP_HEAD_CONTENT_TYPE, HTTP::http_content_type(extention));
        response->addHeader(HTTP_HEAD_CONTENT_LEN, sfilesize);
        response->addHeader(HTTP_HEAD_CONNECTION, "close");
        response->setBody(fbuff, filesize);
        delete fbuff;
    }
    return response;
}

//HTTPServer实现
int HTTPServer::start(int backlog)
{
    if(m_server.isRun())
	{
		Logger::log(Logger::Console, "Server is running...");
        return 0;
	}
    
    if(m_server.initSocket(backlog) < 0)
	{
		Logger::log(Logger::Console, "Initialize socket failed.");
        return -1;
	}

    if(m_server.setNonBlocking(true) < 0)
    {
		Logger::log(Logger::Console, "Server set no block failed.");
	}

    return register_event(m_server);
}

int HTTPServer::close()
{
    return shutdown_event(m_server);
}

HTTPServer::HTTPServer(const std::string& ip, unsigned int port)
    : m_port(port),
    m_ip(ip),
    m_server(ip, port)
{
}

HTTPServer::~HTTPServer()
{}

void HTTPServer::epoll_handlein(int fd)
{
    do
    {
        SocketInterface::ClientInterface* newcon = m_server.acceptSocket();
        if(newcon == nullptr)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                break;
            else
            {
                close();
                return;
            }
        }

        HTTPStream* httpstream = new HTTPStream(newcon);
    }while(true);
}

void HTTPServer::epoll_handleout(int fd)
{

}

void HTTPServer::epoll_handleclose(int fd)
{

}

void HTTPServer::epoll_handleerr(int fd)
{
    close();
}

}
