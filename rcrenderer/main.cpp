// rcrenderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rcrenderer.h"
#include "rcSyncImp.h"
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
int main(int argc, char* argv[])
{
	osg::ArgumentParser arguments(&argc, argv);
	// set up the usage document, in case we need to print out how to use this program.
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates how to approach implementation of clustering.");
	arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options] filename ...");
	arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
	arguments.getApplicationUsage()->addCommandLineOption("-m", "Set viewer to MASTER mode");
	arguments.getApplicationUsage()->addCommandLineOption("-s", "Set viewer to SLAVE mode");
	arguments.getApplicationUsage()->addCommandLineOption("-f <float>", "Field of view of camera");
	arguments.getApplicationUsage()->addCommandLineOption("-k <n>", "Index of Camera. -k determins the parallax of this camera");
	arguments.getApplicationUsage()->addCommandLineOption("-c <n>", "A coefficient to adjust the parallax.");
	arguments.getApplicationUsage()->addCommandLineOption("-w <n>", "Width of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-h <n>", "Height of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-x <n>", "offset_x of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-y <n>", "offset_y of the view");
	TYPE viewMode;
	int width=1280, height=800, offset_x=0, offset_y=0;
	int cam_no, para_c;
	if (arguments.find("-k")==-1) {
		std::cout << "The camera index is missing.!" << std::endl;
		return 1;
	}
	while (arguments.read("-k", cam_no)){}

	if (arguments.find("-c")==-1) {
		std::cout << "The parameter for parallax adjustment is missing.!" << std::endl;
		return 1;
	}
	while (arguments.read("-c", para_c)){}

	while (arguments.read("-m")) 
		viewMode = _MASTER;
	while (arguments.read("-s")) 
		viewMode = _SLAVE;
	while (arguments.read("-w", width)){}
	while (arguments.read("-h", height)){}
	while (arguments.read("-x", offset_x)){}
	while (arguments.read("-y", offset_y)){}
	while (arguments.read("-y", offset_y)){}
	while (arguments.read("-y", offset_y)){}
	if (!arguments.find("-c")) {
		std::cout << "The camera index is missing.!" << std::endl;
		return 1;
	}
	// if user request help write it out to cout.
	if (arguments.read("-h") || arguments.read("--help"))
	{
		arguments.getApplicationUsage()->write(std::cout);
		return 1;
	}
	// any option left unread are converted into errors to write out later.
	arguments.reportRemainingOptionsAsUnrecognized();
	// report any errors if they have occured when parsing the program aguments.
	if (arguments.errors())
	{
		arguments.writeErrorMessages(std::cout);
		return 1;
	}

	if (arguments.argc() <= 1)
	{
		arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
		return 1;
	}

	// load model.
	osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFiles(arguments);
	osg::ref_ptr<rcrenderer> viewer;
	switch (viewMode)
	{
	case _MASTER:
		viewer = new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncMasterModule());
		break;
	case _SLAVE:
		viewer = new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncSlaveModule());
		break;
	default:
		return 1;
	}
	viewer->setupRenderer(width, height,NULL);
	viewer->setCameraManipulator(new osgGA::TerrainManipulator());
	// add the stats handler
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->addEventHandler(new  osgViewer::HelpHandler);
	// add the state manipulator
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	ParallaxPara& p = viewer->getParaParameter();
	p._cam_no = cam_no;
	p._para_c = para_c;
	return viewer->run();
}

