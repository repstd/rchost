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
#include "rcpipe.h"
#ifdef _LOG 
#define HOST_LOG_FILENAME "./host.log"
#define _LOG_INIT_HOST __LOG_INIT(HOST_LOG_FILENAME)
#define _LOG_FORMAT_HOST(format,data,...) __LOG_FORMAT(HOST_LOG_FILENAME,format,data) 
#else
#define HOST_LOG_FILENAME 
#define _LOG_INIT_HOST
#define _LOG_FORMAT_HOST(format,data,...) 
#endif

typedef std::map<std::string, std::string>::iterator HOST_MAP_ITER;
typedef std::map<std::string, PROCESS_INFORMATION>::iterator HOST_INFO_ITER;
typedef std::map < std::string, std::shared_ptr<namedpipeServer>> HOST_PIPE;
typedef std::map < std::string, std::shared_ptr<namedpipeServer>>::iterator HOST_PIPE_ITER;

//global variables
static std::map<const std::string, bool, cmp> g_mapIpFlag;
//Start a brocaster to signal the child process in the slave host
class PipeSignalBrocaster :protected client, public THREAD
{
public:
	PipeSignalBrocaster(const int port);
	DWORD loadIP(const char* confg);
	virtual void run();
	virtual int cancel();
private:
	std::map<const std::string, bool, cmp> m_mapIpFlag;
};
//A single thread which would reuse the soceket for collecting slave feedback.
class listenerSlave : public client, public THREAD, rcmutex
{
public:
	listenerSlave(SOCKET socket);
	virtual void run();
};

//A thread which would handle the feedback information from the salves and send signals back to them regularly.
class multiListener :protected client, public THREAD, public rcmutex
{
public:
	multiListener(const int port);
	~multiListener();
	DWORD loadIP(const char* confg);
	virtual void run();
	virtual int cancel();
private:
	std::vector<std::shared_ptr<listenerSlave>> m_vecSlaves;
};
//Abstract of a class for finishing the tasks assigned to the host.
class HostOperatorAPI :public HOST_CONFIG_API, public rcmutex
{

public:
	HostOperatorAPI();
	~HostOperatorAPI();
	virtual DWORD handleProgram(std::string filename, const char op) = 0;
	virtual DWORD handleProgram(std::string filename, const char op, bool isCurDirNeeded);
protected:
	std::string getPath(std::string filename);
	std::string getArg(std::string filename);
	std::string getArg(std::string filename, std::string additional);

	DWORD createProgram(std::string filename, std::string path, const char* curDir, std::string args, const int argc);
	virtual char* parsePath(const char* fullpath);
	DWORD createProgram(std::string filename, bool isCurDirNeeded);
	DWORD closeProgram(std::string filename);
	std::map<std::string, std::string> m_mapNamePath;
	std::map<std::string, std::string> m_mapNameArgs;
	std::map<std::string, std::string> m_mapNameAdditionInfo;
	std::map<std::string, PROCESS_INFORMATION> m_vecProgInfo;

};
//Concrete class for host_operator.
class HostOperator :public HostOperatorAPI
{
public:
	static HostOperator* instance();
	virtual DWORD loadConfig(const char* filename);
	virtual DWORD handleProgram(std::string filename, const char op);
	//We can use this to add timestamp and something else to the child process.
	void updateArg(std::string filename, std::string additional);
	void saveHostName(const char* hostname);
	const char* getHostName();
	void saveAdapter(const char* addr);
	const char* getPrimaryAdapter();
	void saveHostent(const hostent* host);
	const hostent* getHostent();
	void setPort(int port);
	const int getPort();
	DWORD loadIP(const char* confg);
protected:
	HostOperator()
		:HostOperatorAPI()
	{
		m_bIsDataLoaded = false;

	}
	~HostOperator()
	{
		delete[] m_inst;
		__STD_PRINT("%s\n", "host removed.");
	}
private:
	bool m_bQueit;
	bool m_bIsDataLoaded;
	//Host information
	char m_hostname[MAX_PATH];
	std::unique_ptr<hostent> m_host;
	int m_port;
	std::vector<std::string> m_vecAdapter;
	std::vector<std::string> m_vecClients;
	//std::auto_ptr<PipeSignalBrocaster> m_pipeBrocaster;
	std::vector < std::auto_ptr<multiListener>> m_vecPipebroadercaster;
	static HostOperator* m_inst;

};
//For each host thread,we need a handler to finish message handling routines.
class HostMsgHandler :public THREAD, public rcmutex
{

public:
	HostMsgHandler(const HOST_MSG* msg);

	virtual void handle() const;
	virtual void run();
	const HOST_MSG* getMSG();
	void setMSG(HOST_MSG* msg);

protected:

	void syncTime() const;
	std::auto_ptr<HOST_MSG> m_taskMsg;

};
//Concrete host implementation.
class host :protected server, public THREAD, rcmutex
{
public:
	host(const int port);
	~host()
	{

	}
	virtual void run();
	void addPipeServer(const char* pipename);
	void signalPipeClient();
	void queryHostInfo(HostOperator* ope);
	const char* getName() const;
	const hostent* getHostent() const;
	const char* getIP() const;
	HOST_PIPE getPipeServers();
private:
	std::map < std::string, std::shared_ptr<namedpipeServer>> m_mapNamedPipeServer;
	int m_port;
};

class PipeServer :public THREAD
{
public:
	PipeServer(const char* pipename);
	virtual void run();
	virtual void cancle();
private:
	std::unique_ptr<namedpipeServer> m_pipeServer;
};

//For each host thread,we need a listener to listen the specified port and recive the feedback from clients.
class PipeSignalHandler :protected server, public THREAD
{
public:
	PipeSignalHandler(host* phost, const int port);
	virtual void run();
private:
	std::unique_ptr<host> m_host;
};

#endif
