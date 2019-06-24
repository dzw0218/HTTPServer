#include "socketepoll.h"

namespace SOCKETEPOLL
{

SocketEpoll::SocketEpoll()
		: _socket(INVALID_SOCKET),
		oneFlag(true)
{
	
}

SocketEpoll::~SocketEpoll()
{
	closeSocket();
}

SOCKET SocketEpoll::initSocket()
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(SOCKET_ERROR == _socket)
	{
		std::cout /*<< getHintInfor(SERVER)*/ << "socket created failed." << std::endl;
		return INVALID_SOCKET;
	}
	std::cout /*<< getHintInfor(SERVER)*/ << "socket created successfully." << std::endl;
	return _socket;
}

int SocketEpoll::bindServer(const std::string ip, const unsigned port)
{
	if(INVALID_SOCKET == _socket)
		initSocket();

	port = ((port <= 1024) || (port >= 65535)) ? 6666 : port;
	//清除bind时出现“Address already in use”
	int optval = 0;
	if(setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1)
		return SOCKET_ERROR;

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	_sin.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
	int ret = bind(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));
									
	if(SOCKET_ERROR == ret)
	{
		std::cout /*<< getHintInfor(SERVER)*/ << "bind server failed." << std::endl;
		return SOCKET_ERROR;
	}
	std::cout /*<< getHintInfor(SERVER)*/ << "bind server " << ip << ":" << port << " successfully." << std::endl;
	return ret;
}

int SocketEpoll::listenServer(const int backlog)
{
	int ret = listen(_socket, backlog);
	if(ret < 0)
	{
		std::cout /*<< getHintInfor(SERVER)*/ << "listen to server failed." << std::endl;
		return -1;
	}
	else
		std::cout /*<< getHintInfor(SERVER)*/ << "listen to server successfully." << std::endl;
	return ret;
}

int SocketEpoll::nonblockingSocket(SOCKET listenSock)
{
	int flag = fcntl(listenSock, F_GETFL, 0);
	if(-1 == flag)
		return -1;

	flag |= O_NONBLOCK;
	if(fcntl(listen, F_SETFL, flag) == -1)
		return -1;
}

SOCKET SocketEpoll::acceptClient()
{
	SOCKET _cSock = INVALID_SOCKET;
	sockaddr_in _sin = {};
	int nAddrLen = sizeof(_sin);
	_cSock = accept(_socket, (sockaddr*)&_sin, (socklen_t*)&nAddrLen);
							
	if(INVALID_SOCKET == _cSock)
	{
		std::cout /*<< getHintInfor(SERVER)*/ << "connect to client failed." << std::endl;
		return SOCKET_ERROR;
	}
	std::cout /*<< getHintInfor(CLIENT)*/ << "client " << inet_ntoa(_sin.sin_addr) << ":" << ntohs(_sin.sin_port) << " connected to server" << std::endl;
	nonblockingSocket(_cSock); // 将监听到的客户端设为非阻塞模式
	//_clients.push_back(new ClientSocket(_cSock));
	return _cSock;
}

void SocketEpoll::closeSocket()
{
	/*if(_socket != INVALID_SOCKET)
	{
		for(size_t index = 0; index < _clients.size(); ++index)
		{
			delete _clients[index];
			_clients[index] = nullptr;
		}
		close(_socket);
		_clients.clear();
	}*/
}

bool SocketEpoll::epollRun()
{
	if(!isRun())
		return false;

	if(oneFlag)
	{
		oneFlag = false;
		efd = epoll_create(MAX_CLIENT);
		if(-1 == efd)
		{
			std::cout /*<< getHintInfor(SERVER)*/ << "epoll create failed." << std::endl;
			return false;
		}
		tep.events = EPOLLIN | EPOLLET;
		tep.data.fd = _socket;
		int ret = epoll_ctl(efd, EPOLL_CTL_ADD, _socket, &tep);
		if(-1 == ret)
		{
			std::cout /*<< getHintInfor(SERVER)*/ << "server add to epoll failed." << std::endl;
			return false;
		}
		return true;
	}
	else
	{
		int nReady = epoll_wait(efd, ep, AMX_CLIENT, -1);
		if(-1 == nReady)
		{
			std::cout /*<< getHintInfor(SERVER)*/ << "server failed to wait for connection." << std::endl;
			return false;
		}

		for(int index = 0; index < nReady; ++index)
		{
			if(!(ep[index].events & EPOLLIN))
				continue;
			if(ep[index].data.fd == _socket)
			{
				SOCKET _cSock = acceptClient();
				tep.events = EPOLLIN | EPOLLET;
				tep.data.fd = _cSock;
				int ret = epoll_ctl(efd, EPOLL_CTL_ADD, _cSock, &tep);
				if(-1 == ret)
					std::cout /*<< getHintInfor(SERVER)*/ << "add client to epoll failed." << std::endl;
			}
			else
			{
			}
		}
	}
}

bool SocketEpoll::isRun()
{
			return _socket != INVALID_SOCKET;
}

} // SOCKETEPOLL
