// rcplayer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <osg/NODE>
#include <osgViewer/Viewer>
#include <osg/ImageStream>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgGA/FirstPersonManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <osg/TexMat>
#include "player.h"
#include "rcviewer.h"
#include <stdlib.h>

osg::Camera* assignKeystoneDistortionCamera(osgViewer::Viewer& viewer, osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, osg::Texture* texture, osgViewer::Keystone* keystone)
{
	double screenDistance = ds->getScreenDistance();
	double screenWidth = ds->getScreenWidth();
	double screenHeight = ds->getScreenHeight();
	double fovy = osg::RadiansToDegrees(2.0*atan2(screenHeight / 2.0, screenDistance));
	double aspectRatio = screenWidth / screenHeight;

	osg::Geode* geode = keystone->createKeystoneDistortionMesh();

	//// new we need to add the texture to the mesh, we do so by creating a
	//// StateSet to contain the Texture StateAttribute.
	osg::StateSet* stateset = geode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	osg::TexMat* texmat = new osg::TexMat;
	texmat->setScaleByTextureRectangleSize(true);
	stateset->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setGraphicsContext(gc);
	camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	camera->setViewport(new osg::Viewport(x, y, width, height));
	camera->setDrawBuffer(buffer);
	camera->setReadBuffer(buffer);
	camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
	//camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	camera->setViewMatrix(osg::Matrix::identity());
	camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, 0.1, 1000.0);

	// add subgraph to render
	camera->addChild(geode);

	camera->addChild(keystone->createGrid());

	camera->setName("DistortionCorrectionCamera");

	// camera->addEventCallback(new KeystoneHandler(keystone));

	viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
	return camera.release();
}

#if 0
int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	std::string file;
	int width = 1280, height = 800;
	char **vlc_args = (char **)malloc((argc)* sizeof(char *));
	int realArgc = 0;
	ULONGLONG targetStartTime = 0;

	__LOG_INIT(_PLAYER_LOG);

	std::string keyStoneCorrFile("D:\\cow.osgt");
	if (argc < 1)
	{
		file = std::string("Data\\1.avi");
		//return 0;
	}
	else
	{

		file = std::string(argv[0]);
		int flag;
		char *p = NULL;
		for (int i = 1; i < argc; i++)
		{
			flag = 0;
			p = strstr(argv[i], "--width");
			if (p != NULL)
			{
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					width = atoi(p + 1);
				}
				flag = 1;
			}

			p = strstr(argv[i], "--height");
			if (p != NULL)
			{
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					height = atoi(p + 1);
				}

				flag = 1;
			}
			p = strstr(argv[i], "--SettledTime");
			if (p != NULL)
			{
				flag = 1;
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					sscanf(p + 1, "%I64u", &targetStartTime);
				}
			}
			p = strstr(argv[i], "--keyStone");
			if (p != NULL)
			{
				flag = 1;
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					keyStoneCorrFile = std::string(p + 1);
				}

			}
			if (flag)
				continue;
			vlc_args[realArgc] = (char*)malloc(sizeof(char*));
			vlc_args[realArgc++] = argv[i];

		}
	}
	__STD_PRINT("width: %d ", width);
	__STD_PRINT("height: %d\n", height);
	RCPLAYER::instance()->initPlayer(vlc_args, realArgc);


	RCPLAYER::instance()->setTargetTime(targetStartTime);

	if (RCPLAYER::instance() != NULL)

#ifdef _PIPE_SYNC
	RCPLAYER::instance()->open(file, false, width, height);
	RCPLAYER::instance()->start();
#else
	RCPLAYER::instance()->open(file, true, width, height);
#endif
	osgViewer::Viewer viewer;
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;

	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	//if (gc.valid())
	//{
	//	gc->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f));
	//	gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//}
	
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();

	osg::ref_ptr<osgViewer::Keystone> keystone = osgDB::readFile<osgViewer::Keystone>(keyStoneCorrFile);
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setTextureSize(width, height);
	texture->setImage(RCPLAYER::instance());
	texture->setResizeNonPowerOfTwoHint(false);

	osg::Camera* camera=assignKeystoneDistortionCamera(viewer, ds, gc, 0, 0, width, height, GL_COLOR, texture, keystone);
	viewer.setSceneData(camera);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	osg::setNotifyHandler(new LogFileHandler("warn.txt"));

	int flag=1;
	while (!viewer.done())
	{

		RCPLAYER::instance()->updateTexture();

		viewer.frame();

		RCPLAYER::instance()->dirty();

		if (flag)
		{
			flag = 0;
			RCPLAYER::instance()->syncStart();
		}
	}
	__STD_PRINT("%s\n", "done");
	RCPLAYER::instance()->quit();
	viewer.setDone(true);
	return 1;
}
#endif

int main(int argc, char** argv)
{

	osg::ArgumentParser arguments(&argc, argv);
	std::string file;
	int width = 1280, height = 800;
	char **vlc_args = (char **)malloc((argc)* sizeof(char *));
	int realArgc = 0;
	ULONGLONG targetStartTime = 0;

	std::string keyStoneCorrFile("D:\\cow.osgt");
	if (argc < 1)
	{
		file = std::string("Data\\1.avi");
		//return 0;
	}
	else
	{
		file = std::string(argv[0]);
		int flag;
		char *p = NULL;
		for (int i = 1; i < argc; i++)
		{
			flag = 0;
			p = strstr(argv[i], "--width");
			if (p != NULL)
			{
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					width = atoi(p + 1);
				}
				flag = 1;
			}

			p = strstr(argv[i], "--height");
			if (p != NULL)
			{
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					height = atoi(p + 1);
				}

				flag = 1;
			}
			p = strstr(argv[i], "--SettledTime");
			if (p != NULL)
			{
				flag = 1;
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					sscanf(p + 1, "%I64u", &targetStartTime);
				}
			}
			p = strstr(argv[i], "--keyStone");
			if (p != NULL)
			{
				flag = 1;
				p = strstr(argv[i] + 2, "-");
				if (p != NULL)
				{
					keyStoneCorrFile = std::string(p + 1);
				}

			}
			if (flag)
				continue;
			vlc_args[realArgc] = (char*)malloc(sizeof(char*));
			vlc_args[realArgc++] = argv[i];

		}
	}
	__STD_PRINT("width: %d ", width);
	__STD_PRINT("height: %d\n", height);
	__STD_PRINT("filename: %s\n", file.c_str());
#ifdef _VLC_IMPLEMENTATION
	playerImp* imp = impFactory::instance()->createVLCImp();
	vlcImp* vlc = (vlcImp*)imp;
	vlc->initPlayer(vlc_args, realArgc);
	vlc->open(file, false, width, height);

#ifdef _TIME_SYNC
	vlc->setTargetTime(targetStartTime);
#endif

	rcviewer* viewer= new rcviewer(vlc);
#else
	std::unique_ptr<cvImp> opencvImp(dynamic_cast<cvImp*>(impFactory::instance()->createOpenCVImp()));
	opencvImp->open(file);

#ifdef _TIME_SYNC
	opencvImp->setTargetTime(targetStartTime);
#endif

	std::unique_ptr<rcviewer> viewer(new rcviewer(opencvImp.get()));
#endif
	//Here we use OpenCV Implementation
	viewer->setupViewer(width, height, keyStoneCorrFile.c_str());
	viewer->run();
	return 0;
}