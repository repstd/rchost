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

class MultiListener:public THREAD
{

public:
	MultiListener(const int port) :
		THREAD()
	{

	}
	virtual void run()
	{
		char msgRcv[_MAX_DATA_SIZE];
		int size = -1;
		std::string strMsg;
	}
	virtual int cancel()
	{

	}

	std::vector<client> m_vecListener;
	client* m_sender;
};
int _tmain(int argc, _TCHAR* argv[])
{
	//cv::VideoCapture video;
	//video.open("./img/0.avi");
	//if (!video.isOpened())
	//	return 0;
	//cv::Mat frame;
	//cv::Size size = cv::Size((int)video.get(CV_CAP_PROP_FRAME_WIDTH), (int)video.get(CV_CAP_PROP_FRAME_HEIGHT));
	//cv::namedWindow("video", CV_WINDOW_AUTOSIZE);
	//cvMoveWindow("video", size.width, size.height); //750, 2 (bernat =0)
	//while (1)
	//{
	//	video >> frame;
	//	cv::imshow("video", frame);
	//	cv::waitKey(0);
	//}
	pipeSignal();
	return 0;
}
