// rcrenderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rcrenderer.h"
#include "rcSyncImp.h"
#include "rcapp.h"
#include "rcModelScene.h"
#include "rcTiledMapScene.h"
#include <osgDB/readFile>
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
	arguments.getApplicationUsage()->addCommandLineOption("-r <n>", "A coefficient to determine the rotate_angle of the camera.");
	arguments.getApplicationUsage()->addCommandLineOption("-w <n>", "Width of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-h <n>", "Height of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-x <n>", "offset_x of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-y <n>", "offset_y of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-memShare <n>", "offset_y of the view");
	arguments.getApplicationUsage()->addCommandLineOption("-keyStone <n>", "KeyStone FileName");
	TYPE viewMode;
	int width=1280, height=800, offset_x=0, offset_y=0;
	int cam_no, para_c, rotate_angle;
	int isMemShare=false;
	int isEarth=false;
	std::string keyStoneFilename;
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

	if (arguments.find("-r")==-1) {
		std::cout << "The parameter for rotate_angle of the camera is missing." << std::endl;
		return 1;
	}
	while (arguments.read("-r", rotate_angle)){}

	if (arguments.find("-keyStone")==-1) {
		std::cout << "KeyStone Filename is missing." << std::endl;
		return 1;
	}
	while (arguments.read("-keyStone", keyStoneFilename)){}

	if (arguments.find("-m")!=-1) {
		viewMode = _MASTER;
	}
	if (arguments.find("-s")!=-1) {
		viewMode = _SLAVE;
	}
	if (arguments.find("-memShare") != -1)
		isMemShare = true;
	while (arguments.read("-memShare")){}
	if (arguments.find("-earth") != -1)
		isEarth= true;
	while (arguments.read("-earth")){}
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

	if (arguments.argc() <= 1){
		arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
		return 1;
	}


	std::shared_ptr<rcrenderer> viewer;
#if 0 
	osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFiles(arguments);
	switch (viewMode)
	{
	case _MASTER:
		if (isMemShare)
			viewer=std::shared_ptr<rcrenderer>(new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncMasterModule(_RC_OSG_HOST_MEMSHARE_MASTER_NAME),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		else
			viewer.reset(new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncMasterModule(_RC_OSG_HOST_SYNC_BROCAST_PORT),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		break;
	case _SLAVE:
		if (isMemShare)
			viewer.reset(new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncSlaveModule(_RC_OSG_HOST_MEMSHARE_SLAVE_NAME),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		else
			viewer.reset(new rcrenderer(rootnode.get(), rcSyncImpFactory::instance()->createRcSyncSlaveModule(_RC_OSG_HOST_SYNC_BROCAST_PORT),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		break;
	default:
		std::cout << "exit without mode specified." << std::endl;
		std::cout << width << std::endl;
		return 1;
	}
	viewer->setup();
	viewer->getViewer()->setCameraManipulator(new osgGA::TerrainManipulator());
#else
	switch (viewMode)
	{
	case _MASTER:
		if (isMemShare)
			viewer.reset(new rcrenderer(rcSyncImpFactory::instance()->createRcSyncMasterModule(_RC_OSG_HOST_MEMSHARE_MASTER_NAME), width, height, const_cast<char*>(keyStoneFilename.c_str())));
		else
			viewer.reset(new rcrenderer(rcSyncImpFactory::instance()->createRcSyncMasterModule(_RC_OSG_HOST_SYNC_BROCAST_PORT), width, height, const_cast<char*>(keyStoneFilename.c_str())));
		break;
	case _SLAVE:
		if (isMemShare)
			viewer.reset(new rcrenderer(rcSyncImpFactory::instance()->createRcSyncSlaveModule(_RC_OSG_HOST_MEMSHARE_SLAVE_NAME),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		else
			viewer.reset(new rcrenderer(rcSyncImpFactory::instance()->createRcSyncSlaveModule(_RC_OSG_HOST_SYNC_BROCAST_PORT),width, height, const_cast<char*>(keyStoneFilename.c_str())));
		break;
	default:
		std::cout << "exit without mode specified." << std::endl;
		std::cout << width << std::endl;
		return 1;
	}
	rcScene* model;
	if (isEarth)
		model = new rcTiledMapScene(viewer.get(),arguments);
	else
		model = new rcModelScene(viewer.get(),arguments);
	viewer->setup();
	model->active();
#endif
	ParallaxPara& p = viewer->getParaParameter();
	p._cam_no = cam_no;
	p._para_c = para_c;
	p._rotate_angle =rotate_angle;
	viewer->run();
	return 0;
}

