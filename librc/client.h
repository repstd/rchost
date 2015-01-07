#pragma once
#include "stdafx.h"
#include "rcsocket.h"
#ifndef MAX_LEN
#define MAX_LEN 300
#endif
#define PORT 6000


class client :public rcsocket
{
public:
	client();
	client(int port,const char* serverAddr);
	~client();
	virtual void sendPacket(char* data, int size);
public:
	int initForPort(int port,const char* serverAddr=NULL);
	virtual int initSocket(int port, ULONG S_addr);
};

