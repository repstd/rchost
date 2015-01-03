// rchost.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "rc_common.h"
#include "host.h"
int main(int argc, char* argv[])
{
	std::cout
		<< "Usage: " << std::endl
		<< "host dWindowFlag port(optional) " << std::endl;
	if (argc < 2)
	{
		std::cout << "e.g:\n host 0 or host 0 6000" << std::endl;
		return 0;
	}
	int port= 20715;
	int quiet = atoi(argv[1]);
	if (quiet)
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	if (argc > 2)
			port= atoi(argv[2]);
	HOST_OPERATOR::instance()->loadPathMap("control.ini");
	std::auto_ptr<HOST> host(new HOST(port));

	host->run();

	return 0;
}

//int test_data()
//{
//	HOST_MSG  msg;
//	msg._prog = _MANHATTAN;
//	msg._operation = _OPEN;
//	char buf[30];
//	std::cout << sizeof(msg) << std::endl;
//	for (int i = 0; i <10 ; i++)
//	{
//		if (i%2)
//			sprintf(buf, "TEST MSG DATA STRUCTURE");
//		else
//			sprintf(buf, "fdfdfd");
//
//		writeArgs(&msg, buf);
//	}
//	char dataRcv[500];
//	memcpy(dataRcv, &msg, sizeof(msg));
//	HOST_MSG* pMsg = reinterpret_cast<HOST_MSG*>(dataRcv);
//	char read[30];
//	for (int i = 0; i < pMsg->_argc; i++)
//	{
//		readArgs(pMsg, i,read);
//		std::cout << read << std::endl;
//	}
//
//	std::cout << sizeof(msg) << std::endl;
//	return 0;
//}
