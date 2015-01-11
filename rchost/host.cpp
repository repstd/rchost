#include "stdafx.h"
#include "host.h"
#include <iostream>
#include <string>
#include <atlstr.h> 
#include <sstream>
#include <iterator>
#include <vector>
#include "rc_common.h"
#include <string>
void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

HOST_OPERATOR_API::HOST_OPERATOR_API() :
HOST_CONFIG_API(),
rcmutex()
{
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
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

const char* HOST_OPERATOR_API::getPath(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNamePath.find(filename);
	if (iter == m_mapNamePath.end())
		return NULL;
	else
		return iter->second.c_str();
}
const char* HOST_OPERATOR_API::getArg(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNameArgs.find(filename);
	if (iter == m_mapNameArgs.end())
		return NULL;
	else
		return iter->second.c_str();

}
const char* HOST_OPERATOR_API::getArg(std::string filename, std::string additional)
{

	const char* originalArg = getArg(filename);
	if (originalArg == NULL)
		return NULL;
	char buf[MAX_PATH];
	sprintf(buf, "%s", originalArg);
	char* p = strstr(buf, "--SettledTime");
	if (p==NULL)
	{
		strcat(buf, additional.c_str());
	}
	else
	{
		*p = '\0';
		strcat(buf, additional.c_str());
	}
	return buf;
}
DWORD HOST_OPERATOR_API::createProgram(std::string filename, std::string path, const char* curDir, std::string args, const int arg)

{
	lock();
	std::cout << filename << " " << path << " " << args << std::endl;
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	//Make sure no duplicated task is to be created.
	if (iter != m_vecProgInfo.end())
	{
		__STD_PRINT("%s\n", "duplicated task is pending.");

		PROCESS_INFORMATION oldInfo = iter->second;
		DWORD dwExitCode = 0;
		GetExitCodeProcess(oldInfo.hProcess, &dwExitCode);

		__STD_PRINT("status of forked process: %d\n", dwExitCode);

		if (dwExitCode == STILL_ACTIVE)
		{
			unlock();
			return ERROR_SERVICE_EXISTS;
		}
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
	si.dwFlags = STARTF_RUNFULLSCREEN;
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
		unlock();
		return GetLastError();
	}
#else
	//if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, const_cast<char*>(curDir.c_str()), &si, &pi))
	if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, curDir, &si, &pi))
	{
		unlock();
		return GetLastError();
	}
	m_vecProgInfo[filename.c_str()] = pi;
#if 0
	//now verify the running processes.
	for (HOST_INFO_ITER iter = m_vecProgInfo.begin(); iter != m_vecProgInfo.end(); iter++)
		std::cout << iter->first << " " << iter->second.dwProcessId << std::endl;
#endif
#endif
	unlock();
	return ERROR_SUCCESS;
}
DWORD HOST_OPERATOR_API::createProgram(std::string filename, bool isCurDirNeeded)
{

	const char* path = getPath(filename);
	if (path == NULL)
		return ERROR_NOT_FOUND;

	char* curDir = NULL;
	if (isCurDirNeeded)
		curDir = parsePath(path);
	const char* argv = getArg(filename);

	char args[MAX_PATH];
	if (argv != NULL)
	{
		strcpy(args, argv);

	}
	else
		strcpy(args, "");

	////Add timestamp to the argv
	//if (strstr(filename.c_str(), "rcplayer")!= NULL)
	//{
	//	char timeStampBuf[40];

	//	//sprintf(timeStampBuf, " --StartTime %ld",)

	//}

	return createProgram(filename, path, curDir, args, 1);

}
DWORD HOST_OPERATOR_API::closeProgram(std::string filename)
{
	lock();
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	if (iter == m_vecProgInfo.end())
	{
		unlock();
		return ERROR_NOT_FOUND;
	}

	PROCESS_INFORMATION pi = iter->second;

	//if (!PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0))
	//{
	//	unlock();
	//	return GetLastError();
	//}

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
	unlock();
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
	static HOST_OPERATOR inst;
	return &inst;
}
DWORD HOST_OPERATOR::loadConfig(const char* config)
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

	for (std::vector<std::string>::iterator iter = vecName.begin(); iter != vecName.end(); iter++)
	{
		hr = GetPrivateProfileString("Additional", iter->c_str(), "", attriStr, MAX_PATH, strINIPath);
		if (hr)
		{
			m_mapNameAdditionInfo[iter->c_str()] = std::string(attriStr);
			memset(attriStr, 0, MAX_PATH);
		}
	}
	m_bIsDataLoaded = true;
	return 0;
}

