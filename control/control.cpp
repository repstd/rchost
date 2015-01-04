// control.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "client.h"
#include "rc_common.h"
#include <stdio.h>
#include <memory>
#include <iostream>
int main(int argc, char *argv[])
{
	std::cout
		<< "Usage: " << std::endl
		<< "control host(optional) port(optional)" << std::endl
		<< "\nTo specify the path and the arguments list of the programs,modify the 'control.ini' in the directory of the hosting program." << std::endl;
	std::auto_ptr<client> rc(new client());
	//cmd:control
	if (argc < 2)
		rc->initForPort(20715, NULL);
	//cmd:control host
	else if (argc < 3)
		rc->initForPort(20715, argv[1]);
	//cmd:control host port
	else if (argc < 4)
		rc->initForPort(atoi(argv[2]), argv[1]);
	else
		return 0;
	std::auto_ptr<_MSG> msg(new _MSG());
	msg->_operation = _OPEN;
	//msg->_prog = _MANHATTAN;
	char buf[_MSG_BUF_SIZE];
	int cnt = 0;
	int op = 0;
	WORD st= 0;
	std::cout << "Now input controlling instructions." << std::endl;
	std::cout << "e.g: type 'osgSync 1(open_close_flag) 1(sync_time_val)' to open osgSync.exe with time synchronized oand 'osgSync 0 1' to close." << std::endl;
	while (++cnt<50)
	{
				
		writeArgs(msg.get(), buf);
		scanf("%s %d %d", msg->_filename,&op,&st);
		if (op==0)
			msg->_operation = _CLOSE;
		else if (op==1)
			msg->_operation = _OPEN;
		else
				break;
		if (st < 0)
		{
			std::cout << "Error:  The time value in which the slave programs are to be invoked should be no less than 0." << std::endl;
			continue;
		}
		msg->_timeout = st;
		SYSTEMTIME systime;
		FILETIME  filetime;
		GetLocalTime(&systime);

		//_STD_PRINT_TIME(systime);

		//Invoke the slaves in 5 seconds
		systime.wSecond += st;
		if (systime.wSecond > 59)
		{
			systime.wSecond = 0;
			systime.wMinute += 1;
			if (systime.wMinute>59)
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

		__STD_PRINT("%s ", "Expected Exection Time:");
		_STD_PRINT_TIME(systime);

		SystemTimeToFileTime(&systime, &filetime);
		msg->_time = filetime;

		rc->sendPacket((char*)msg.get(), sizeof(_MSG ) );
#if 0
		char msgRcv[_MSG_BUF_SIZE];
		sockaddr from;
		int size = -1;
		rc->getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
		std::cout << msgRcv << std::endl;
		clear(msg.get());
#endif	
	}
	return 0;
}

