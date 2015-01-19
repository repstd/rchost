#include "stdafx.h"
#include <iostream>
#include <string>
#include <atlstr.h> 
#include <sstream>
#include <iterator>
#include <vector>
#include <string>
#include "host.h"
#include "rc_common.h"
void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}
PipeSignalBrocaster::PipeSignalBrocaster(const int port) : client(port, NULL), THREAD()
{

	loadIP("ip.ini");
	__STD_PRINT("%s\n", "ip list loaded");
}
DWORD PipeSignalBrocaster::loadIP(const char* confg)
{
	char strINIPATH[MAX_PATH];
	_fullpath(strINIPATH, confg, MAX_PATH);
	if (GetFileAttributes(strINIPATH) == 0xFFFFFFFF)
	{
		FILE* fp = fopen(confg, "w");
		fclose(fp);
		return ERROR_NOT_FOUND;
	}

	CHAR attrStr[MAX_PATH];
	long hr;
	LPTSTR lpReturnedSections = new TCHAR[MAX_PATH];
	if (!GetPrivateProfileSectionNames(lpReturnedSections, MAX_PATH, strINIPATH))
	{
		return ERROR_NOT_FOUND;
	}
	CHAR* psection = lpReturnedSections;
	_LOG_FORMAT_HOST("%s\n", lpReturnedSections);
	std::string app;
	while (*psection != 0x00)
	{
		__STD_PRINT("%s\n", psection);
		app = std::string(psection);
		psection += app.size() + 1;
		hr = GetPrivateProfileString(psection, "ip", "", attrStr, MAX_PATH, strINIPATH);
		m_mapIpFlag[app.c_str()] = false;
		memset(attrStr, 0, MAX_PATH);
	}
	return ERROR_SUCCESS;

}
void PipeSignalBrocaster::run()
{

	char msgRcv[_MAX_DATA_SIZE];
	int size = -1;
	std::string strMsg;
	while (isSocketOpen())
	{
		char* msg = "pipe";
		struct sockaddr from;
		sendPacket(msg, strlen(msg));
#if 0
		__STD_PRINT("%s\n", "message Sent");
#endif
		while (1)
		{
#if 0
			__STD_PRINT("%s\n", "waiting...");
#endif
			getPacket(from, msgRcv, size, _MAX_DATA_SIZE);
			strMsg.assign(msgRcv);
			m_mapIpFlag[strMsg.c_str()] = true;
			int allReceived = 1;
			for (std::map<const std::string, bool, cmp>::iterator iter = m_mapIpFlag.begin(); iter != m_mapIpFlag.end(); iter++)
			{
				if (!iter->second)
				{
					allReceived = 0;
					_LOG_FORMAT_HOST("IPLIST End :%s\n", iter->first.c_str());
					break;
				}
			}
			if (allReceived == 1)
			{
				for (std::map<const std::string, bool, cmp>::iterator iter = m_mapIpFlag.begin(); iter != m_mapIpFlag.end(); iter++)
				{
					iter->second = false;
				}
#if 0
				__STD_PRINT("%s\n", "All Reveived.");
#endif
				break;
			}
		}
	}

}
int PipeSignalBrocaster::cancel()
{
	return THREAD::cancel();
}
listenerSlave::listenerSlave(SOCKET socket) :THREAD(), client(), rcmutex()
{
	m_socket = socket;
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
}
void listenerSlave::run()
{
	char msgRcv[_MAX_DATA_SIZE];
	int size = -1;
	sockaddr in;
	std::string strMsg;
	std::map<const std::string, bool, cmp>::iterator iter;
	while (isSocketOpen())
	{
		size = -1;
		getPacket(in, msgRcv, size, _MAX_DATA_SIZE);
		if (size != -1)
		{
			strMsg.assign(msgRcv);

			lock();
			iter = g_mapIpFlag.find(strMsg.c_str());
			if (iter != g_mapIpFlag.end())
				iter->second = true;
			unlock();
			__STD_PRINT("ThreadID: %d\t Fliped:\t", getThreadId());
			__STD_PRINT("%s\n ", strMsg.c_str());
		}
	}
}

