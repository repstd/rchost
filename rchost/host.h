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
struct cmp
{

	bool operator()(std::string a, std::string b)
	{

		return strcmp(a.c_str(), b.c_str());
	}

};
class PIPESIGNAL_BROCASTER :protected client, public THREAD
{

public:
	PIPESIGNAL_BROCASTER(const int port) :
		client(port, NULL),
		THREAD()
	{
		loadIP("ip.ini");
		__STD_PRINT("%s\n", "ip list loaded");
	}
	DWORD loadIP(const char* confg);
	virtual void run()
	{
		char msgRcv[_MAX_DATA_SIZE];
		int size = -1;
		while (isSocketOpen())
		{
			char* msg = "pipe";
			struct sockaddr from;
			sendPacket(msg, strlen(msg));

			while (1)
			{
				getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
				std::string ms(msgRcv);
				m_mapIpFlag[msgRcv] = 1;
				int allReceived = 1;
				for (std::map<std::string, int,cmp>::iterator iter = m_mapIpFlag.begin(); iter != m_mapIpFlag.end(); iter++)
				{

					if (iter->second == 0)
					{
						allReceived = 0;
						//_LOG_FORMAT_HOST("IPLIST End :%s\n", iter->first.c_str());
						break;
					}
				}
				if (allReceived==1)
				{
					for (std::map<std::string, int,cmp>::iterator iter = m_mapIpFlag.begin(); iter != m_mapIpFlag.end(); iter++)
					{
						iter->second = 0;
					}
					//__STD_PRINT("%s\n", "All Reveived.");
					break;
				}
			}
		}
	}
	virtual int cancel()
	{
		return THREAD::cancel();
	}

	std::map<std::string, int,cmp> m_mapIpFlag;


};
//Abstract of a class for finishing the tasks assigned to the host.
class HOST_OPERATOR_API :public HOST_CONFIG_API, public rcmutex
{

public:
	HOST_OPERATOR_API();
	~HOST_OPERATOR_API();
	virtual DWORD HandleProgram(std::string filename, const char op)
	{
		return 0;
	}
	virtual DWORD handleProgram(std::string filename, const char op, bool isCurDirNeeded);
protected:
	const char* getPath(std::string filename);
	const char* getArg(std::string filename);
	const char* getArg(std::string filename, std::string additional);

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
class HOST_OPERATOR
	:public HOST_OPERATOR_API
{
public:
	virtual DWORD loadConfig(const char* filename);
	virtual DWORD handleProgram(std::string filename, const char op);
	//We can use this to add timestamp and something else to the child process.

	void updateArg(std::string filename, std::string additional);

	static HOST_OPERATOR* instance();


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
	HOST_OPERATOR()
		:HOST_OPERATOR_API()
	{
		m_bIsDataLoaded = false;
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
	std::vector < std::unique_ptr<PIPESIGNAL_BROCASTER>> m_vecPipebroadercaster;

};

//For each host thread,we need a handler to finish message handling routines.
class HOST_MSGHANDLER :public THREAD, public rcmutex
{

public:
	HOST_MSGHANDLER(const HOST_MSG* msg);
	virtual void handle() const;
	virtual void run();
	const HOST_MSG* getMSG();

protected:

	void syncTime() const;
	std::unique_ptr<HOST_MSG> m_taskMsg;

};

//For each host thread,we need a listener to listen the specified port and recive the feedback from clients.

class PIPESERVER :public THREAD
{
public:

	PIPESERVER(const char* pipename)
	{

		m_pipeServer = std::unique_ptr<namedpipeServer>(new namedpipeServer(pipename));
	}
	virtual void run()
	{
		char log[512];
		SYSTEMTIME systime;
		GetLocalTime(&systime);
		_STD_ENCODE_TIMESTAMP(log, systime);
		_LOG_FORMAT_HOST("%s", log);

		_LOG_FORMAT_HOST("%s\n", "connected");
		while (1)
		{

			m_pipeServer->signalClient();
			memset(log, 0, 512);
			GetLocalTime(&systime);
			_STD_ENCODE_TIMESTAMP(log, systime);
			strcat(log, "#");
			strcat(log, m_pipeServer->getPipeName());
			_LOG_FORMAT_HOST("%s\n", log);
		}
		__STD_PRINT("%s\n", "pipe exit");
	}
	virtual void cancle()
	{

		m_pipeServer->disconnect();

		m_pipeServer->closeHandle();

		THREAD::cancel();

	}
	std::unique_ptr<namedpipeServer> m_pipeServer;
};

class HOST :protected server, public THREAD, rcmutex
{
public:
	HOST(const int port);
	~HOST()
	{

	}

	virtual void run();
	void addPipeServer(const char* pipename);
	void signalPipeClient();
	void queryHostInfo(HOST_OPERATOR* ope);
	const char* getName() const;
	const hostent* getHostent() const;
	const char* getIP() const;
	HOST_PIPE getPipeServers();
private:

	std::map < std::string, std::shared_ptr<namedpipeServer>> m_mapNamedPipeServer;
	int m_port;
};
class PIPESIGNAL_HANDLER :protected server, public THREAD
{
public:
	PIPESIGNAL_HANDLER(HOST* host, const int port) :
		server(port),
		THREAD()
	{
		m_host = std::unique_ptr<HOST>(host);
	}
	virtual void run()
	{
		char msgRcv[MAX_PATH];
		sockaddr client;
		int szRcv = -1;
		const char* received = HOST_OPERATOR::instance()->getHostName();
		while (isSocketOpen())
		{
			szRcv = -1;
			getPacket(client, msgRcv, szRcv, MAX_PATH);
			if (szRcv != -1)
			{
				char* msgStr = (char*)msgRcv;
				if (strstr(msgStr, "pipe") != NULL)
				{
					//__STD_PRINT("tring to %s\n", "SignalPipeClient");
					m_host->signalPipeClient();
				}
				sendPacket(client, const_cast<char*>(received), strlen(received) + 1, _MAX_DATA_SIZE);
			}

		}

	}
private:
	std::unique_ptr<HOST> m_host;
};
#endif
