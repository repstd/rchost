#pragma once
#include "stdafx.h"
#include <Windows.h>
#include <map>
#include <memory>
#include "server.h"
#include "client.h"
#include "rc_common.h"
#include "rcthread.h"
#include "rcfactory.h"
struct simpleCmd{
	simpleCmd() {
		path = "";
		arg = "";
		argc = 0;
	}
	std::string path;
	std::string arg;
	int argc;
};
class simplehost:protected server, public THREAD
{
public:
	simplehost(const int port);
	~simplehost();
	virtual void run();
private:
	const char* parsePath(const char* fullpath);
	void parseMsg(char* inMsg, simpleCmd& outMsg, const char* delimeter);
	DWORD handle(simpleCmd& msg);
	DWORD createProgram(std::string path, const char* curDir, std::string args, const int argc);
	DWORD closeProgram(std::string filename);
	bool isBlocked;
	std::map<std::string, PROCESS_INFORMATION> m_vecProgInfo;
};