multiListener::multiListener(const int port) : client(port, NULL), THREAD(), rcmutex()
{
	loadIP("ip.ini");
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	__STD_PRINT("%s\n", "ip list loaded");
}

multiListener ::~multiListener()
{
	for (std::vector<std::shared_ptr<listenerSlave>>::iterator iter = m_vecSlaves.begin(); iter != m_vecSlaves.end(); iter++)
	{
		(*iter)->setCancelModeAsynchronous();
		(*iter)->cancel();
	}
}
DWORD multiListener::loadIP(const char* confg)
{

	char strINIPATH[MAX_PATH];
	_fullpath(strINIPATH, confg, MAX_PATH);
	if (GetFileAttributes(strINIPATH) == 0xFFFFFFFF)
	{
		FILE* fp = fopen(confg, "w");
		fclose(fp);
		return ERROR_NOT_FOUND;
	}

	CHAR attrStr[MAX_PATH];
	long hr;
	LPTSTR lpReturnedSections = new TCHAR[MAX_PATH];
	if (!GetPrivateProfileSectionNames(lpReturnedSections, MAX_PATH, strINIPATH))
	{
		return ERROR_NOT_FOUND;
	}
	CHAR* psection = lpReturnedSections;
	_LOG_FORMAT_HOST("%s\n", lpReturnedSections);
	std::string app;
	while (*psection != 0x00)
	{
		__STD_PRINT("%s\n", psection);
		app = std::string(psection);
		psection += app.size() + 1;
		hr = GetPrivateProfileString(psection, "ip", "", attrStr, MAX_PATH, strINIPATH);
		g_mapIpFlag[app.c_str()] = false;
		memset(attrStr, 0, MAX_PATH);
	}
	return ERROR_SUCCESS;
}
void multiListener::run()
{
	int first = 1;
	Sleep(5000);
	int cnt = 0;
	SYSTEMTIME time;
	std::unique_ptr<SYNC_MSG> syncMsg(new SYNC_MSG);
	ULONGLONG indexFrame = 0;
	struct sockaddr from;
	int totalSlaveCnt = g_mapIpFlag.size();
	int unrespondedSlaveCnt = 0;
	int tolerance = 2;
	int allReceived;
	while (isSocketOpen())
	{

		GetLocalTime(&time);
		syncMsg->_timeStamp = time;
		syncMsg->_index = ++indexFrame;
		strcpy(syncMsg->_userData, "control#next_frame");
		sendPacket((char*)(syncMsg.get()), sizeof(SYNC_MSG));
		if (first)
		{
			first = 0;

			for (int i = 0; i < 24; i++)
			{
				std::shared_ptr<listenerSlave> slave(new listenerSlave(getSocket()));
				slave->Init();
				slave->start();
				m_vecSlaves.push_back(slave);
			}
		}
		Sleep(10);
		cnt = 0;
		while (1)
		{
			//for (std::map<const std::string, bool, cmp>::iterator iter = g_mapIpFlag.begin(); iter != g_mapIpFlag.end(); iter++)
			//{
			//	__STD_PRINT("%s  ", iter->first.c_str());
			//	__STD_PRINT("%d \n", iter->second);
			//}
			cnt += 1;
			allReceived = 1;
			unrespondedSlaveCnt = 0;
			for (std::map<const std::string, bool, cmp>::iterator iter = g_mapIpFlag.begin(); iter != g_mapIpFlag.end(); iter++)
			{
				if (!iter->second)
				{
					allReceived = 0;
					unrespondedSlaveCnt += 1;
				}
			}
			if (unrespondedSlaveCnt == totalSlaveCnt)
				continue;
			if (unrespondedSlaveCnt< tolerance)
				allReceived = 1;
			if (cnt>300 && !allReceived)
			{
				allReceived = 1;
				__DEBUG_PRINT("%s\n", "Expired.");
			}
			if (allReceived == 1)
			{
				lock();
				for (std::map<const std::string, bool, cmp>::iterator iter = g_mapIpFlag.begin(); iter != g_mapIpFlag.end(); iter++)
				{
					iter->second = false;
				}
				unlock();
				__DEBUG_PRINT("%s\n", "All received.");
				__DEBUG_PRINT("%s\n", "*******************************");
				break;
			}
		}
	}
}

