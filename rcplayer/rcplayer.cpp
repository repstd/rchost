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
int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	std::string file;
	int width = 1280, height = 800;
	char **vlc_args = (char **)malloc((argc)* sizeof(char *));
	int realArgc = 0;
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
			if (flag)
				continue;
			vlc_args[realArgc] = (char*)malloc(sizeof(char*));
			vlc_args[realArgc++] = argv[i];
		}
	}
	__STD_PRINT("width: %d ", width);
	__STD_PRINT("height: %d\n", height);

	RCPLAYER::instance()->initPlayer(vlc_args, realArgc);

	if (RCPLAYER::instance() != NULL)
#ifdef _PIPE_SYNC
		RCPLAYER::instance()->open(file, false, width, height);
	RCPLAYER::instance()->start();
#else
		RCPLAYER::instance()->open(file, true, width, height);
#endif

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;

	texture->setImage(RCPLAYER::instance());

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
		osg::Vec3(), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), 0, 1, 1, 0);
	quad->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearMask(0);
	camera->setCullingActive(false);
	camera->setAllowEventFocus(false);
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
	camera->addChild(geode.get());

	osg::StateSet* ss = camera->getOrCreateStateSet();
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0));

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(camera.get());

	osgViewer::Viewer viewer;
	viewer.setSceneData(root.get());
	return viewer.run();
}
