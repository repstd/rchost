#pragma once
#include "stdafx.h"
#include "rcthread.h"
#include <stdlib.h>
#include <map>

//A additional host running in the controlling client,and is mainly used for slave information storage.

struct StringComparerForMap
{
public:
	bool operator()(std::string x, std::string y)
	{
		return strcmp(x.c_str(), y.c_str());
	}
};
typedef std::map<std::string, std::string,StringComparerForMap> CRTLHOST_MAP;
typedef std::map<std::string, std::string,StringComparerForMap>::iterator CRTHOST_MAP_ITER;




class CTRLHOST_OPERATOR :public HOST_CONFIG_API, public rcmutex
{

public:

	static CTRLHOST_OPERATOR* instance();
	void addClientIP(const char* name,const char* ip);
	virtual DWORD loadConfig(const char* confg);
	virtual DWORD updateConfig(const char* confg);
	
protected:
	CTRLHOST_OPERATOR()
		:HOST_CONFIG_API(), rcmutex()
	{
		initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	}
	CRTLHOST_MAP m_mapNameIP;
};