int multiListener::cancel()
{
	return THREAD::cancel();
}
HostOperatorAPI::HostOperatorAPI() : HOST_CONFIG_API(), rcmutex()
{
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
	return;
}

HostOperatorAPI::~HostOperatorAPI()
{
}

DWORD HostOperatorAPI::handleProgram(std::string filename, const char op, bool isCurDirNeeded)
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

std::string HostOperatorAPI::getPath(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNamePath.find(filename);
	if (iter == m_mapNamePath.end())
		return "";
	else
		return iter->second;
}
std::string HostOperatorAPI::getArg(std::string filename)
{
	HOST_MAP_ITER iter = m_mapNameArgs.find(filename);
	if (iter == m_mapNameArgs.end())
		return NULL;
	else
		return iter->second;

}
std::string HostOperatorAPI::getArg(std::string filename, std::string additional)
{

	std::string originalArg = getArg(filename).c_str();
	if (originalArg.c_str() == NULL)
		return NULL;
	char buf[MAX_PATH];
	sprintf(buf, "%s", originalArg.c_str());
	char* p = strstr(buf, "--SettledTime");
	if (p == NULL)
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
DWORD HostOperatorAPI::createProgram(std::string filename, std::string path, const char* curDir, std::string args, const int argc)

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
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

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
DWORD HostOperatorAPI::createProgram(std::string filename, bool isCurDirNeeded)
{

	std::string strPath = getPath(filename);
	if (strPath.empty())
		return ERROR_NOT_FOUND;

	char* curDir = NULL;
	if (isCurDirNeeded)
		curDir = parsePath(strPath.c_str());

	std::string strArgv = getArg(filename).c_str();
	__STD_PRINT("Fetched Args: %s\n", strArgv.c_str());
	char args[MAX_PATH];
	if (!strArgv.empty())
	{
		strcpy(args, strArgv.c_str());
	}
	else
		strcpy(args, "");

	////Add timestamp to the argv
	//if (strstr(filename.c_str(), "rcplayer")!= NULL)
	//{
	//	char timeStampBuf[40];

	//	//sprintf(timeStampBuf, " --StartTime %ld",)

	//}

	return createProgram(filename, strPath, curDir, args, 1);

}
DWORD HostOperatorAPI::closeProgram(std::string filename)
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

	if (!PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0))
	{
		unlock();
		return GetLastError();
	}

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

char* HostOperatorAPI::parsePath(const char* fullpath)
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

HostOperator* HostOperator::m_inst = new HostOperator;
HostOperator* HostOperator::instance() { return m_inst; }

DWORD HostOperator::loadConfig(const char* config)
{
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
	return 0;
}

void HostOperator::saveHostName(const char* hostname)
{
	if (hostname != NULL)
		strcpy(m_hostname, hostname);
}
const char* HostOperator::getHostName()
{
	return m_hostname;
}
void HostOperator::saveAdapter(const char* addr)
{
	if (addr != NULL)
		m_vecAdapter.push_back(addr);
}
const char* HostOperator::getPrimaryAdapter()
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
void HostOperator::saveHostent(const hostent* host)
{
	if (host != NULL)
		m_host = std::unique_ptr<hostent>(const_cast<hostent*>(host));
}
const hostent* HostOperator::getHostent()
{
	return m_host.get();
}

void HostOperator::setPort(int port)
{
	m_port = port;
}
const int HostOperator::getPort()
{
	return m_port;
}