void HOST_OPERATOR::saveHostName(const char* hostname)
{
	if (hostname != NULL)
		strcpy(m_hostname, hostname);
}
const char* HOST_OPERATOR::getHostName()
{
	return m_hostname;
}
void HOST_OPERATOR::saveAdapter(const char* addr)
{
	if (addr != NULL)
		m_vecAdapter.push_back(addr);
}
const char* HOST_OPERATOR::getPrimaryAdapter()
{
	for (std::vector<std::string>::iterator iter = m_vecAdapter.begin(); iter != m_vecAdapter.end(); iter++)
	{
		if (strncmp(iter->c_str(), "10.", 4) != -1
			|| strncmp(iter->c_str(), "192.", 4) != -1
			)
			return iter->c_str();
	}
	return m_vecAdapter[0].c_str();
}
void HOST_OPERATOR::saveHostent(const hostent* host)
{
	if (host != NULL)
		m_host = std::auto_ptr<hostent>(const_cast<hostent*>(host));
}
const hostent* HOST_OPERATOR::getHostent()
{
	return m_host.get();
}

void HOST_OPERATOR::setPort(int port)
{
	m_port = port;
}
const int HOST_OPERATOR::getPort()
{
	return m_port;
}
HOST_MSGHANDLER::HOST_MSGHANDLER(const HOST_MSG* msg) :THREAD(), rcmutex()
{
	//use a normal mutex instead of a recursive one.
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));

	//Assign the server a msg to handle
	m_taskMsg = std::auto_ptr<HOST_MSG>(const_cast<HOST_MSG*>(msg));
}

void HOST_OPERATOR::updateArg(std::string filename, std::string additional)
{

	const char* composite = getArg(filename, additional);
	if (composite == NULL)
		return;
	
	//Now Update
	char temp[MAX_PATH];
	sprintf(temp, "%s", composite);
	m_mapNameArgs.erase(filename);
	m_mapNameArgs[filename.c_str()] = std::string(temp);

}
DWORD HOST_OPERATOR::handleProgram(std::string filename, const char op)
{
	/*
	*Handle miscellaneous conditions for any possible programs here.
	*/
	HOST_MAP_ITER iter = m_mapNameAdditionInfo.find(filename);
	bool isCurDirNeeded = false;
	if (iter != m_mapNameAdditionInfo.end())
	{

		if (strstr(iter->second.c_str(), "-m") != NULL)
		{
			__STD_PRINT("%s\n", "-m");
			if (m_vecPipebroadercaster.empty())
				m_vecPipebroadercaster.push_back(std::auto_ptr<PIPESIGNAL_BROCASTER>(new PIPESIGNAL_BROCASTER(_RC_PIPE_BROADCAST_PORT)));

			if (strstr(filename.c_str(), "video") != NULL)
			{
				switch (op)
				{
				case _OPEN:

					if (!m_vecPipebroadercaster[0]->isRunning())
					{

						__STD_PRINT("Start to signal the rcplayer for %d\n", _RC_PIPE_BROADCAST_PORT);
						/*
						*Start a brocaster for signalling the  vlc player to  step frame by frame
						*/
						m_vecPipebroadercaster[0]->start();
					}
					break;
				case _CLOSE:
					__STD_PRINT("%s\n", "Finish signaling the rcplayer");
					//close the brocaster

					m_vecPipebroadercaster[0]->setCancelModeAsynchronous();
					m_vecPipebroadercaster[0]->cancel();
					m_vecPipebroadercaster[0]->cancelCleanup();
					m_vecPipebroadercaster[0].release();
					m_vecPipebroadercaster.pop_back();
					break;
				}
			}
			//TODO:Specify operation for master prog.
		}
		if (strstr(iter->second.c_str(), "-s") != NULL)
		{
			//TODO:Specigy operatoin for slave prog.
		}
		if (strstr(iter->second.c_str(), "-curDir") != NULL)
		{
			isCurDirNeeded = true;
		}
		if (strstr(iter->second.c_str(), "-defaultDir") != NULL)
		{
			isCurDirNeeded = false;
		}
	}
	return HOST_OPERATOR_API::handleProgram(filename, op, isCurDirNeeded);

}

