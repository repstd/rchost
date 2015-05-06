#pragma once
#include "rcapp.h"
#include <osg/Node>
class rcTiledMapScene:public rcScene
{
public:
	rcTiledMapScene(rcApp* app,osg::ArgumentParser& arg);
	~rcTiledMapScene();
	virtual void active();
	virtual void setupScene();
	virtual rcApp* getApp();
	virtual osg::Node* getSceneNode();
private:
	std::unique_ptr<rcApp> m_renderer;
	osg::ref_ptr<osg::Node> m_model;
	osg::ArgumentParser& m_arg;
};

