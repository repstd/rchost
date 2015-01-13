#include "stdafx.h"
#include "rcthread.h"


rcmutex::rcmutex()
{
	
}

rcmutex::~rcmutex()
{

}
void rcmutex::initMutex(MUTEX* mutex) const
{
	m_mutex = std::unique_ptr<MUTEX>(mutex);
}
const MUTEX* rcmutex::getMutex() const
{
	return m_mutex.get();
}
void rcmutex::lock() const
{
	if (m_mutex.get() == NULL)
		initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	m_mutex->lock();
}
void rcmutex::unlock() const
{
	m_mutex->unlock();
}