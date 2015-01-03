#pragma once
#include "stdafx.h"
#include <OpenThreads\Thread>
#ifndef _SERVER_H
#define _SERVER_H

class server
{
public:
	server(int port);
	~server();

public:
	bool initForPort(int portNumber);
	int getPort();
	bool isSocketOpen();

	bool getPacket(sockaddr& from, void *data, int &size, int maxSize);
	bool sendPacket(sockaddr to, void *data, int size, int maxSize);

private:
	int m_port;
	SOCKET m_netSocket;

};
#endif

