#include "stdafx.h"
#include "host.h"
#include <iostream>
#include <string>
#include <atlstr.h> 
#include <sstream>
#include <iterator>
#include <vector>

void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

HOST_OPERATOR_API::HOST_OPERATOR_API()
{
	return;
}


HOST_OPERATOR_API::~HOST_OPERATOR_API()
{
}

DWORD HOST_OPERATOR_API::handleProgram(std::string filename, const char op, bool isCurDirNeeded)
{
	switch (op)
	{
	case _OPEN:
		return createProgram(filename, isCurDirNeeded);
	case _CLOSE:
		return closeProgram(filename);
	default:
		return RPC_X_ENUM_VALUE_OUT_OF_RANGE;
	}

}
DWORD HOST_OPERATOR_API::createProgram(std::string filename, std::string path, const char* curDir, std::string args, const int arg)

{
	std::cout << filename << " " << path << " " << args << std::endl;
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	//Make sure no duplicated task is to be created.
	if (iter != m_vecProgInfo.end())
	{
		std::cout << "duplicated task is to pending." << std::endl;
		PROCESS_INFORMATION oldInfo = iter->second;
		DWORD dwExitCode = 0;
		GetExitCodeProcess(oldInfo.hProcess, &dwExitCode);
		std::cout << "status of forked process: " << dwExitCode << std::endl;
		if (dwExitCode == STILL_ACTIVE)
			return ERROR_SERVICE_EXISTS;
		else
		{
			CloseHandle(oldInfo.hProcess);
			CloseHandle(oldInfo.hThread);
			m_vecProgInfo.erase(iter);
		}
	}
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	//Add process access attributes.
#if 0
	PSECURITY_DESCRIPTOR pSD = NULL;
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (NULL == pSD)
	{
		return GetLastError();
	}
	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION))
	{
		return GetLastError();
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		return GetLastError();
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;
#endif

#ifdef UNICODE
	WCHAR* wPath = (WCHAR*)malloc(strlen(path));
	WCHAR* wArgs = (WCHAR*)malloc(strlen(args));

	MultiByteToWideChar(CP_UTF8, 0, path, strlen(path), wPath, strlen(path));
	MultiByteToWideChar(CP_UTF8, 0, args, strlen(args), wArgs, strlen(args));
	if (CreateProcess(_T("C://Windows//System32//calc.exe"), NULL, 0, 0, false, 0, 0, curDir, &si, &pi) != ERROR_SUCCESS)
	{
		return GetLastError();
	}
#else
	//if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, const_cast<char*>(curDir.c_str()), &si, &pi))
	if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, curDir, &si, &pi))
	{
		return GetLastError();
	}
	m_vecProgInfo[filename.c_str()] = pi;
#if 0
	//now verify the running processes.
	for (HOST_INFO_ITER iter = m_vecProgInfo.begin(); iter != m_vecProgInfo.end(); iter++)
		std::cout << iter->first << " " << iter->second.dwProcessId << std::endl;
#endif
#endif

	return ERROR_SUCCESS;
}
DWORD HOST_OPERATOR_API::createProgram(std::string filename, bool isCurDirNeeded)
{

	std::string path;
	HOST_MAP_ITER iter = m_mapNamePath.find(filename);
	if (iter == m_mapNamePath.end())
		return ERROR_NOT_FOUND;
	else
		path = iter->second;
	iter = m_mapNameArgs.find(filename);
	char* curDir = NULL;
	if (isCurDirNeeded)
		curDir = parsePath(path.c_str());

	if (iter != m_mapNameArgs.end())
		return createProgram(filename, path, curDir, iter->second, 1);
	else
		return createProgram(filename, path, curDir, "", 0);

}
DWORD HOST_OPERATOR_API::closeProgram(std::string filename)
{
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	if (iter == m_vecProgInfo.end())
	{
		return ERROR_NOT_FOUND;
	}

	PROCESS_INFORMATION pi = iter->second;

	if (!PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0))
		return GetLastError();
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{
		TerminateThread(pi.hThread, dwExitCode);
		TerminateProcess(pi.hProcess, dwExitCode);
	}
	//if (!PostThreadMessage(pi.dwThreadId, WM_KEYDOWN, VK_ESCAPE, 0))
	//	return GetLastError();
	//if (!PostThreadMessage(pi.dwThreadId, WM_KEYUP, VK_ESCAPE, 0))
	//	return GetLastError();
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	m_vecProgInfo.erase(iter);
	return ERROR_SUCCESS;

}

