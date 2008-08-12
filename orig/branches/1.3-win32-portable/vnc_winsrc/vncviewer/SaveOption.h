#ifndef __SAVEOPTION_H__
#define __SAVEOPTION_H__

#include "TCHAR.h"
#include "stdhdrs.h"
#include "VirtualReg.h"


enum SaveOptTo
{
	S_FILE,
	S_REG
};


class SaveOption
{
protected:
	SaveOptTo sOptTo;
	TCHAR *sFileName;
	VirtualReg *vReg;
public:
	SaveOption(SaveOptTo sot, TCHAR *fname);
	void ReInit(SaveOptTo sot, TCHAR *fname);
	~SaveOption();

	LSTATUS soRegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS soRegOpenKey(HKEY hKey,	LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS soRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	LSTATUS soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
	LSTATUS soRegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData);
	LSTATUS soRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
		REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,LPDWORD lpdwDisposition);
	LSTATUS soRegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
		LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS soRegEnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved,
		LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS soRegDeleteValue(HKEY hKey, LPCTSTR lpValueName);
	LSTATUS soRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	LSTATUS soRegCloseKey(HKEY hKey);
};

// Global link
extern SaveOption *svOpt;

#endif // __SAVEOPTION_H__
