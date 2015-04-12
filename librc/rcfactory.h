#pragma once
#include "stdafx.h"
#include "server.h"
#include "client.h"
#include "rc_common.h"
#include "rcthread.h"
#include "rcmemshare.h"
#include "rcpipe.h"
#include "rcosg.h"
#include <unordered_map>
bool isReady();
void setStatus(bool status);
enum PROTOCOL
{
	UDP = 0,
	TCP
};
class rcfactory
{
public:
	static rcfactory* instance();
	tcpServer*	createTcpServer(int port);
	udpServer*	createUdpServer(int port);
	tcpClient*	createTcpClient(const SOCKET socket);
	tcpClient*	createTcpClient(int port, const char* serverAddr);
	tcpClient*	createTcpClient();
	udpClient*	createUdpClient(const SOCKET socket);
	udpClient*	createUdpClient(int port, const char* serverAddr);
	udpClient*	createUdpClient();
	rcFileMapWriter* createFileMapWriter(char* name);
	rcFileMapReader* createFileMapReader(char* name);
	rcOsgHostClient* createOsgHostClient(char* name, int port);
	rcOsgHostServer* createOsgHostServer(char* name, int port);
	int			getPortStatic(int port);
private:
	rcfactory();
	~rcfactory();
	std::unordered_map<int, int> m_mapUsedPort;
	std::unique_ptr<rcmutex_ext> m_mutex;
};

