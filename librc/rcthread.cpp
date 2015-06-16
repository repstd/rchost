#include "stdafx.h"
#include "rcthread.h"


rcmutex_ext::rcmutex_ext()
{
	
}
rcmutex_ext::~rcmutex_ext()
{

}
void rcmutex_ext::initMutex(MUTEX* mutex) const
{
	m_mutex = std::unique_ptr<MUTEX>(mutex);
}
MUTEX* rcmutex_ext::getMutex() const
{
	return m_mutex.get();
}
void rcmutex_ext::lock() const
{
	if (m_mutex.get() == NULL)
		initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	m_mutex->lock();
}
void rcmutex_ext::unlock() const
{
	m_mutex->unlock();
}

rcmutex::rcmutex() :rcmutex_ext() {
	m_mutex = std::unique_ptr<rcmutex_ext>(new rcmutex_ext);
}
rcmutex::~rcmutex() { 
}

void rcmutex::initMutex(MUTEX* mutex) const {
	m_mutex->initMutex(mutex); 
}

const MUTEX* rcmutex::getMutex() const {
	return m_mutex->getMutex();
}
void rcmutex::lock() const
{
	m_mutex->lock();
}
void rcmutex::unlock() const
{
	m_mutex->unlock();
}
