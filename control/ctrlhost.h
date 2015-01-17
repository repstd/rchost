#pragma once
#include "stdafx.h"
#include "rcthread.h"
#include <stdlib.h>
#include <map>

//A additional host running in the controlling client,and is mainly used for slave information storage.

struct StringComparerForMap
{
public:
	int operator()(std::string x, std::string y)
	{
		return strcmp(x.c_str(), y.c_str());
	}
};
typedef std::map<std::string, std::string,StringComparerForMap> CRTLhost_MAP;
typedef std::map<std::string, std::string,StringComparerForMap>::iterator CRThost_MAP_ITER;




class CTRLhost_OPERATOR :public HOST_CONFIG_API, public rcmutex
{

public:

	static CTRLhost_OPERATOR* instance();
	void addClientIP(const char* name,const char* ip);
	virtual DWORD loadConfig(const char* confg);
	virtual DWORD updateConfig(const char* confg);
	
protected:
	CTRLhost_OPERATOR()
		:HOST_CONFIG_API(), rcmutex()
	{
		initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	}
	CRTLhost_MAP m_mapNameIP;
};

