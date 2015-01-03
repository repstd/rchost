#pragma once
#ifndef _HOST_H
#define _HOST_H
#endif

#ifdef _HOST_H

#include "stdafx.h"
#include <Windows.h>
#include <memory>
#include "server.h"
#include "rc_common.h"
#include <OpenThreads\Thread>
#include <OpenThreads\Mutex>
#include <map>

//Abstract of a class for finishing the tasks assigned to the host.
class HOST_OPERATOR_API 
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
protected:
	HOST_OPERATOR()
		:HOST_OPERATOR_API()
	{
		m_bIsDataLoaded = false;
	}
private:
	bool m_bIsDataLoaded;
};

static HOST_OPERATOR* g_hostOperator = NULL;
//Threads to serve the clients.
class HOST_SLAVE :public OpenThreads::Thread
{

public:
	HOST_SLAVE(const HOST_MSG* msg);
	virtual void handle() const;
	virtual void run();
	const OpenThreads::Mutex* getMutex() const;
protected:
	virtual void initMutex(OpenThreads::Mutex* mutex);
	std::auto_ptr<OpenThreads::Mutex> m_mutex;
	std::auto_ptr<HOST_MSG> m_taskMsg;
};

class HOST:protected server
{
public:
	HOST(const int port);
	~HOST()
	{
			
	}
	void run();
protected:
	HOST();
private:
	int m_port;
};
typedef std::map<std::string, std::string>::iterator HOST_MAP_ITER;
typedef std::map<std::string, PROCESS_INFORMATION>::iterator HOST_INFO_ITER;

#endif
