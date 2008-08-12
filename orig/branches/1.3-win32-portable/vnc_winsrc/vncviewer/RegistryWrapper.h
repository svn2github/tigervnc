#ifndef __SAVEOPTION_H__
#define __SAVEOPTION_H__

#include "TCHAR.h"
#include "stdhdrs.h"
#include "IniFileRegistry.h"

enum RegistryBackend
{
	BACKEND_FILE,
	BACKEND_REGISTRY
};

class RegistryWrapper
{
protected:
	RegistryBackend m_backend;
	TCHAR *m_sFileName;
	IniFileRegistry *m_iniReg;
public:
	RegistryWrapper(RegistryBackend backend, TCHAR *fname);
	void ReInit(RegistryBackend backend, TCHAR *fname);
	~RegistryWrapper();

	LSTATUS CreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS OpenKey(HKEY hKey,	LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS OpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	LSTATUS SetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
	LSTATUS SetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData);
	LSTATUS CreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
		REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,LPDWORD lpdwDisposition);
	LSTATUS QueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
		LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS EnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved,
		LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS DeleteValue(HKEY hKey, LPCTSTR lpValueName);
	LSTATUS DeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	LSTATUS CloseKey(HKEY hKey);
};

// Global link
extern RegistryWrapper *registry;

#endif // __SAVEOPTION_H__
