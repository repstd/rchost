// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rcpipe.h"
#include "server.h"
#include "client.h"
#include "rcfactory.h"
#include "../rcplayer/playerImp.h"
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
void pipeSignal()
{


	std::unique_ptr<client> nextFrameCtrl = std::unique_ptr<client>(rcfactory::instance()->createUdpClient(_RC_PIPE_BROADCAST_PORT, NULL));
	while (1)
	{
		char* msg = "pipe";
		if (!nextFrameCtrl->isSocketOpen())
			continue;
		if (getchar() != 'Q')
		{
			for (int i = 0; i < 100; i++)
				nextFrameCtrl->sendPacket(msg, strlen(msg));
		}

	}
}
class pipeServer:public THREAD
{
public:
	pipeServer() :THREAD(){}
	void run()
	{
		rcpipeServer* pipeSvr = new rcpipeServer(_RC_PIPE_NAME);
		char* msg = "pipe\n";
		char s[512];
		DWORD sizeWriten;
		HANDLE hEvent = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 
		if (hEvent == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
		}
		OVERLAPPED oOverlap;
		oOverlap.hEvent = hEvent;

		while (1)
		{
			if (1)
			{
				scanf("%s\n", s);
				if (pipeSvr->writeto(s, strlen(s), sizeWriten, &oOverlap))
					std::cout << "written: " <<sizeWriten<< std::endl;
			}
		}

	}
};
class pipeClient :public THREAD
{
public:
	pipeClient() : THREAD(){}
	void run()
	{
		rcpipeClient* cli = new rcpipeClient(_RC_PIPE_NAME);
		char msg[512];
		DWORD sizeRead;
		HANDLE hEvent = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 
		if (hEvent == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
		}
		OVERLAPPED oOverlap;
		oOverlap.hEvent = hEvent;
		int cnt = 1000;
		while (1)
		{
			if (cli->readfrom(msg, 512, sizeRead, &oOverlap)!=-1)
			{
				std::cout << sizeRead<< std::endl;
				//cli->closeHandle();
				//cli->createPipe(_RC_PIPE_NAME);
			}
		}
	}

};
#define MODE 1
int _tmain(int argc, _TCHAR* argv[])
{

	//pipeServer svr;
	pipeClient cli;
	cli.start();
	//Sleep(3000);
	//svr.start();
	cli.join();
	//svr.join();
	return 0;
}
