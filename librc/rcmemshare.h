#pragma once
#include "rcthread.h"
#include "rc_common.h"
class rcfactory;
class rcmemshare
{
public:
	rcmemshare();
	~rcmemshare();
};

class rcFileMap
{
protected:
	rcFileMap();
	virtual ~rcFileMap();
	virtual HANDLE CreateFileMap(char* filename)=0;
	virtual void close()=0;

	virtual void rcMapViewofFile(HANDLE hMapFile);
	BYTE*	getViewOfFile();
	DWORD getMaxSizeHigh();
	DWORD getMaxSizeLow();
	void setMaxSizeHigh(DWORD szHigh);
	void setMaxSizeLow(DWORD szLow);
	DWORD m_dwMaximumSizeHigh;
	DWORD m_dwMaximumSizeLow;
	std::string m_lpName;
	BYTE* m_pViewOfFile;
};
class rcFileMapReader:public rcFileMap
{
public:
	~rcFileMapReader();
	virtual void close();
	void read(void* dst, DWORD sizeToRead);
	void lockedRead(void* dst, DWORD sizeToRead);
protected:
	rcFileMapReader(char* filename );
	virtual HANDLE CreateFileMap(char* filename);
private:
	std::unique_ptr<rcmutex_ext> m_mutex;
	HANDLE m_hMapFileObject;
	friend rcfactory;
};

class rcFileMapWriter:public rcFileMap
{
public:
	~rcFileMapWriter();
	virtual void close();
	void write(void* src, DWORD sizeToWrite);
	void lockedWrite(void* dst, DWORD sizeToRead);
protected:
	rcFileMapWriter(char* filename);
	virtual HANDLE CreateFileMap(char* filename);
private:
	std::unique_ptr<rcmutex_ext> m_mutex;
	HANDLE m_hMapFileObject;
	friend rcfactory;
};

