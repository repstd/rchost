#include "stdafx.h"
#include "rcmemshare.h"
//#pragma data_seg(".share")
static bool bReadyToRead = true;
//#pragma data_seg()
//#pragma comment(linker, "/section:.share,rws")
bool isReady() {
	return bReadyToRead;
}
void setStatus(bool status) {
	bReadyToRead = status;
}
rcmemshare::rcmemshare()
{
}


rcmemshare::~rcmemshare()
{
}
rcFileMap::rcFileMap() {
	m_dwMaximumSizeHigh = 0;
	m_dwMaximumSizeLow = 65536;
}
rcFileMap::~rcFileMap()
{
	if (m_pViewOfFile)
		UnmapViewOfFile(m_pViewOfFile);
}
BYTE* rcFileMap::getViewOfFile()
{
	return m_pViewOfFile;
}

DWORD rcFileMap::getMaxSizeHigh()
{
	return m_dwMaximumSizeHigh;
}

DWORD rcFileMap::getMaxSizeLow()
{
	return m_dwMaximumSizeLow;
}
void rcFileMap::setMaxSizeHigh(DWORD szHigh)
{
	m_dwMaximumSizeHigh = szHigh;
}
void rcFileMap::setMaxSizeLow(DWORD szLow)
{
	m_dwMaximumSizeLow = szLow;
}

void rcFileMap::rcMapViewofFile(HANDLE hMapFile)
{

	size_t sz;
	sz = getMaxSizeHigh();
	sz <<= 16;
	sz ^= getMaxSizeLow();
	m_pViewOfFile = (BYTE*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sz);
	if (m_pViewOfFile == NULL) {
		__STD_PRINT(TEXT("Could not map view of file (%d).\n"), GetLastError());
		CloseHandle(hMapFile);
		return;
	}
}

bool rcFileMap::isValid() {
	return m_pViewOfFile!=NULL ? true : false;
}
rcFileMapReader::~rcFileMapReader()
{
	if (m_hMapFileObject)
		close();
}
rcFileMapReader::rcFileMapReader(char* filename) :rcFileMap()
{
	m_mutex = std::unique_ptr<rcmutex_ext>(new rcmutex_ext);
	m_mutex->initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	m_hMapFileObject = CreateFileMap(filename);
	rcMapViewofFile(m_hMapFileObject);
}

HANDLE rcFileMapReader::CreateFileMap(char* filename)
{
	m_lpName = filename;
	HANDLE hMapFile = NULL;
	/*@yulw,2014-4-14.
	*Keep waiting until the FileMapping Server has started.
	*/
	while (!hMapFile) {
		hMapFile = OpenFileMappingA(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			filename);               // name of mapping object
	}
	if (!hMapFile) {
		__STD_PRINT("Error in Open Shared Memory.ErrorCode:%d\n", GetLastError());
		return NULL;
	}
	return hMapFile;
}

void rcFileMapReader::close()
{
	CloseHandle(m_hMapFileObject);
}

void rcFileMapReader::read(void* dst, DWORD sizeToRead)
{
	//if (isReady()&&dst&&getViewOfFile()) {
	//	setStatus(false);
	//	memcpy(dst, getViewOfFile(), sizeToRead);
	//	setStatus(true);
	//}
	memcpy(dst, getViewOfFile(), sizeToRead);
}

void rcFileMapReader::lockedRead(void* dst, DWORD sizeToRead)
{
	m_mutex->lock();
	read(dst, sizeToRead);
	m_mutex->unlock();
}
rcFileMapWriter::~rcFileMapWriter()
{
	if (m_hMapFileObject)
		close();
}

rcFileMapWriter::rcFileMapWriter(char* filename) :rcFileMap()
{
	m_mutex = std::unique_ptr<rcmutex_ext>(new rcmutex_ext());
	m_mutex->initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	m_hMapFileObject = CreateFileMap(filename);
	rcMapViewofFile(m_hMapFileObject);
	//memset(this->getViewOfFile(), 0, this->getMaxSizeLow());
}
void rcFileMapWriter::close()
{
	CloseHandle(m_hMapFileObject);
}
HANDLE rcFileMapWriter::CreateFileMap(char* filename)
{
	HANDLE hMapFile;
	hMapFile = CreateFileMappingA(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		getMaxSizeHigh(),                       // maximum object size (high-order DWORD)
		getMaxSizeLow(),         // maximum object size (low-order DWORD)
		filename);               // name of mapping object
	if (!hMapFile) {
		__STD_PRINT("Error in Open Shared rcMemory.ErrorCode:%d\n", GetLastError());
		return NULL;
	}
	return hMapFile;
}

void rcFileMapWriter::write(void* src, DWORD sizeToWrite)
{
	//if (isReady()&&src&&getViewOfFile()) {
	//	setStatus(false);
	//	memcpy(getViewOfFile(), src, sizeToWrite);
	//	setStatus(true);
	//}
	//else
	memcpy(getViewOfFile(), src, sizeToWrite);
}
void rcFileMapWriter::lockedWrite(void* src, DWORD sizeToWrite)
{
	m_mutex->lock();
	write(src, sizeToWrite);
	m_mutex->unlock();
}
