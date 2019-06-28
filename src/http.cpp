#include "http.h"
#include <cstring>

namespace HTTP
{

size_t addrlen(const char* start, const char* end)
{
    return (size_t)(end - start);
}

std::string extention_name(const std::string& basename)
{
    std::string extention("");
    size_t nPos = basename.find_last_of(".");
    if(nPos == std::string::npos)
        return extention;
    return basename.substr(nPos + 1);
}

const char* find_content(const char* start, const char* end, char endc, size_t& contlen, size_t& sumlen)
{
    size_t contentlen = 0;
    const char* content = nullptr;
    for(const char* mstart = start; mstart < end; ++mstart)
    {
        if(content == nullptr)
        {
            if(*mstart != ' ')
            {
                content = mstart;
                contentlen = 1;
            }
        }
        else
        {
            if(*mstart == endc)
            {
                contlen = contentlen;
                sumlen = addrlen(start, mstart);
                return content;
            }
            contentlen++;
        }
    }
    return nullptr;
}

const char* find_line(const char* start, const char* end)
{
    for(const char* lstart = start; lstart < (end - 1); ++lstart)
    {
        if(lstart[0] == '\r' && lstart[1] == '\n')
        {
            return &lstart[2];
        }
    }
}

const char* find_headline(const char* start, const char* end)
{
    for(const char* hstart = start; hstart < (end - 3); ++hstart)
    {
        if(hstart[0] == '\r' && hstart[1] == '\n' && hstart[2] == '\r' && hstart[3] == '\n')
            return &hstart[4];
    }
    return nullptr;
}

const char* http_content_type(const std::string& extension)
{
    if(extension.compare("html") == 0)
        return HTTP_HEAD_HTML_TYPE;
    else if(extension.compare("css") == 0)
        return HTTP_HEAD_CSS_TYPE;
    else if(extension.compare("gif") == 0)
        return HTTP_HEAD_GIF_TYPE;
    else if(extension.compare("jpg") == 0)
        return HTTP_HEAD_JPG_TYPE;
    else if(extension.compare("png") == 0)
        return HTTP_HEAD_PNG_TYPE;
    
    return nullptr;
}

//MyHTTPRequest实现
MyHTTPRequest::MyHTTPRequest()
    : m_strerr("success"),
    body(nullptr),
    bodylen(0)
{
}

MyHTTPRequest::~MyHTTPRequest()
{
    if(body != nullptr)
        delete body;
}

int MyHTTPRequest::loadPacket(const char* msg, size_t msglen)
{
    const char* remainmsg = msg;
    const char* endmsg = msg + msglen;
    const char* endline = find_line(remainmsg, endmsg);
    if(endline == nullptr)
        return -1;

    if(parserStartLine(remainmsg, endline) < 0)
        return -1;

    remainmsg = endline;
    const char* headline_end = find_headline(remainmsg, endmsg);
    if(headline_end == nullptr)
        return -1;

    if(parserHeaders(remainmsg, endmsg) < 0)
        return -1;
    remainmsg = headline_end;
    if(parserBody(remainmsg, endmsg) < 0)
        return -1;

    return 0;
}

const std::string& MyHTTPRequest::getStartLine()
{
    return m_startline;
}

const std::string& MyHTTPRequest::getMethod()
{
    return m_method;
}

const std::string& MyHTTPRequest::getUrl()
{
    return m_url;
}

const std::string& MyHTTPRequest::getVersion()
{
    return m_version;
}

const HTTPHeader_t& MyHTTPRequest::getHeader()
{
    return m_headers;
}

bool MyHTTPRequest::hasHeader(const std::string& header)
{
    HTTPHeader_t::iterator iter = m_headers.find(header);
    if(iter == m_headers.end())
        return false;
    return true;
}

const std::string& MyHTTPRequest::headerContent(const std::string& header)
{
    const static std::string nullstring("");
    HTTPHeader_t::iterator iter = m_headers.find(header);
    if(iter == m_headers.end())
        return nullstring;

    return iter->second;
}

const size_t MyHTTPRequest::bodyLen()
{
    return bodylen;
}

const char* MyHTTPRequest::getBody()
{
    return body;
}

const char* MyHTTPRequest::getError()
{
    return m_strerr.c_str();
}

void MyHTTPRequest::setStrError(const char* str)
{
    m_strerr = str;
}

int MyHTTPRequest::parserStartLine(const char* start, const char* end)
{
    size_t contentlen = 0;
    size_t sumlen = 0;

    const char* content = nullptr;
    const char* remainbuff = start;

    content = find_content(remainbuff, end, '\r', contentlen, sumlen);
    if(content == nullptr)
        return -1;

    m_startline = std::string(content, contentlen);
    content = find_content(remainbuff, end, ' ', contentlen, sumlen);
    if(content == nullptr)
        return -1;
    m_method = std::string(content, contentlen);
    remainbuff += sumlen;

    content = find_content(remainbuff, end, ' ', contentlen, sumlen);
    if(content == nullptr)
        return -1;
    m_url = std::string(content, contentlen);
    remainbuff += sumlen;

    content = find_content(remainbuff, end, '\r', contentlen, sumlen);
    if(content == nullptr)
        return -1;
    m_version = std::string(content, contentlen);
    return 0;
}

int MyHTTPRequest::parserHeaders(const char* start, const char* end)
{
    size_t contentlen = 0;
    size_t sumlen = 0;

    const char* line_start = start;
    std::string head, attr;
    m_headers.clear();

    while(true)
    {
        const char* line_end = find_line(line_start, end);
        if(line_end == nullptr)
            return -1;
        else if(line_end == end)
            break;

        const char* headstart = find_content(line_start, line_end, ':', contentlen, sumlen);
        if(headstart == nullptr)
            return -1;
        head = std::string(headstart, contentlen);

        const char* attrstart = line_start + sumlen + 1;
        attrstart = find_content(attrstart, line_end, '\r', contentlen, sumlen);
        if(attrstart == nullptr)
            return -1;
        attr = std::string(attrstart, contentlen);

        line_start = line_end;
        m_headers[head] = attr;
    }
    return 0;
}

int MyHTTPRequest::parserBody(const char* start, const char* end)
{
    size_t body_len = addrlen(start, end);
    if(body_len == 0)
        return 0;

    char* buff = new char[body_len];
    if(buff == nullptr)
        return -1;
    memcpy(buff, start, body_len);

    if(body != nullptr)
        delete body;
    body = buff;
    bodylen = body_len;
    return 0;
}

//MyHTTPResponse类实现
MyHTTPResponse::MyHTTPResponse()
{
    m_package.body = nullptr;
    m_package.bodylen = 0;
    m_package.data = nullptr;
    m_package.datalen = 0;
    m_package.dirty = true;
}

MyHTTPResponse::~MyHTTPResponse()
{
    if(m_package.body != nullptr)
        delete[] m_package.body;
    if(m_package.data != nullptr)
        delete[] m_package.data;
}

int MyHTTPResponse::setVersion(const std::string& version)
{
    m_package.version = version;
    m_package.dirty = true;
    return 0;
}

int MyHTTPResponse::setStatus(const std::string& status, const std::string& reason)
{
    m_package.status = status;
    m_package.reason = reason;
    m_package.dirty = true;
    return 0;
}

int MyHTTPResponse::addHeader(const std::string& header, const std::string& content)
{
    if(header.empty() || content.empty())
        return -1;

    m_package.headers[header] = content;
    m_package.dirty = true;
    return 0;
}

int MyHTTPResponse::delHeader(const std::string& header)
{
    if(header.empty())
        return -1;

    HTTPHeader_t::iterator iter = m_package.headers.find(header);
    if(iter == m_package.headers.end())
        return -1;
    
    m_package.headers.erase(iter);
    m_package.dirty = true;
    return 0;
}

int MyHTTPResponse::setBody(const std::string& body, size_t bodylen)
{
    if(body.empty() || bodylen == 0 || bodylen > BODYMAXSIZE)
        return -1;

    char* buff = new char[bodylen];
    if(buff == nullptr)
        return -1;

    memcpy(buff, body.c_str(), bodylen);
    if(m_package.body != nullptr)
        delete[] m_package.body;
    m_package.body = buff;
    m_package.bodylen = bodylen;
    m_package.dirty = true;
    return 0;
}

size_t MyHTTPResponse::size()
{
    if(m_package.dirty)
    {
        m_package.totallen = startLineSize() + headersSize();
        m_package.totallen += m_package.bodylen;
    }
    return m_package.totallen;
}

const char* MyHTTPResponse::serialize()
{
    if(!m_package.dirty)
        return m_package.data;

    size_t totalsize = size();
    char* buffserver = new char[totalsize];
    if(buffserver == nullptr)
        return "";
    
    char* buff = buffserver;
    int nprint = snprintf(buff, totalsize, "%s %s %s\r\n", m_package.version.c_str(), m_package.status.c_str(), m_package.reason.c_str());
    if(nprint < 0)
    {
        delete buffserver;
        return nullptr;
    }

    totalsize -= nprint;
    buff += nprint;

    for(HTTPHeader_t::iterator iter = m_package.headers.begin(); iter != m_package.headers.end(); ++iter)
    {
        const std::string& name = iter->first;
        const std::string& attr = iter->second;

        nprint = snprintf(buff, totalsize, "%s: %s\r\n", name.c_str(), attr.c_str());
        if(nprint < 0)
        {
            delete buffserver;
            return nullptr;
        }

        totalsize -= nprint;
        buff += nprint;
    }

    nprint = snprintf(buff, totalsize, "\r\n");
    if(nprint < 0)
    {
        delete buffserver;
        return nullptr;
    }
    totalsize -= nprint;
    buff += nprint;

    memcpy(buff, m_package.data, totalsize);
    if(totalsize != m_package.bodylen)
    {
        //todo
    }
    if(m_package.data != nullptr)
        delete m_package.data;
    m_package.data = buffserver;
    m_package.dirty = false;
    return m_package.data;
}

size_t MyHTTPResponse::headersSize()
{
    const size_t othrechar_size = 2 + 2;
    const size_t heade_terminatorsize = 2;

    size_t stringsize = 0; 
    HTTPHeader_t::iterator iter = m_package.headers.begin();
    for(; iter != m_package.headers.end(); ++iter)
    {
        const std::string& name = iter->first;
        const std::string& attr = iter->second;

        stringsize += name.size() + attr.size() + othrechar_size;
    }
    stringsize += heade_terminatorsize;
    return stringsize;
}

size_t MyHTTPResponse::startLineSize()
{
    const size_t otherchar_size = 1 * 2 + 2;
    size_t totalsize = otherchar_size + m_package.version.size();
    totalsize += m_package.status.size() + m_package.reason.size();
    return totalsize;
}

} // namespace HTTP