void HostOperator::updateArg(std::string filename, std::string additional)
{

	lock();
	const char* composite = getArg(filename, additional).c_str();

	if (composite == NULL)
	{
		unlock();
		return;
	}

	//Now Update
	char temp[MAX_PATH];
	sprintf(temp, "%s", composite);
	m_mapNameArgs.erase(filename);
	m_mapNameArgs[filename.c_str()] = std::string(temp);
	unlock();
}

DWORD HostOperator::handleProgram(std::string filename, const char op)
{
	/*
	*Handle miscellaneous conditions for any possible programs here.
	*/

	HOST_MAP_ITER iter = m_mapNameAdditionInfo.find(filename);
	bool isCurDirNeeded = false;
	if (iter != m_mapNameAdditionInfo.end())
	{
		if (strstr(iter->second.c_str(), "-s") != NULL)
		{
			//TODO:Specify operatoins for slave prog.
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

	HostOperatorAPI::handleProgram(filename, op, isCurDirNeeded);

	if (iter != m_mapNameAdditionInfo.end())
	{
		if (strstr(iter->second.c_str(), "-m") != NULL)
		{
			__DEBUG_PRINT("%s\n", "-m");
			if (m_vecPipebroadercaster.empty())
				m_vecPipebroadercaster.push_back(std::auto_ptr<multiListener>(new multiListener(_RC_PIPE_BROADCAST_PORT)));

			switch (op)
			{
			case _OPEN:

				if (!m_vecPipebroadercaster[0]->isRunning())
				{

					__STD_PRINT("Start to signal the rcplayer for %d\n", _RC_PIPE_BROADCAST_PORT);
					//Start a brocaster for signalling the  vlc player to  step frame by frame
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
			default:
				break;
			}
			//TODO:Specify operation for master prog.
		}
	}
	return ERROR_SUCCESS;
}

HostMsgHandler::HostMsgHandler(const HOST_MSG* msg) :THREAD(), rcmutex()
{
	//use a normal mutex instead of a recursive one.
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));

	//Assign the server a msg to handle
	m_taskMsg = std::auto_ptr<HOST_MSG>(const_cast<HOST_MSG*>(msg));
}
void HostMsgHandler::setMSG(HOST_MSG* msg)
{

	m_taskMsg = std::auto_ptr<HOST_MSG>(const_cast<HOST_MSG*>(msg));
}
const HOST_MSG* HostMsgHandler::getMSG()
{
	return m_taskMsg.get();
}
void HostMsgHandler::handle() const
{

	//#ifdef _TIME_SYNC
	//	syncTime();
	//#endif
	HostOperator::instance()->handleProgram(m_taskMsg->_filename, m_taskMsg->_operation);

}
void HostMsgHandler::run()
{
	lock();

	handle();

	unlock();
}

void HostMsgHandler::syncTime() const
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

#if 0 
	/*
	*
	*Here we add the timestamp to the arguments list instead of call Sleep.@150110
	*/

	if (m_taskMsg->_operation == _OPEN)
	{

		char timestamp[MAX_PATH];
		sprintf(timestamp, " --SettledTime-%llu", master.QuadPart);

		_LOG_FORMAT_HOST("%s\n", timestamp);

		HostOperator::instance()->updateArg(m_taskMsg->_filename, timestamp);
	}
#else
	ULONGLONG sleepTime = (master.QuadPart - slave.QuadPart) / 10000;

	if (sleepTime < 0)
		sleepTime = 0;
	if (sleepTime > 30 * 1000.0)
		sleepTime = 30 * 1000.0;
	Sleep(sleepTime);
	GetLocalTime(&curSysTime);
	__STD_PRINT("#%d: ", 2); _STD_PRINT_TIME(curSysTime);
#endif
}


