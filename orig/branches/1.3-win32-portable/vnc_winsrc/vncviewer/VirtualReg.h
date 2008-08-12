#ifndef __VIRTUALREG_H__
#define __VIRTUALREG_H__

#include <map>
#include "stdhdrs.h"

#define PREF_STRING 'S'
#define PREF_DWORD	'D'

// Key of registr with one parameter
struct ValueKey
{
	char *hive;
	char *valueName;
};

class VirtualReg
{
protected:
	HKEY ihkey;
	TCHAR *fname;
public:
	VirtualReg();
	~VirtualReg();

	void setfname(TCHAR *fileName);

	std::map<HKEY, TCHAR *> hHive;

	LSTATUS RegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS RegOpenKey(HKEY hKey,	LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	LSTATUS RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
	LSTATUS RegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData);
	LSTATUS RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
		REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,LPDWORD lpdwDisposition);
	LSTATUS RegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
		LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS RegEnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved,
		LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS RegDeleteValue(HKEY hKey, LPCTSTR lpValueName);
	LSTATUS RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	LSTATUS RegCloseKey(HKEY hKey);
};

#endif // __VIRTUALREG_H__