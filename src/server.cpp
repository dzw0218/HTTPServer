#include <iostream>
#include "log.h"
#include "httpserver.h"

int main()
{
    HTTPSERVER::HTTPServer httpserver("0.0.0.0", 8080);
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