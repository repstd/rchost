#pragma once
#include <OpenThreads\Thread>
#include <OpenThreads\Mutex>
#include <memory>
typedef OpenThreads::Mutex MUTEX;
typedef OpenThreads::Thread THREAD;

/*
*@yulw,2015-4-4
*Used as one of the base for a class in which a mutex is needed.This is convinient,
*but is deprecated because of the confilict with the principals of Object-Oriented programming.
*Here the original version of rcmutex is replaced with a wrappered class rcmutex_ext.
*/
//class rcmutex
//{
//public:
//
//	rcmutex();
//	~rcmutex();
//	const MUTEX* getMutex() const;
//protected:
//	virtual void initMutex(MUTEX* mutex) const;
//	void lock() const;
//	void unlock() const;
//private:
//	mutable std::unique_ptr<MUTEX> m_mutex;
//};


/*@yulw,2015-4-4
*Compared with the original rcmutex, the abbreviation _ext means this class can be used
*in more circumstances and the use of whom is suggested.
*/
class rcmutex_ext
{
public:
	rcmutex_ext();
	~rcmutex_ext();
	void initMutex(MUTEX* mutex) const;
	void lock() const;
	void unlock() const;
	const MUTEX* getMutex() const;
private:
	mutable std::unique_ptr<MUTEX> m_mutex;

};

class rcmutex:public rcmutex_ext
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
	mutable std::unique_ptr<rcmutex_ext> m_mutex;
};

