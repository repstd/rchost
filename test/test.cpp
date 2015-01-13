// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rcpipe.h"
#include <memory>
int _tmain(int argc, _TCHAR* argv[])
{
	
	std::unique_ptr<namedpipeClient> client(new namedpipeClient(_RC_PIPE_NAME));

	while (1)
	{

		client->receive();

	}
	return 0;
}

