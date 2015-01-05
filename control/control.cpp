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
		sockaddr client;
		int sizeRcv;
		while (m_client->isSocketOpen())
		{
			sizeRcv = -1;
			m_client->getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
			if (sizeRcv != -1 && sizeRcv != sizeof(HOST_MSG))
				__STD_PRINT("%s\n", msgRcv);
			char* delimeter = strstr(msgRcv, "#");
			if (delimeter)
			{
				*delimeter = '\0';
				CTRLHOST_OPERATOR::instance()->addClientIP(msgRcv, delimeter +1 );
			}
			memset(msgRcv, 0, _MAX_DATA_SIZE);
		}

	}
	std::auto_ptr<client> m_client;
};

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
	CTRLHOST_OPERATOR::instance()->loadConfig("clients.ip");
	std::auto_ptr<_MSG> msg(new _MSG());
	msg->_operation = _OPEN;
	//msg->_prog = _MANHATTAN;
	char buf[_MSG_BUF_SIZE];
	int cnt = 0;
	int op = 0;
	WORD et = 0;
	std::cout << "Now input controlling instructions." << std::endl;
	std::cout << "e.g: type 'osgSync 1(open_close_flag) 1(elapse_time_val)' to open osgSync.exe with time synchronized oand 'osgSync 0 1' to close." << std::endl;
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
		static std::auto_ptr<HOST_LISTENER> listener(new HOST_LISTENER(rc.get()));
		listener->Init();
		listener->start();
	}
	return 0;
}

