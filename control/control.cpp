// control.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "client.h"
#include "rc_common.h"
#include "rcthread.h"
#include "../rchost/host.h"
#include "ctrlhost.h"
#include <stdio.h>
#include <memory>
#include <iostream>
static char* test_instruction[2] =
{
	"image 1 0",
	"image 0 0"
};

class host_LISTENER : public THREAD, rcmutex
{
public:
	host_LISTENER::host_LISTENER(const client* cl)
		:THREAD(), rcmutex()
	{
		m_client = std::unique_ptr<client>(const_cast<client*>(cl));
	}
	virtual void run()
	{

		char msgRcv[_MAX_DATA_SIZE];
		memset(msgRcv, 0, _MAX_DATA_SIZE);
		sockaddr client;
		int sizeRcv;
		while (m_client->isSocketOpen())
		{
			sizeRcv = -1;
			m_client->getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
			//if (GetLastError() == WSAEWOULDBLOCK)
			//	Sleep(10);
			if (sizeRcv != -1 && sizeRcv != sizeof(HOST_MSG))
				__STD_PRINT("%s\n", msgRcv);
#if 0 
			char* delimeter = strstr(msgRcv, "#");
			if (delimeter)
			{
				*delimeter = '\0';
				__DEBUG_PRINT("%s\n", msgRcv);
				__DEBUG_PRINT("%s\n", delimeter + 1);
				CTRLhost_OPERATOR::instance()->addClientIP(msgRcv, delimeter + 1);
			}
			CTRLhost_OPERATOR::instance()->updateConfig("clients.ini");
#endif
			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}

	std::unique_ptr<client> m_client;
};

class hostLISTENER : public THREAD, rcmutex, client
{
public:
	hostLISTENER::hostLISTENER(const SOCKET socket)
		:THREAD(), rcmutex(), client(socket)
	{
		initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	}
	virtual void run()
	{
		
		char msgRcv[_MAX_DATA_SIZE];
		memset(msgRcv, 0, _MAX_DATA_SIZE);
		sockaddr client;
		int sizeRcv;
		while (isSocketOpen())
		{
			sizeRcv = -1;
			getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
			//if (GetLastError() == WSAEWOULDBLOCK)
			//	Sleep(10);
			__STD_PRINT("ThreadID: %d\t", getThreadId());
			if (sizeRcv != -1 && sizeRcv != sizeof(HOST_MSG))
			{
				__STD_PRINT("%s\n", msgRcv);
			}
			char* delimeter = strstr(msgRcv, "#");
			if (delimeter)
			{
				*delimeter = '\0';
				__DEBUG_PRINT("%s\n", msgRcv);
				__DEBUG_PRINT("%s\n", delimeter + 1);
				CTRLhost_OPERATOR::instance()->addClientIP(msgRcv, delimeter + 1);
			}
			CTRLhost_OPERATOR::instance()->updateConfig("clients.ini");

			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}

};
class LISTENERTHREAD:public client
{

public:
	LISTENERTHREAD(const SOCKET socket)
		:client(socket)
	{
		
	}
	void operator()()
	{
		
		char msgRcv[_MAX_DATA_SIZE];
		memset(msgRcv, 0, _MAX_DATA_SIZE);
		sockaddr client;
		int sizeRcv;
		while (isSocketOpen())
		{
			sizeRcv = -1;
			getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
			//if (GetLastError() == WSAEWOULDBLOCK)
			//	Sleep(10);
			if (sizeRcv != -1 && sizeRcv != sizeof(HOST_MSG))
			{
				__STD_PRINT("%s\n", msgRcv);
			}
			char* delimeter = strstr(msgRcv, "#");
			if (delimeter)
			{
				*delimeter = '\0';
				__DEBUG_PRINT("%s\n", msgRcv);
				__DEBUG_PRINT("%s\n", delimeter + 1);
				CTRLhost_OPERATOR::instance()->addClientIP(msgRcv, delimeter + 1);
			}
			CTRLhost_OPERATOR::instance()->updateConfig("clients.ini");

			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}
};
typedef std::vector<std::unique_ptr<hostLISTENER>> LISTENER_THREAD_POOL;

int main(int argc, char *argv[])
{
	std::cout
		<< "Usage: " << std::endl
		<< "control host(optional) port(optional)" << std::endl
		<< "\nTo specify the path and the arguments list of the programs,modify the 'control.ini' in the directory of the hosting program." << std::endl;
	std::unique_ptr<client> rc(new client());
	int defaultPort = 20715;
	//cmd:control
	if (argc < 2)
		rc->initForPort(defaultPort, NULL);
	//cmd:control host
	else if (argc < 3)
		rc->initForPort(defaultPort, argv[1]);
	//cmd:control host port
	else if (argc < 4)
	{
		defaultPort = atoi(argv[2]);
		rc->initForPort(defaultPort, argv[1]);
	}
	else
		return 0;

	CTRLhost_OPERATOR::instance()->loadConfig("clients.ini");
	_MSG* msg = new _MSG;
	msg->_operation = _OPEN;

	_MSG* pre = new _MSG;

	//msg->_prog = _MANHATTAN;
	char buf[_MSG_BUF_SIZE];
	int cnt = 0;
	int op = 0;
	WORD et = 0;
	std::cout << "Now input controlling instructions." << std::endl;
	std::cout << "e.g: type 'osgSync 1(open_close_flag) 1(elapse_time_val)' to open osgSync.exe with time synchronized oand 'osgSync 0 1' to close." << std::endl;
#ifdef _MULTI_THREAD_SAME_PORT_LISTEN
	LISTENER_THREAD_POOL vecListenerClientPool;
	for (int i = 0; i < 36; i++)
	{
		vecListenerClientPool.push_back(std::unique_ptr<hostLISTENER>(new hostLISTENER(rc->getSocket())));
	}
#endif
	while (++cnt < 10000000)
	{
		writeArgs(msg, buf);
		
		scanf("%s %d %d", msg->_filename, &op, &et);
		if (op == 0)
			msg->_operation = _CLOSE;
		else if (op == 1)
			msg->_operation = _OPEN;
		else if (op == 2)
			msg->_operation = _PLAY_PAUSE;
		else
			break;
		if (et < 0)
		{
			std::cout << "Error:  The elapse time in which the slave programs are to be invoked should be no less than 0." << std::endl;
			continue;
		}
		msg->_elapseTime = et+1;
		SYSTEMTIME systime;
		FILETIME  filetime;
		GetLocalTime(&systime);

		//_STD_PRINT_TIME(systime);

		//Invoke the slaves in 5 seconds
		systime.wSecond += et+1;
		if (systime.wSecond > 59)
		{
			systime.wSecond = 0;
			systime.wMinute += 1;
			if (systime.wMinute > 59)
			{
				systime.wMinute = 0;
				systime.wHour += 1;
				if (systime.wHour > 23)
				{
					systime.wHour = 0;
					systime.wDay += 1;
				}
			}
		}

		//__STD_PRINT("%s ", "Expected Exection Time:");
		//_STD_PRINT_TIME(systime);

		SystemTimeToFileTime(&systime, &filetime);
		msg->_time = filetime;
#if 0
		if ((*pre) == (*msg))
				continue;
#endif
		memcpy(pre, msg, sizeof(msg));
		rc->sendPacket((char*)msg, sizeof(_MSG));
		if (cnt > 1)
			continue;

#ifdef _MULTI_THREAD_SAME_PORT_LISTEN
		for (LISTENER_THREAD_POOL::iterator iter = vecListenerClientPool.begin(); iter != vecListenerClientPool.end(); iter++)
		{
			(*iter)->Init();
			(*iter)->start();
		}
#else
		static std::unique_ptr<host_LISTENER> listener(new host_LISTENER(rc.get()));
		listener->Init();
		listener->start();
#endif
	}


	exit(0);
}

