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
#include "player.h"
#include <stdlib.h>

void setUpViewer(osgViewer::View& viewer, const std::string keystoneFile,int width,int height)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;

	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (gc.valid())
	{
		gc->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);
	viewer.getCamera()->setGraphicsContext(gc.get());

	osg::DisplaySettings* ds = osg::DisplaySettings::instance();

	osg::ref_ptr<osgViewer::Keystone> keystone = osgDB::readFile<osgViewer::Keystone>(keystoneFile);
	ds->setKeystoneHint(true);
	bool keystonesLoaded = true;
	ds->getKeystones().push_back(keystone.get());
	viewer.assignStereoOrKeystoneToCamera(viewer.getCamera(), ds);

	double fovy, aspectRatio, zNear, zFar;
	viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
	double newAspectRatio = double(traits->width) / double(traits->height);
	double aspectRatioChange = newAspectRatio / aspectRatio;

	if (aspectRatioChange != 1.0)
	{

		viewer.getCamera()->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
	}
	float camera_fov = -1.0f;

	if (camera_fov>0.0f)
	{
		double fovy, aspectRatio, zNear, zFar;
		viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

		double original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;

		fovy = atan(tan(osg::DegreesToRadians(camera_fov)*0.5) / aspectRatio)*2.0;
		viewer.getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

		viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
		original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
	}


}
int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	std::string file;
	int width = 1280, height = 800;
	char **vlc_args = (char **)malloc((argc)* sizeof(char *));
	int realArgc = 0;
	ULONGLONG targetStartTime = 0;
	std::string keyStoneCorrFile("D:\cow.osgt");
	if (argc < 1)
	{
		file = std::string("Data\1.avi");
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


	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setTextureSize(width, height);
	texture->setImage(RCPLAYER::instance());
	texture->setResizeNonPowerOfTwoHint(false);
	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
		osg::Vec3(), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), 0, 1, 1, 0);

	quad->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
	mt->addChild(geode.get());

	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearMask(0);
	camera->setCullingActive(false);
	camera->setAllowEventFocus(false);
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
	camera->addChild(geode.get());

	//camera->setDisplaySettings(ds);

	osg::StateSet* ss = camera->getOrCreateStateSet();
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0));

	osgViewer::Viewer viewer;


	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(camera.get());

	//setUpViewer(viewer,keyStoneCorrFile,width,height);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.setSceneData(root.get());
	
	return viewer.run();
}
