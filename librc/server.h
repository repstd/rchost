#pragma once
#include "stdafx.h"
#include "rcsocket.h"
#ifndef _SERVER_H
#define _SERVER_H

class server:public rcsocket
{
public:
	server(int port);
	~server();

public:
	int initForPort(int portNumber);
	virtual int initSocket(int port, ULONG S_addr);

};
#endif

