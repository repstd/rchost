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
class TCPServerThread :public THREAD
{
public:
	TCPServerThread() :THREAD() { }
	void run()
	{
		tcpServer* svr = rcfactory::instance()->createTcpServer(8888);
		char msg[30];
		int maxSize = 30;
		int size;
		SOCKADDR from;
		while (svr->isSocketOpen())
		{
			std::cout << "accepting..." << std::endl;
			svr->getPacket(from, msg, size,maxSize);
			std::cout << from.sa_data<< std::endl;
		}
	}
};
class TCPClientThread :public THREAD
{
public:
	TCPClientThread() :THREAD() { }
	void run()
	{
		tcpClient* cli = rcfactory::instance()->createTcpClient(8888, "127.0.0.1");
		std::cout << "sending..." << std::endl;
		char* msg = "From CTH.";
		int maxSize = 30;
		while (1)
		{
			std::cout << "sending..." << std::endl;
			cli->sendPacket(msg, strlen(msg)+1);
		}
	}
};
void testTCP()
{
	TCPServerThread svrth;
	TCPClientThread clith;
	//svrth.start();
	clith.start();
	clith.join();
	//svrth.join();
}
void testMemShare()
{
	//rcFileMapWriter* writer = rcfactory::instance()->createFileMapWriter("Memshare");
	char* msg = "test";
	//writer->write(msg, strlen(msg));
	//Sleep(30);
	char buf[512];
	double* p;
	rcFileMapReader* reader = rcfactory::instance()->createFileMapReader("MemShare");
	while (1)
	{
		reader->read(buf, 16 * sizeof(double));
		p = reinterpret_cast<double*>(buf);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
				std::cout << *((p+ 4* i + j)) << " ";
			std::cout << std::endl;
		}
		std::cout << "Frame Matrix Obtained."<< std::endl;
	}
}
void testOsgSyncMsg()
{
	rcOsgHostClient* cli = rcfactory::instance()->createOsgHostClient("MemShare", 6011);
	cli->start();
	cli->join();
}
#define MODE 1
int _tmain(int argc, _TCHAR* argv[])
{
	//testOsgSyncMsg();
	osg::Matrixd mat;
	mat.makeTranslate(10, 0, 0);
	osg::Matrix rotChange= osg::Matrix::rotate(osg::DegreesToRadians(-10.0), 0.0f, 0.0f, 1.0f);
	mat.makeRotate(osg::DegreesToRadians(10.0), 0.0f, 0.0f, 1.0f);

	//mat.makeRotate(osg::DegreesToRadians(-10.0), 0.0f, 0.0f, 1.0f);
	mat = rotChange*mat;

	osg::Vec3d trans, scale;
	osg::Quat rot, so;
	mat.decompose(trans, rot, scale, so);
	trans;
	mat.makeTranslate(20, 0, 0);
	return 0;
}
