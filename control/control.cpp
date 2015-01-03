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
		<< "To specify the path of the programs,modify the control.ini in the directory." << std::endl;
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
	rc->sendPacket((char*)msg.get(), sizeof(_MSG));
	std::cout << "Now input controlling instrucions." << std::endl;
	std::cout << "e.g: type 'osgSync 0' to open osgSync.exe and 'osgSync 1' to close." << std::endl;
	while (++cnt<50)
	{
				
		writeArgs(msg.get(), buf);
		scanf("%s %d", msg->_filename,&op);
		if (op==0)
			msg->_operation = _CLOSE;
		else if (op==1)
			msg->_operation = _OPEN;
		else
				break;
		//msg->_operation = operation(op);
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