char* HOST_OPERATOR_API::parsePath(const char* fullpath)
{

	//Parse the full path of the program to be created.
	//Here we suppose the input path is of the following style:
	//				c://path//to//program.exe
	//The current dir shuold be: 
	//				c://path//to
	char* p = const_cast<char*>(fullpath);
	char* start = p;
	char* sub;
	while (p != NULL && (sub = strstr(p, "//")) != NULL)
		p = sub + 2;
	int sz = p - start - 2;
	char* curDir = new char[sz];
	memcpy(curDir, start, sz);
	//std::cout<<curDir<<std::endl;
	return curDir;

}

HOST_OPERATOR* HOST_OPERATOR::instance()
{
	if (g_hostOperator == NULL)
	{
		g_hostOperator = new HOST_OPERATOR;
	}
	return g_hostOperator;
}
const char* HOST_OPERATOR::getPath(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNamePath.find(filename);
	if (iter == m_mapNamePath.end())
		return NULL;
	else
		return iter->second.c_str();
}
const char* HOST_OPERATOR::getArg(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNameArgs.find(filename);
	if (iter == m_mapNameArgs.end())
		return NULL;
	else
		return iter->second.c_str();
}
DWORD HOST_OPERATOR::loadPathMap(const char* config)
{
	if (m_bIsDataLoaded)
		return 0;
	char strINIPath[MAX_PATH];
	_fullpath(strINIPath, config, MAX_PATH);
	if (GetFileAttributes(strINIPath) == 0xFFFFFFFF)
	{

		return ERROR_NOT_FOUND;
	}
	CHAR	attriStr[MAX_PATH];
	long hr;
	hr = GetPrivateProfileString("ProgramsList", "Names", "", attriStr, MAX_PATH, strINIPath);
	std::vector<std::string> vecName;
	if (hr)
	{
		std::istringstream buf(attriStr);
		std::istream_iterator<std::string> beg(buf), end;
		std::vector<std::string> tokens(beg, end);
		vecName.assign(tokens.begin(), tokens.end());
	}
	else
	{
		return ERROR_NOT_FOUND;
	}

	memset(attriStr, 0, MAX_PATH);
	for (std::vector<std::string>::iterator iter = vecName.begin(); iter != vecName.end(); iter++)
	{
		hr = GetPrivateProfileString("Path", iter->c_str(), "", attriStr, MAX_PATH, strINIPath);
		if (!hr)
			return ERROR_NOT_FOUND;
		m_mapNamePath[iter->c_str()] = std::string(attriStr);
		memset(attriStr, 0, MAX_PATH);
	}
	for (std::vector<std::string>::iterator iter = vecName.begin(); iter != vecName.end(); iter++)
	{
		hr = GetPrivateProfileString("Args", iter->c_str(), "", attriStr, MAX_PATH, strINIPath);
		if (hr)
		{
			m_mapNameArgs[iter->c_str()] = std::string(attriStr);
			memset(attriStr, 0, MAX_PATH);
		}
	}
	m_bIsDataLoaded = true;
	return 0;
}


HOST_SLAVE::HOST_SLAVE(const HOST_MSG* msg)
:OpenThreads::Thread()
{
	initMutex(new OpenThreads::Mutex(OpenThreads::Mutex::MUTEX_RECURSIVE));
	m_taskMsg = std::auto_ptr<HOST_MSG>(const_cast<HOST_MSG*>(msg));
}
void HOST_SLAVE::handle() const
{
	if (strstr(m_taskMsg->_filename, "cegui") == 0)
		HOST_OPERATOR::instance()->handleProgram(m_taskMsg->_filename, m_taskMsg->_operation, false);
	else
		HOST_OPERATOR::instance()->handleProgram(m_taskMsg->_filename, m_taskMsg->_operation, true);
}
void HOST_SLAVE::run()
{
	m_mutex->lock();

	handle();

	m_mutex->unlock();
}
void HOST_SLAVE::initMutex(OpenThreads::Mutex* mutex)
{
	m_mutex = std::auto_ptr<OpenThreads::Mutex>(mutex);
}
const OpenThreads::Mutex* HOST_SLAVE::getMutex() const
{
	return m_mutex.get();
}

HOST::HOST(const int port)
:server(port),
	OpenThreads::Thread()
{
}

void HOST::run()
{
	char msgRcv[_MAX_DATA_SIZE];
	sockaddr client;
	int sizeRcv;
	while (isSocketOpen())
	{
		sizeRcv = -1;
		getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
		if (sizeRcv == _MAX_DATA_SIZE)
		{
			std::auto_ptr<HOST_SLAVE> slave(new HOST_SLAVE(reinterpret_cast<HOST_MSG*>(msgRcv)));
			slave->Init();
			slave->start();
			//slave->join();
			slave.release();
		}

	}
}