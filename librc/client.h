#pragma once
#include "stdafx.h"
#include "rcsocket.h"
#ifndef MAX_LEN
#define MAX_LEN 300
#endif
#define PORT 6000
class rcfactory;
class udpClient :public rcsocket
{
public:
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
	virtual int initSocket(int port, ULONG S_addr);

	virtual void sendPacket(char* data, int size);
	virtual int initForPort(int port,const char* serverAddr=NULL);
	SOCKET getSocket();
	int setSocketNonBlock(int flag);
	virtual ~udpClient();
protected:
	udpClient();
	udpClient(int port,const char* serverAddr);
	udpClient(const SOCKET socket);
	friend rcfactory;
};
class tcpClient :public rcsocket
{
public:
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
	virtual int initSocket(int port, ULONG S_addr);

	virtual void sendPacket(char* data, int size);
	virtual int initForPort(int port,const char* serverAddr=NULL);
	SOCKET getSocket();
	int setSocketNonBlock(int flag);
	virtual ~tcpClient();
protected:
	tcpClient();
	tcpClient(int port,const char* serverAddr);
	tcpClient(const SOCKET socket);
private:
	SOCKET m_sClient;
	friend rcfactory;
	
};
/*
*@yulw,2015-4-4.Here the typedef is used to adapt to the old implementation of client.
*/
typedef udpClient client;