#pragma once
#include "host.h"
class HOST_API;
class osghost: public HOST_API
{
public:
	osghost(int port);
	~osghost();
public:
	virtual DWORD handle(const HOST_MSG* msg);
};

