#include <iostream>
#include "log.h"
#include "httpserver.h"

int main()
{
    HTTPSERVER::HTTPServer httpserver("127.0.0.1", 8080);
	Logger::log(Logger::All, "HTTPServer starting...");
    if(httpserver.start(1024) < 0)
        return -1;

    while(true)
    {
        char buff[1024];
        char* cmd = fgets(buff, sizeof(buff), stdin);
        if(cmd == nullptr)
            break;
    }

    return 0;
}
