#pragma once
#include "stdafx.h"
#include "rcsocket.h"
#ifndef _SERVER_H
#define _SERVER_H
class rcfactory;
class udpServer :public rcsocket
{
public:
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
	virtual int initSocket(int port, ULONG S_addr);
	virtual int initForPort(int portNumber);
	virtual ~udpServer();
protected:	
	udpServer(int port);
	friend rcfactory;
};

class tcpServer :public rcsocket
{
public:
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
	virtual int initSocket(int port, ULONG S_addr);
	virtual int initForPort(int portNumber);
	virtual ~tcpServer();
protected:	
	tcpServer(int port);
	friend rcfactory;
private:
	SOCKET m_sClient;
};
/*
*@yulw,2015-4-4.Here the typedef is used to adapt to the old implementation of server.
*/
typedef udpServer server;
#endif

