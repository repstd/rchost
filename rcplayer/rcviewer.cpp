#include "StdAfx.h"
#include "rcviewer.h"
#include "osgDB/readFile"
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#define _PLAYER_LOG_FILENAME "./opencv_player.log"
#define _PLAYER_LOG_INIT __LOG_INIT(_PLAYER_LOG_FILENAME)
#define _PLAYER_LOG(fmt,data) __LOG_FORMAT(_PLAYER_LOG_FILENAME,fmt,data)
#define _PLAYER_LOG_TIME(time) __LOG__FORMAT_TIME(_PLAYER_LOG_FILENAME,time)

rcviewer::rcviewer(playerImp* imp) : osgViewer::Viewer()
{
	m_pPlayerImplementation = imp;
	_PLAYER_LOG_INIT
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
#ifdef _PIPE_SYNC
	sync_pipe_client = std::shared_ptr<namedpipeClient>(new namedpipeClient(_RC_PIPE_NAME));
#else
	sync_server = std::shared_ptr<server>(new server(_RC_PIPE_BROADCAST_PORT));
	char host_name[MAX_PATH];
	gethostname(host_name, MAX_PATH);
	m_hostname = host_name;
#endif
}
playerImp* rcviewer::getImp()
{
	return m_pPlayerImplementation;
}
int rcviewer::handleOpenCV()
{
	cvImp* imp = dynamic_cast <cvImp*>(getImp());
#ifdef _TIME_SYNC
	imp->syncStart();
#endif

#ifdef _PIPE_SYNC
	while (!done()&&sync_pipe_client->receive())
#else
	sockaddr from;
	char msgRcv[_MAX_DATA_SIZE];
	const char* msg = m_hostname.c_str();
	int size;
	SYSTEMTIME time;
	SYNC_MSG* syncMsgRcv;
	char timeStamp[90];
	ULONGLONG totalPlayed = 0;
	while (!done() && sync_server->isSocketOpen())
#endif
	{
#ifndef _PIPE_SYNC
		size = -1;
		sync_server->getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
		if (size == sizeof(SYNC_MSG))
		{
#endif

			syncMsgRcv = reinterpret_cast<SYNC_MSG*>(msgRcv);
			imp->nextFrame();
			totalPlayed += 1;
			imp->syncFrame(totalPlayed, syncMsgRcv->_index);
			imp->updateTex();
			frame();
			imp->imageDirty();
			_PLAYER_LOG("%d ", imp->getFrameIndex());
			time = syncMsgRcv->_timeStamp;
			_STD_ENCODE_TIMESTAMP(timeStamp, time);
			_PLAYER_LOG("%s ", timeStamp);
			GetLocalTime(&time);
			_STD_ENCODE_TIMESTAMP(timeStamp, time);
			_PLAYER_LOG(" %s\n", timeStamp);
#ifndef _PIPE_SYNC
			sync_server->sendPacket(from, const_cast<char*>(msg), strlen(msg), _MAX_DATA_SIZE);
		}

#endif
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
	while (!done() && sync_server->isSocketOpen())
#endif
	{
#ifndef _PIPE_SYNC
		size = -1;
		sync_server->getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
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
			sync_server->sendPacket(from, const_cast<char*>(msg), strlen(msg), _MAX_DATA_SIZE);
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