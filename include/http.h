#ifndef HTTP_H_
#define HTTP_H_

#include <iostream>
#include <string>
#include <map>

namespace HTTP
{
#define HTTP_VERSION			"HTTP/1.1"

#define HTTP_HEAD_CONTENT_TYPE	"Content-type"
#define HTTP_HEAD_CONTENT_LEN	"Content-length"
#define HTTP_HEAD_CONNECTION	"Connection"
#define HTTP_HEAD_KEEP_ALIVE	"Keep-Alive"

#define HTTP_ATTR_KEEP_ALIVE 	"keep-alive"

#define HTTP_HEAD_HTML_TYPE		"text/html"
#define HTTP_HEAD_CSS_TYPE		"text/css"
#define HTTP_HEAD_GIF_TYPE		"image/gif"
#define HTTP_HEAD_JPG_TYPE		"image/jpeg"
#define HTTP_HEAD_PNG_TYPE		"image/png"

size_t addrlen(const char *start, const char *end);
std::string  extention_name(const std::string &basename);

const char *find_content(const char *start, const char *end, char endc, size_t &contlen, size_t &sumlen);
const char *find_line(const char *start, const char *end);
const char *find_headline(const char *start, const char *end);
const char *http_content_type(const std::string &extension);

typedef std::map<std::string, std::string> HTTPHeader_t;

class HTTPRequest
{
public:
    ~HTTPRequest(){};

    virtual const std::string& getStartLine() = 0; //获取起始行
    virtual const std::string& getMethod() = 0; //获取报文方法
    virtual const std::string& getUrl() = 0; //获取报文的URl
    virtual const std::string& getVersion() = 0; //获取报文版本
    virtual const HTTPHeader_t& getHeader() = 0; //获取报文头map
    virtual bool hasHeader(const std::string&) = 0; //检测是否拥有该头部
    virtual const std::string& headerContent(const std::string&) = 0; //获取报文头部属性内容
    virtual const size_t bodyLen() = 0; //获取报文长度
    virtual const char* getBody() = 0; //获取报文内容 
};

class HTTPResponse
{
public:
    ~HTTPResponse(){};

    virtual int setVersion(const std::string&) = 0; //设置报文版本
    virtual int setStatus(const std::string&, const std::string&) = 0; //设置报文状态
    virtual int addHeader(const std::string&, const std::string&) = 0; //增加报文头部
    virtual int delHeader(const std::string&) = 0; //删除报文头部信息
    virtual int setBody(const std::string&, size_t) = 0; //设置报文主体
    virtual size_t size() = 0; //获取报文长度
    virtual const char* serialize() = 0; //序列化http报文
};

class MyHTTPRequest : public HTTPRequest
{
public:
    MyHTTPRequest();
    ~MyHTTPRequest();

    int loadPacket(const char* msg, size_t msglen);
    const std::string& getStartLine();
    const std::string& getMethod();
    const std::string& getUrl();
    const std::string& getVersion();
    const HTTPHeader_t& getHeader();
    bool hasHeader(const std::string& header);
    const std::string& headerContent(const std::string& header);
    const size_t bodyLen();
    const char* getBody();
    const char* getError();

private:
    inline void setStrError(const char* str);
    int parserStartLine(const char* start, const char* end);
    int parserHeaders(const char* start, const char* end);
    int parserBody(const char* start, const char* end);

private:
    std::string m_strerr;
    std::string m_startline;
    std::string m_method;
    std::string m_url;
    std::string m_version;

    HTTPHeader_t m_headers;

    char* body;
    size_t bodylen;
};

class MyHTTPResponse : public HTTPResponse
{
public:
    MyHTTPResponse();
    ~MyHTTPResponse();

    int setVersion(const std::string& version);
    int setStatus(const std::string& status, const std::string& reason);
    int addHeader(const std::string& header, const std::string& content);
    int delHeader(const std::string& header);
    int setBody(const std::string& body, size_t bodylen);
    size_t size();
    const char* serialize();

private:
    size_t headersSize();
    size_t startLineSize();

private:
    enum Config
    {
        MAXLINE = 1024,
        BODYMAXSIZE = 64*1024,
    };
    typedef struct
    {
        std::string version;
        std::string status;
        std::string reason;
        HTTPHeader_t headers;
        char* body;
        size_t bodylen;
        char* data;
        size_t datalen;
        size_t totallen;
        bool dirty;
    }response_t;
    response_t m_package;
};

} // namespace HTTP
#endif // HTTP_H_