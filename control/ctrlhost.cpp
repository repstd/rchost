#include "StdAfx.h"
#include "ctrlhost.h"

CTRLHOST_OPERATOR* CTRLHOST_OPERATOR::instance()
{
	static CTRLHOST_OPERATOR host;
	return &host;
}
int CTRLHOST_OPERATOR::loadConfig(const char* confg)
{
	char strINIPATH[MAX_PATH];
	_fullpath(strINIPATH, confg, MAX_PATH);
	if (GetFileAttributes(strINIPATH) == 0xFFFFFFFF)
	{
		FILE* fp=fopen(confg, "w");
		fclose(fp);

		return ERROR_NOT_FOUND;
	}
	CHAR attrStr[MAX_PATH];
	long hr;
	LPTSTR lpReturnedSections = new TCHAR[MAX_PATH];
	int nSectionsCnt=GetPrivateProfileSectionNames(lpReturnedSections, MAX_PATH, strINIPATH);
	CHAR* psection = lpReturnedSections;
	std::string app;

	while (*psection!=0x00)
	{
		//__STD_PRINT("%s\n", psection);
		app= std::string(psection);
		psection += app.size()+1;
		hr = GetPrivateProfileString(psection,"ip", "", attrStr, MAX_PATH, strINIPATH);
		m_mapNameIP[app.c_str()] = attrStr;
		memset(attrStr, 0, MAX_PATH);
	}
	return ERROR_SUCCESS;
}
int CTRLHOST_OPERATOR::updateConfig(const char* confg)
{
	char strINIPATH[MAX_PATH];
	_fullpath(strINIPATH, confg, MAX_PATH);
	if (GetFileAttributes(strINIPATH) == 0xFFFFFFFF)
	{
		return ERROR_NOT_FOUND;
	}
	CHAR attrStr[MAX_PATH];
	long hr;
	__DEBUG_PRINT("number of clients found: %d\n", m_mapNameIP.size())
	for (CRTHOST_MAP_ITER iter = m_mapNameIP.begin(); iter != m_mapNameIP.end(); iter++)
	{
		__DEBUG_PRINT("trying finding app: %s\n", iter->first.c_str());
		hr = GetPrivateProfileSection(iter->first.c_str(), attrStr, MAX_PATH,strINIPATH);
		if (hr!=0)
			continue;
		__DEBUG_PRINT("trying adding section: %s\n", iter->first.c_str());
		hr = WritePrivateProfileString(iter->first.c_str(), "ip", iter->second.c_str(),strINIPATH);
		if (!hr)
			return GetLastError();
		__DEBUG_PRINT("added section: %s\n", iter->first.c_str());
	}
	return ERROR_SUCCESS;
}
void CTRLHOST_OPERATOR::addClientIP(const char* name,const char* ip)
{
	lock();
	CRTHOST_MAP_ITER iter = m_mapNameIP.find(name);
	if (iter == m_mapNameIP.end())
		m_mapNameIP[name] = ip;
	unlock();

}










