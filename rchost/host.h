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

class HOST_API:
	public OpenThreads::Thread
{

public:
	HOST_API(int port);
	~HOST_API();
	virtual void run();
    virtual int start();
    virtual int startThread();
    virtual int cancel();
	const OpenThreads::Mutex* getMutex() const;
protected:
	//@deprecated
	void lock();
	void unlock();
	virtual DWORD handle(const HOST_MSG* msg)=0;
	virtual DWORD handleProgram(std::string filename, const char op,bool isCurDirNeeded);
	virtual void initMutex(OpenThreads::Mutex* mutex);
	virtual char* parsePath(const char* fullpath);
	//use pathes stored in a configuration file.
	virtual DWORD loadPathMap();
	DWORD createProgram(std::string filename,std::string path, const char* curDir,std::string args, const int argc);
	DWORD createProgram(std::string filename,bool isCurDirNeeded);
	DWORD closeProgram(std::string filename);
private:
	mutable std::auto_ptr<OpenThreads::Mutex> m_mutex;
	std::auto_ptr<server> m_pServer;
	std::map<std::string, std::string> m_mapNamePath;
	std::map<std::string, std::string> m_mapNameArgs;
	std::map<std::string, PROCESS_INFORMATION> m_vecProgInfo;
};
typedef std::map<std::string, std::string>::iterator HOST_MAP_ITER;
typedef std::map<std::string, PROCESS_INFORMATION>::iterator HOST_INFO_ITER;
#endif
