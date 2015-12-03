#include "StdAfx.h"
#include "simplehost.h"
#include <sstream>
#include <iostream>
typedef std::map<std::string, PROCESS_INFORMATION>::iterator HOST_INFO_ITER;
simplehost::simplehost(const int port) :server(port), THREAD() {
}
simplehost::~simplehost() {

}
void simplehost::run(){
	char msgRcv[_MAX_DATA_SIZE];
	sockaddr client;
	int sizeRcv;
	DWORD result;
	isBlocked = true;
	BlockInput(true);
	while (isSocketOpen())
	{
		sizeRcv = -1;
		memset(msgRcv, 0, sizeof(msgRcv));
		getPacket(client, msgRcv, sizeRcv, _MAX_DATA_SIZE);
		simpleCmd cmd;
		parseMsg(msgRcv, cmd, "#");
		result = 0;
		result = handle(cmd);
		sendPacket(client, &result, sizeof(result), 64);
	}
	if (isBlocked) {
		BlockInput(false);
		isBlocked = false;
	}

}
const char* simplehost::parsePath(const char* fullpath)
{
	//Parse the full path of the program to be created.
	//Here we suppose the input path is of the following style:
	//				c://path//to//program.exe
	//The current dir shuold be: 
	//				c://path//to
	if (!fullpath)
		return fullpath;
	char* p = const_cast<char*>(fullpath);
	char* start = p;
	char* sub;
	while (p != NULL && ((sub = strstr(p, "//")) != NULL || (sub = strstr(p, "\\")) != NULL))
		p = sub + 2;
	int sz = p - start - 2;
	char* curDir = new char[sz];
	memcpy(curDir, start, sz);
	return curDir;
}
void simplehost::parseMsg(char* inMsg, simpleCmd& outMsg, const char* delimeter)
{
	char* pd = strstr(inMsg, delimeter);
	if (pd)
		*pd = '\0';
	outMsg.path = inMsg;
	if (outMsg.path.size())
		outMsg.argc++;
	if (pd != NULL && (pd + 1) != NULL&&*(pd + 1) != '\0') {
		outMsg.arg = pd + 1;
		std::istringstream iss(outMsg.arg);
		std::string temp;
		while (iss >> temp)
			outMsg.argc++;
	}
}
DWORD simplehost::createProgram(std::string path, const char* curDir, std::string args, const int argc) {
	std::cout << "path: " << path << std::endl
		<< "args: " << args << std::endl
		<< "argc: " << argc << std::endl
		<< "curDir: ";
	if (curDir)
		std::cout << curDir << std::endl;
	else
		std::cout << "./" << std::endl;

	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(path);
	//Make sure no duplicated task is to be created.
	if (iter != m_vecProgInfo.end())
	{
		//__STD_PRINT("%s\n", "duplicated task found.");

		PROCESS_INFORMATION oldInfo = iter->second;
		DWORD dwExitCode = 0;
		GetExitCodeProcess(oldInfo.hProcess, &dwExitCode);

		//__STD_PRINT("status of forked process: %d\n", dwExitCode);

		if (dwExitCode == STILL_ACTIVE) {
			closeProgram(path);
			//return ERROR_SERVICE_EXISTS;
		}
		else {
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
	if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, curDir, &si, &pi)){
		return GetLastError();
	}
#endif
	m_vecProgInfo[path.c_str()] = pi;
	return ERROR_SUCCESS;
}
DWORD simplehost::closeProgram(std::string filename)
{
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	if (iter == m_vecProgInfo.end()) {
		return ERROR_NOT_FOUND;
	}

	PROCESS_INFORMATION pi = iter->second;

	if (!PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0)) {
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

	m_vecProgInfo.erase(filename);
	return ERROR_SUCCESS;

}
DWORD simplehost::handle(simpleCmd& cmd){
	DWORD result = 0;
	std::cout << "Path: " << cmd.path << " Args: " << cmd.arg << "  Argc:" << cmd.argc << std::endl;
	if (cmd.path.find("/") == std::string::npos && cmd.path.find("\\") == std::string::npos) {
		if (cmd.path.find("quit") != std::string::npos) {
			std::cout << "quit" << std::endl;
			exit(0);
			return ERROR_SUCCESS;
		}
		else if (cmd.path == "unblock"){
			if (!isBlocked)
				return ERROR;
			std::cout << "UnblockInput" << std::endl;
			isBlocked = false;
			result = BlockInput(false);
			if (result != ERROR_SUCCESS)
				result = GetLastError();
		}
		else if (cmd.path == "block"){
			if (isBlocked)
				return ERROR;
			std::cout << "BlockInput" << std::endl;
			isBlocked = true;
			result = BlockInput(true);
			if (result != ERROR_SUCCESS)
				result = GetLastError();
		}
	}
	else {
		const char* curDir = parsePath(cmd.path.c_str());
		if (cmd.path.size()) {
			result = createProgram(cmd.path, NULL, cmd.arg, cmd.argc);
			if (result != ERROR_SUCCESS)
				result = createProgram(cmd.path, curDir, cmd.arg, cmd.argc);
		}
	}
	return result;
}
