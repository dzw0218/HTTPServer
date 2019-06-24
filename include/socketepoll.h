
/**
 * socket class for setup socket.
 * date: 2019.06.21
 * */

#ifndef SOCKETEPOLL_H_
#define SOCKETEPOLL_H_

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <cstring>

namespace SOCKETEPOLL
{

#define LISTEN_NUM 1024
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR -1
#define Handler int
#define MAX_CLIENT 1000

class SocketEpoll
{
public:
	SocketEpoll();
	virtual ~SocketEpoll();

	SOCKET initSocket();
	int bindServer(const std::string ip, const unsigned port);
	int listenServer(const int backlog);
	SOCKET acceptClient();
	void closeSocket();
	bool isRun();
	bool epollRun();

private:
	SOCKET _socket;
	Handler _efd;
	bool oneFlag;
	struct epoll_event tep, ep[MAX_CLIENT];
}; // SocketEpoll

} // SOCKETEPOLL

#endif // SOCKETEPOLL_H_
