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

class HOST_LISTENER : public THREAD, rcmutex
{
public:
	HOST_LISTENER::HOST_LISTENER(const client* cl)
		:THREAD(), rcmutex()
	{
		m_client = std::auto_ptr<client>(const_cast<client*>(cl));
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
				CTRLHOST_OPERATOR::instance()->addClientIP(msgRcv, delimeter + 1);
			}
			CTRLHOST_OPERATOR::instance()->updateConfig("clients.ini");
#endif
			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}

	std::auto_ptr<client> m_client;
};

class HOSTLISTENER : public THREAD, rcmutex, client
{
public:
	HOSTLISTENER::HOSTLISTENER(const SOCKET socket)
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
			lock();
			__STD_PRINT("%d ", getThreadId());
			unlock();
			if (sizeRcv != -1 && sizeRcv != sizeof(HOST_MSG))
			{
			lock();
				__STD_PRINT("%s\n", msgRcv);
			unlock();
			}
			char* delimeter = strstr(msgRcv, "#");
			if (delimeter)
			{
				*delimeter = '\0';
				__DEBUG_PRINT("%s\n", msgRcv);
				__DEBUG_PRINT("%s\n", delimeter + 1);
				CTRLHOST_OPERATOR::instance()->addClientIP(msgRcv, delimeter + 1);
			}
			CTRLHOST_OPERATOR::instance()->updateConfig("clients.ini");

			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}

};
typedef std::vector<std::auto_ptr<HOSTLISTENER>> LISTENER_THREAD_POOL;
int main(int argc, char *argv[])
{
	std::cout
		<< "Usage: " << std::endl
		<< "control host(optional) port(optional)" << std::endl
		<< "\nTo specify the path and the arguments list of the programs,modify the 'control.ini' in the directory of the hosting program." << std::endl;
	std::auto_ptr<client> rc(new client());
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

	CTRLHOST_OPERATOR::instance()->loadConfig("clients.ini");
	std::auto_ptr<_MSG> msg(new _MSG());
	msg->_operation = _OPEN;
	//msg->_prog = _MANHATTAN;
	char buf[_MSG_BUF_SIZE];
	int cnt = 0;
	int op = 0;
	WORD et = 0;
	std::cout << "Now input controlling instructions." << std::endl;
	std::cout << "e.g: type 'osgSync 1(open_close_flag) 1(elapse_time_val)' to open osgSync.exe with time synchronized oand 'osgSync 0 1' to close." << std::endl;
#ifdef _MULTI_THREAD_SAME_PORT_LISTEN
	LISTENER_THREAD_POOL vecListenerClientPool;
	for (int i = 0; i < 24; i++)
	{
		vecListenerClientPool.push_back(std::auto_ptr<HOSTLISTENER>(new HOSTLISTENER(rc->getSocket())));

	}
#endif
	while (++cnt < 50)
	{
		writeArgs(msg.get(), buf);
		scanf("%s %d %d", msg->_filename, &op, &et);
		if (op == 0)
			msg->_operation = _CLOSE;
		else if (op == 1)
			msg->_operation = _OPEN;
		else
			break;
		if (et < 0)
		{
			std::cout << "Error:  The elapse time in which the slave programs are to be invoked should be no less than 0." << std::endl;
			continue;
		}
		msg->_elapseTime = et;
		SYSTEMTIME systime;
		FILETIME  filetime;
		GetLocalTime(&systime);

		//_STD_PRINT_TIME(systime);

		//Invoke the slaves in 5 seconds
		systime.wSecond += et;
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

		rc->sendPacket((char*)msg.get(), sizeof(_MSG));
		if (cnt > 1)
			continue;

#ifdef _MULTI_THREAD_SAME_PORT_LISTEN
		for (LISTENER_THREAD_POOL::iterator iter = vecListenerClientPool.begin(); iter != vecListenerClientPool.end(); iter++)
		{
			iter->get()->Init();
			iter->get()->start();
		}
#else
<<<<<<< HEAD
=======

>>>>>>> 063bbd53ac8cbd47b10d870fa5aa8fb89a91ad95
		static std::auto_ptr<HOST_LISTENER> listener(new HOST_LISTENER(rc.get()));
		listener->Init();
		listener->start();
#endif
	}

	//for (LISTENER_THREAD_POOL::iterator iter = vecListenerClientPool.begin(); iter != vecListenerClientPool.end();iter++)
	//{
	//	iter->get()->join();
	//}
	return 0;
}

