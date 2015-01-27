#include "StdAfx.h"
#include "rcviewer.h"
#include "osgDB/readFile"
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#define _PLAYER_LOG_FILENAME "./rcviewer.log"
#define _PLAYER_LOG_INIT __LOG_INIT(_PLAYER_LOG_FILENAME)
#define _PLAYER_LOG(fmt,data) __LOG_FORMAT(_PLAYER_LOG_FILENAME,fmt,data)
#define _PLAYER_LOG_TIME(time) __LOG__FORMAT_TIME(_PLAYER_LOG_FILENAME,time)

rcviewer::rcviewer(playerImp* imp) : osgViewer::Viewer()
{
	
	_PLAYER_LOG_INIT
	m_pPlayerImplementation = imp;

}

rcviewer::rcviewer(impFactory* factory, char* nameImp) : osgViewer::Viewer()
{
	if (strstr(nameImp, "VLC"))
		m_pPlayerImplementation = factory->createVLCImp();
}

rcviewer::~rcviewer()
{

}
void rcviewer::setupViewer(int width, int height, const char* keyStoneFilename)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->useCursor = false;
	m_tex = new osg::Texture2D;
	m_tex->setTextureSize(width, height);
	//m_tex->setInternalFormatMode(osg::Texture2D::USE_USER_DEFINED_FORMAT);
	/*
	* Bind the texture with the exact implenemtation
	*/
	m_pPlayerImplementation->bindTexSrc(m_tex);
	m_tex->setInternalFormat(GL_RGB);
	m_tex->setResizeNonPowerOfTwoHint(false);

	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();

	if (keyStoneFilename)
	{
		osg::ref_ptr<osgViewer::Keystone> keystone = osgDB::readFile<osgViewer::Keystone>(keyStoneFilename);
		osg::Camera* camera = assignKeystoneDistortionCamera(ds, gc, 0, 0, width, height, GL_COLOR, m_tex, keystone);
		setSceneData(camera);
	}
	else
	{

	}
	addEventHandler(new osgViewer::WindowSizeHandler);
	osg::setNotifyHandler(new errorHandler);
	sync_pipe_client = std::shared_ptr<namedpipeClient>(new namedpipeClient(_RC_PIPE_NAME));
	sync_server = std::shared_ptr<server>(new server(_RC_PIPE_BROADCAST_PORT));
	char host_name[MAX_PATH];
	gethostname(host_name, MAX_PATH);
	m_hostname = host_name;
}
playerImp* rcviewer::getImp()
{
	return m_pPlayerImplementation;
}
int rcviewer::handleOpenCV()
{
	playerImp* imp = getImp();
#ifdef _TIME_SYNC
	imp->syncStart();
#endif
	sockaddr from;
	char msgRcv[_MAX_DATA_SIZE];
	const char* msg = m_hostname.c_str();
	int size;
	SYSTEMTIME time;
	SYNC_MSG* syncMsgRcv;
	char timeStamp[90];
	ULONGLONG totalPlayed = 0;
	while (!done() && sync_server->isSocketOpen())
	{
		size = -1;
		sync_server->getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
		if (size == sizeof(SYNC_MSG))
		{
			advance();
			//frameStat();
			syncMsgRcv = reinterpret_cast<SYNC_MSG*>(msgRcv);
			imp->nextFrame();
			totalPlayed += 1;
			imp->syncFrame(totalPlayed, syncMsgRcv->_index);
			imp->updateTex();
			frame();
			imp->imageDirty();
			time = syncMsgRcv->_timeStamp;
			sync_server->sendPacket(from, const_cast<char*>(msg), strlen(msg), _MAX_DATA_SIZE);
		}
				
		else if (size == sizeof(SYSTEMTIME))
		{
			advance();
			frameStat();
			imp->nextFrame();
			totalPlayed += 1;
			imp->updateTex();
			frame();
			imp->imageDirty();
			sync_server->sendPacket(from, const_cast<char*>(msg), strlen(msg), _MAX_DATA_SIZE);
		}
	}
	return 1;
}

int rcviewer::handleVLC()
{
	vlcImp* image = dynamic_cast<vlcImp*>(getImp());

#ifdef _TIME_SYNC
	image->syncStart();
#endif

#ifdef _PIPE_SYNC
	while (!done()&&sync_pipe_client->receive())
#else
	sockaddr from;
	char msgRcv[_MAX_DATA_SIZE];
	int size = -1;
	const char* msg = m_hostname.c_str();

	srand((unsigned)time(NULL)); 

	float a = rand() % 3;
	Sleep(a * 1000);
	while (!done())
#endif
	{
#ifndef _PIPE_SYNC
		size = 0;
		if (size != -1)
		{
#endif
			image->updateTex();
			image->play();
			frame();
			image->pause();
			image->dirty();
			__STD_PRINT("%f\n", image->getPosition());
#ifndef _PIPE_SYNC
		}
#endif
	}

	return 1;

}
int rcviewer::run()
{
#ifdef _VLC_IMPLEMENTATION
	return handleVLC();
#else
	return handleOpenCV();
#endif
}
void rcviewer::frameStat()
{
		static DWORD frameCnt = 0;
		static DWORD stamp = 0;
		static float fps = 0.0;
		static DWORD last = GetTickCount();
		static DWORD elapse_time = 0;
		static DWORD average = 0;
		static int cnt = 0;
		stamp++;
		++frameCnt;
		DWORD now = GetTickCount();
		elapse_time = now - last;
		char buf[MAX_PATH];
		if (now - last >= 1000)
		{
			fps = frameCnt;
			cnt++;
			average += fps;
			sprintf(buf, "%d %0.f\n", cnt, fps);
			//OutputDebugStringA(buf);
			_PLAYER_LOG("%s", buf);
			frameCnt = 0;
			last = now;
		}
}
