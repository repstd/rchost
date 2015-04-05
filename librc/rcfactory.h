#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include "server.h"
#include "client.h"
#include "rc_common.h"
#include "rcthread.h"
#include "rcpipe.h"
enum PROTOCOL
{
	UDP = 0,
	TCP
};

class rcfactory
{
public:
	static rcfactory* instance();
	tcpServer* createTcpServer(int port);
	udpServer* createUdpServer(int port);

	tcpClient* createTcpClient(const SOCKET socket);
	tcpClient* createTcpClient(int port, const char* serverAddr);
	tcpClient* createTcpClient();

	udpClient* createUdpClient(const SOCKET socket);
	udpClient* createUdpClient(int port, const char* serverAddr);
	udpClient* createUdpClient();
	
private:
	rcfactory();
	~rcfactory();
	std::unique_ptr<rcmutex_ext> m_mutex;
};

