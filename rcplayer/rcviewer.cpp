#include "StdAfx.h"
#include "rcviewer.h"
#include "osgDB/readFile"
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
rcviewer::rcviewer(playerImp* imp):
osgViewer::Viewer()
{
	m_pPlayerImplementation = imp;
}

rcviewer::rcviewer(impFactory* factory,char* nameImp):
osgViewer::Viewer()
{
	if (strstr(nameImp, "VLC"))
		m_pPlayerImplementation = factory->createRCVLCImp();
}

rcviewer::~rcviewer()
{

}
void rcviewer::setupViewer(int width,int height,const char* keyStoneFilename)
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
	m_tex= new osg::Texture2D;
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
		osg::Camera* camera=assignKeystoneDistortionCamera(ds, gc, 0, 0, width, height, GL_COLOR, m_tex, keystone);
		setSceneData(camera);
	}
	else
	{

	}
	/*
	*Add Event Handler
	*/	
	addEventHandler(new osgViewer::WindowSizeHandler);
	osg::setNotifyHandler(new errorHandler);
	client = std::shared_ptr<namedpipeClient>(new namedpipeClient(_RC_PIPE_NAME));
}
playerImp* rcviewer::getImp()
{
	return m_pPlayerImplementation;
}
int rcviewer::handleOpenCV()
{
	cvImp* imp = (cvImp*)getImp();
	imp->syncStart();
	while (!done()&&client->receive())
	{
		imp->nextFrame();
		imp->updateTex();
		frame();
		imp->imageDirty();
	}
	return 1;
}
int rcviewer::handleVLC()
{
	vlcImp* image = (vlcImp*)getImp();
	while (!done() && client->receive() )
	{
		//__STD_PRINT("%s\n", "next");
		//image->play();
		//image->pause();
		image->updateTex();
		image->play();
		frame();
		image->pause();	
		image->dirty();
		__STD_PRINT("%f\n", image->getPosition());
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