host::host(const int port) :server(port), THREAD(), rcmutex()
{

	m_port = port;

	queryHostInfo(HostOperator::instance());
}
const char* host::getName() const
{
	return HostOperator::instance()->getHostName();
}
const hostent* host::getHostent() const
{
	return HostOperator::instance()->getHostent();
}
const char* host::getIP() const
{
	return HostOperator::instance()->getPrimaryAdapter();
}
void host::queryHostInfo(HostOperator* ope)
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

		while (hst->h_addr_list[i] != NULL)
		{
			char* addr = inet_ntoa(*(in_addr*)hst->h_addr_list[i++]);
			__STD_PRINT("Adapter : %s\n", addr);
			if (addr != NULL)
				ope->saveAdapter(addr);
		}
	}

}
void host::run()
{
	char msgRcv[_MAX_DATA_SIZE];
	sockaddr client;
	int sizeRcv;
	_LOG_INIT_HOST
		char feedback[64];

	addPipeServer(_RC_PIPE_NAME);
	/*
	*Start a udp server to listen  a specified port for signaling the child processes.
	*/
#ifdef _PIPE_SYNC
	std::unique_ptr<PipeSignalHandler> pipesignal_handler(new PipeSignalHandler(this, _RC_PIPE_BROADCAST_PORT));
	pipesignal_handler->start();
#endif
	while (isSocketOpen())
	{
		sizeRcv = -1;
		getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
		if (sizeRcv == _MAX_DATA_SIZE)
		{
			/*
			*Start a thread to finish the program openning/closing task.
			*/
			std::unique_ptr<HostMsgHandler> slave(new HostMsgHandler(reinterpret_cast<HOST_MSG*>(msgRcv)));
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
void host::addPipeServer(const char* pipename)
{
	m_mapNamedPipeServer[pipename] = std::shared_ptr<namedpipeServer>(new namedpipeServer(pipename));
}
void host::signalPipeClient()
{

	for (HOST_PIPE_ITER iter = m_mapNamedPipeServer.begin(); iter != m_mapNamedPipeServer.end(); iter++)
	{
#ifdef _PIPE_SYNC
		iter->second->signalClient();
#else
		__STD_PRINT("%s\n", "next_frame");
#endif
	}
}
HOST_PIPE host::getPipeServers()
{
	return m_mapNamedPipeServer;
}
PipeServer::PipeServer(const char* pipename) :THREAD()
{
	m_pipeServer = std::unique_ptr<namedpipeServer>(new namedpipeServer(pipename));
}
void PipeServer::run()
{

	char log[512];
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	_STD_ENCODE_TIMESTAMP(log, systime);
	_LOG_FORMAT_HOST("%s", log);
	while (1)
	{
		m_pipeServer->signalClient();
		memset(log, 0, 512);
		GetLocalTime(&systime);
		_STD_ENCODE_TIMESTAMP(log, systime);
		strcat(log, "#");
		strcat(log, m_pipeServer->getPipeName());
		_LOG_FORMAT_HOST("%s\n", log);
	}
	__STD_PRINT("%s\n", "pipe exit");
}
void PipeServer::cancle()
{
	m_pipeServer->disconnect();
	m_pipeServer->closeHandle();
	THREAD::cancel();
}
PipeSignalHandler::PipeSignalHandler(host* phost, const int port) : server(port), THREAD()
{
	m_host = std::unique_ptr<host>(phost);
}
void PipeSignalHandler::run()
{

	char msgRcv[MAX_PATH];
	sockaddr client;
	int szRcv = -1;
	const char* received = HostOperator::instance()->getHostName();
	while (isSocketOpen())
	{
		szRcv = -1;
		getPacket(client, msgRcv, szRcv, MAX_PATH);
		if (szRcv != -1)
		{
			char* msgStr = (char*)msgRcv;
			if (strstr(msgStr, "pipe") != NULL)
			{
				m_host->signalPipeClient();
			}
			sendPacket(client, const_cast<char*>(received), strlen(received) + 1, _MAX_DATA_SIZE);
		}

	}

}