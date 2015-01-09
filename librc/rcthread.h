#pragma once
#include <OpenThreads\Thread>
#include <OpenThreads\Mutex>
#include <memory>
typedef OpenThreads::Mutex MUTEX;
typedef OpenThreads::Thread THREAD;
class rcmutex
{
public:

	rcmutex();
	~rcmutex();
	const MUTEX* getMutex() const;
protected:
	virtual void initMutex(MUTEX* mutex) const;
	void lock() const;
	void unlock() const;
private:
	mutable std::auto_ptr<MUTEX> m_mutex;
};

