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
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include "player.h"
osg::Node* createVideoQuad(const osg::Vec3& corner,int texWidth=1920,int texHeight=1080)
{


	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	osg::Image* img = dynamic_cast<osg::ImageStream*>(RCPLAYER::instance());
	texture->setImage(img);
	texture->setTextureSize(texWidth,texHeight);
	texture->setResizeNonPowerOfTwoHint(false);

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(corner, osg::Vec3(1920.0, 100.0f, 0.0f), osg::Vec3(0.0f, 100.0f, 1080.0f), 0.0f, 1.0f, 1.0f, 0.0f);

	quad->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());
	return geode.release();
}
osg::Camera* createHUDCamera(double left, double right, double bottom, double top, osg::Camera::RenderOrder order, int isClearBuffer = false)
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);

	if (isClearBuffer)
	{
		camera->setClearMask(GL_COLOR_BUFFER_BIT);
		camera->setClearColor(osg::Vec4(0, 0, 0, 0));
	}
	camera->setRenderOrder(order);
	camera->setAllowEventFocus(false);
	camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	return camera.release();
}
int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	std::string file;
	if (argc < 1)
		return 0;

	file = std::string(argv[0]);
	char **vlc_args= (char **)malloc((argc) * sizeof(char *));			
	int width=1280, height=800;
	char *p = NULL;
	int realArgc = 0;
	int flag;

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
				 height= atoi(p + 1);
			}

			flag = 1;
		}
		if (flag)
			continue;
		vlc_args[realArgc] = (char*)malloc(sizeof(char*));
		vlc_args[realArgc++] = argv[i];
	}
	__STD_PRINT("width: %d ", width);
	__STD_PRINT("height: %d\n", height);

	RCPLAYER::instance()->initPlayer(vlc_args,realArgc);

	if (RCPLAYER::instance()!=NULL)
#ifdef _PIPE_SYNC
		RCPLAYER::instance()->open(file, false,width, height);
		RCPLAYER::instance()->start();
#else
		RCPLAYER::instance()->open(file, true,width, height);
#endif

	osg::ref_ptr<osg::Node> video = createVideoQuad(osg::Vec3(),width,height);
	osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
	scene->addChild(video.get());

	scene->setMatrix(osg::Matrix::ortho(0, width, 0, height, 0, 100));
	//osg::ref_ptr<osg::Group> root = new osg::Group;
	//root->addChild(video.get());

	osgViewer::Viewer viewer;
	//viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.setSceneData(video.get());

	return viewer.run();
}