void HOST_MSGHANDLER::syncTime() const
{

	FILETIME masterFileTime = m_taskMsg->_time;
	SYSTEMTIME curSysTime;
	GetLocalTime(&curSysTime);

	__STD_PRINT("#%d: ", 1); _STD_PRINT_TIME(curSysTime);

	FILETIME curFileTime;
	SystemTimeToFileTime(&curSysTime, &curFileTime);
	ULARGE_INTEGER master;
	ULARGE_INTEGER slave;
	master.HighPart = masterFileTime.dwHighDateTime;
	master.LowPart = masterFileTime.dwLowDateTime;
	slave.HighPart = curFileTime.dwHighDateTime;
	slave.LowPart = curFileTime.dwLowDateTime;

	/*
	*
	*Here we add the timestamp to the arguments list instead of call Sleep.@150110
	*/
	if (m_taskMsg->_operation == _OPEN)
	{

		char timestamp[MAX_PATH];
		sprintf(timestamp, " --SettledTime-%llu", master.QuadPart);

		_LOG_FORMAT_HOST("%s\n", timestamp);

		HOST_OPERATOR::instance()->updateArg(m_taskMsg->_filename,timestamp);
	}

#if 0
	ULONGLONG sleepTime = (master.QuadPart - slave.QuadPart) / 10000.0;

	if (sleepTime < 0)
		sleepTime = 0;
	if (sleepTime > 30 * 1000.0)
		sleepTime = 30 * 1000.0;
	Sleep(sleepTime);
	GetLocalTime(&curSysTime);
	__STD_PRINT("#%d: ", 2); _STD_PRINT_TIME(curSysTime);
#endif
}
const HOST_MSG* HOST_MSGHANDLER::getMSG()
{
	return m_taskMsg.get();
}
void HOST_MSGHANDLER::handle() const
{
#ifdef _TIME_SYNC
	if (strstr(m_taskMsg->_filename, "video"))
	{
		syncTime();
	}
#endif
	HOST_OPERATOR::instance()->handleProgram(m_taskMsg->_filename, m_taskMsg->_operation);

}
void HOST_MSGHANDLER::run()
{
	lock();

	handle();

	unlock();
}


HOST::HOST(const int port) :server(port), THREAD(), rcmutex()
{

	m_port = port;

	queryHostInfo(HOST_OPERATOR::instance());
}
const char* HOST::getName() const
{
	return HOST_OPERATOR::instance()->getHostName();
}
const hostent* HOST::getHostent() const
{
	return HOST_OPERATOR::instance()->getHostent();
}
const char* HOST::getIP() const
{
	return HOST_OPERATOR::instance()->getPrimaryAdapter();
}
void HOST::queryHostInfo(HOST_OPERATOR* ope)
{

	if (ope->getHostent() != NULL)
	{
		return;
	}
	ope->setPort(m_port);
	char name[MAX_PATH];
	int error = gethostname(name, MAX_PATH);
	if (error != 0)
	{
		_LOG_FORMAT_HOST("Error in Querying Host.Error Code:%d\n", error);
		__STD_PRINT("Error in Querying Host.Error Code:%d\n", WSAGetLastError());
		return;
	}
	__STD_PRINT("host: %s\n", name);

	ope->saveHostName(name);

	hostent* hst = gethostbyname(name);

	if (hst == NULL)
	{
		_LOG_FORMAT_HOST("%s\n", "Error in getting host info");
		return;
	}
	ope->saveHostent(hst);

	if (hst->h_addrtype == AF_INET)
	{
		__STD_PRINT("AddressType: %s, IPV4\n", "AF_INET");
		int i = 0;
		char saddr[MAX_PATH];

		while (hst->h_addr_list[i] != NULL)
		{
			char* addr = inet_ntoa(*(in_addr*)hst->h_addr_list[i++]);
			__STD_PRINT("Adapter : %s\n", addr);
			if (addr != NULL)
				ope->saveAdapter(addr);
		}
	}

}
void HOST::run()
{
	char msgRcv[_MAX_DATA_SIZE];
	sockaddr client;
	int sizeRcv;
	_LOG_INIT_HOST
		char feedback[64];

	addPipeServer(_RC_PIPE_NAME);
	/*
	*Start a udp server to listening  a specified port for signaling the child processes.
	*/

	std::auto_ptr<PIPESIGNAL_HANDLER> pipesignal_handler(new PIPESIGNAL_HANDLER(this, _RC_PIPE_BROADCAST_PORT));
	pipesignal_handler->start();

	while (isSocketOpen())
	{
		sizeRcv = -1;
		getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
		if (sizeRcv == _MAX_DATA_SIZE)
		{
			/*
			*Start a thread to finish the program openning/closing task.
			*/
			std::auto_ptr<HOST_MSGHANDLER> slave(new HOST_MSGHANDLER(reinterpret_cast<HOST_MSG*>(msgRcv)));
			slave->Init();
			slave->start();
			slave.release();
		}
		/*
		*Send feedback to the central controller.
		*/
		strcpy(feedback, getName());
		strcat(feedback, "#");
		strcat(feedback, getIP());
		sendPacket(client, feedback, strlen(feedback), 64);
	}
}
void HOST::addPipeServer(const char* pipename)
{
	m_mapNamedPipeServer[pipename] = std::auto_ptr<namedpipeServer>(new namedpipeServer(pipename));
}
void HOST::signalPipeClient()
{

	for (HOST_PIPE_ITER iter = m_mapNamedPipeServer.begin(); iter != m_mapNamedPipeServer.end(); iter++)
	{
#ifdef _PIPE_SYNC
		iter->second->signalClient();
#else
		//__STD_PRINT("%s\n", "next_frame");
#endif
	}
}
HOST_PIPE HOST::getPipeServers()
{
	return m_mapNamedPipeServer;
}
