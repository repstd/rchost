// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rcpipe.h"
#include "server.h"
#include "client.h"
#include "../rcplayer/playerImp.h"
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
void pipeSignal()
{


	std::unique_ptr<client> nextFrameCtrl=std::unique_ptr<client>(new client(_RC_PIPE_BROADCAST_PORT,NULL) );
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


int _tmain(int argc, _TCHAR* argv[])
{
	pipeSignal();
	return 0;
}
