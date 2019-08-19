#include <iostream>
#include "log.h"
#include "httpserver.h"

int main()
{
    HTTPSERVER::HTTPServer httpserver("180.201.132.98", 3456);
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
