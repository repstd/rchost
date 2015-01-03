#pragma once
#include <OpenThreads\Thread>
#include <OpenThreads\Mutex>
class rcthread:
	public OpenThreads::Thread
{
public:

	rcthread();
	~rcthread();

private:

	virtual void initMutex(OpenThreads::Mutex* mutex);
	
};

