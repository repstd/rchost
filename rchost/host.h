#pragma once
#ifndef _HOST_H
#define _HOST_H
#endif

#ifdef _HOST_H

#include "stdafx.h"
#include <Windows.h>
#include <map>
#include <memory>
#include "server.h"
#include "client.h"
#include "rc_common.h"
#include "rcthread.h"
//Abstract of a class for finishing the tasks assigned to the host.
class HOST_OPERATOR_API:public rcmutex
{

public:
	HOST_OPERATOR_API();
	~HOST_OPERATOR_API();
	virtual DWORD handleProgram(std::string filename, const char op,bool isCurDirNeeded);
protected:
	DWORD createProgram(std::string filename,std::string path, const char* curDir,std::string args, const int argc);
	virtual char* parsePath(const char* fullpath);
	//use pathes stored in a configuration file.
	virtual DWORD loadPathMap(const char* filename) = 0;
	DWORD createProgram(std::string filename,bool isCurDirNeeded);
	DWORD closeProgram(std::string filename);
	std::map<std::string, std::string> m_mapNamePath;
	std::map<std::string, std::string> m_mapNameArgs;
	std::map<std::string, PROCESS_INFORMATION> m_vecProgInfo;
	FILE* m_fConfig;
};

//Concrete class for host_operator.
class HOST_OPERATOR
	:public HOST_OPERATOR_API
{
public:
	static HOST_OPERATOR* instance();
	virtual DWORD loadPathMap(const char* filename);

	const char* getPath(std::string filename);
	const char* getArg(std::string filename);
	void saveHostName(const char* hostname);
	const char* getHostName();
	void saveAdapter(const char* addr);
	const char* getPrimaryAdapter();
	void saveHostent(const hostent* host);
	const hostent* getHostent();
protected:
	HOST_OPERATOR()
		:HOST_OPERATOR_API()
	{
		m_bIsDataLoaded = false;
	}
private:
	bool m_bIsDataLoaded;	
	//Host information
	char m_hostname[MAX_PATH];
	std::auto_ptr<hostent> m_host;
	std::vector<std::string> m_vecAdapter;
	std::vector<std::string> m_vecClients;
};

//For each host thread,we need a handler to finish message handling routines.
class HOST_MSGHANDLER :public THREAD,public rcmutex
{

public:
	HOST_MSGHANDLER(const HOST_MSG* msg);
	virtual void handle() const;
	virtual void run();
protected:
	void syncTime() const;
	std::auto_ptr<HOST_MSG> m_taskMsg;
};

//For each host thread,we need a listener to listen the specified port and recive the feedback from clients.

class HOST:protected server,public THREAD,rcmutex
{
public:
	HOST(const int port);
	~HOST()
	{
	}
	virtual void run();
	void queryHostInfo(HOST_OPERATOR* ope);
	const char* getName();
	const hostent* getHostent();
private:
	int m_port;
};
typedef std::map<std::string, std::string>::iterator HOST_MAP_ITER;
typedef std::map<std::string, PROCESS_INFORMATION>::iterator HOST_INFO_ITER;

#endif
