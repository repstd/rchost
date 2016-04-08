#include "StdAfx.h"
#include "rcModelScene.h"
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
rcModelScene::rcModelScene(rcApp* app,osg::ArgumentParser& arg) 
{
	m_renderer.reset(app);
	m_model = osgDB::readNodeFiles(arg);
}


rcModelScene::~rcModelScene() {
}
void rcModelScene::active()
{
	rcApp* render = getApp();
	setupScene();
	osg::ref_ptr<osgViewer::Viewer> viewer=render->getViewer();
	viewer->setCameraManipulator(new osgGA::TerrainManipulator());
	// add the stats handler
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->addEventHandler(new  osgViewer::HelpHandler);
	// add the state manipulator
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
}
void rcModelScene::setupScene()
{
	rcApp* render = getApp();
	osg::ref_ptr<osg::MatrixTransform> group = static_cast<osg::MatrixTransform*>(render->getRootNode());
	osg::MatrixTransform* mt = new osg::MatrixTransform;
	mt->setName("rcModelScene");
	mt->addChild(m_model.get());
	if (group)
		group->addChild(mt);
}
rcApp* rcModelScene::getApp() {
	return m_renderer.get();
}
osg::Node* rcModelScene::getSceneNode()
{
	return m_model.get();
}
