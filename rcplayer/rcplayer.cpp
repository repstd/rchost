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
osg::Node* createVideoQuad(const osg::Vec3& corner, const std::string& file)
{
	osg::ref_ptr<RCPLAYER> imageStream = new RCPLAYER;
	if (imageStream)
#ifdef _PIPE_SYNC
		imageStream->open(file, false,1920, 1080);
		imageStream->start();
#else
		imageStream->open(file, true,1920, 1080);
#endif


	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	osg::Image* img = dynamic_cast<osg::ImageStream*>(imageStream.get());
	texture->setImage(img);
	texture->setTextureSize(1920, 1080);
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
	osg::ref_ptr<osg::Node> video = createVideoQuad(osg::Vec3(), file);
	osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
	scene->addChild(video.get());

	scene->setMatrix(osg::Matrix::ortho(0, 1920, 0, 1080, 0, 100));
	//osg::ref_ptr<osg::Group> root = new osg::Group;
	//root->addChild(video.get());

	osgViewer::Viewer viewer;
	//viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.setSceneData(video.get());

	return viewer.run();
}