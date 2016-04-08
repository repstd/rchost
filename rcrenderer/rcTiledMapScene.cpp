#include "StdAfx.h"
#include "rcTiledMapScene.h"
#include <osgEarth/MapNode>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/ExampleResources>
#include <osgEarthUtil/Controls>
#include <osgEarthSymbology/Color>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgGA/GUIEventHandler>
#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/MatrixTransform>
#include "BingOptions"
using namespace osgEarth;
using namespace osgEarth::Drivers;
using namespace osgEarth::Util;
class BingTileSource;
class BingTileSourDriver;
rcTiledMapScene::rcTiledMapScene(rcApp* app, osg::ArgumentParser& arg) 
:m_arg(arg)
{
	m_renderer.reset(app);
}

rcTiledMapScene::~rcTiledMapScene() 
{
	m_renderer.reset();
}
void rcTiledMapScene::active()
{
	rcApp* render = getApp();
	setupScene();

}
void rcTiledMapScene::setupScene()
{
	rcApp* render = getApp();
	//osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(render->getRootNode());
	osg::ref_ptr<osg::MatrixTransform> group = static_cast<osg::MatrixTransform*>(render->getRootNode());
	osgViewer::Viewer* viewer = render->getViewer();
	Map* map = new Map();
#if 1 
	// Add an imagery layer (blue marble from a TMS source)
	{
		TMSOptions tms;
		tms.url() = "http://readymap.org/readymap/tiles/1.0.0/7/";
		ImageLayer* layer = new ImageLayer("Imagery", tms);
		map->addImageLayer(layer);
		TMSOptions* elevation = new TMSOptions();
		elevation->url() = "http://readymap.org/readymap/tiles/1.0.0/9/";
		//elevation->url() = TMS_PATH;
		map->addElevationLayer(new ElevationLayer("SRTM", *(reinterpret_cast<TileSourceOptions*>(elevation))));
	}
	// Create a MapNode to render this map:
	m_model= new MapNode(map);
#else
	viewer->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(false, false);
	// install our default manipulator (do this before calling load)
	viewer->setCameraManipulator(new EarthManipulator());
	// load an earth file, and support all or our example command-line options
	// and earth file <external> tags    
	m_model= osgEarth::Util::MapNodeHelper().load(m_arg, viewer);
#endif
	osg::MatrixTransform* mt= new osg::MatrixTransform;
	mt->setName("rcTileMapScene");
	mt->addChild(m_model.get());
	if (group)
		group->addChild(mt);
	viewer->setCameraManipulator(new osgEarth::Util::EarthManipulator);
	//viewer->setCameraManipulator(new osgGA::TerrainManipulator());
	viewer->addEventHandler(new osgViewer::StatsHandler());
	viewer->addEventHandler(new osgViewer::WindowSizeHandler());
	viewer->addEventHandler(new osgViewer::ThreadingHandler());
	viewer->addEventHandler(new osgViewer::LODScaleHandler());
	//viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
}
rcApp* rcTiledMapScene::getApp() 
{
	return m_renderer.get();
}
osg::Node* rcTiledMapScene::getSceneNode()
{
	return m_model.get();
}
