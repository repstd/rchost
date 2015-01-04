#pragma once
#include "stdafx.h"

#ifndef MAX_LEN
#define MAX_LEN 300
#endif
#define PORT 6000


class client 
{
public:
	client();
	~client();

public:

	int initForPort(int port,const char* serverAddr=NULL);
	void sendPacket(char* data, int size);
	bool getPacket(sockaddr& from, void *data, int &size, int maxSize);
	void close();

private:
	SOCKET				m_socket;
	SOCKADDR_IN			m_addrSvr;
	int					m_port;
	char*				m_svrAddr;
};

