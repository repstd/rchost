#include "stdafx.h"
#include "osghost.h"
#include "host.h"

osghost::osghost(int port):
	HOST_API(port)
{
	Init();
}


osghost::~osghost()
{
}
DWORD osghost::handle(const HOST_MSG* msg)
{
	std::string filename = msg->_filename;
	bool isCurDirNeeded = 0;
	//handle all of the specialized settings here.
	if (filename.compare("cegui") == 0)
		isCurDirNeeded = 1;
	return handleProgram(filename, msg->_operation, isCurDirNeeded);
}
