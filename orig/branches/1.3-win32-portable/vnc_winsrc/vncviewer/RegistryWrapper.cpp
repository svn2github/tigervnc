#include "RegistryWrapper.h"

RegistryWrapper *registry;

RegistryWrapper::RegistryWrapper(RegistryBackend backend, TCHAR *fname)
{
	m_backend = backend;
	if (fname == NULL) {
		m_sFileName = _tcsdup("Settings.ini");
	} else {
		m_sFileName = _tcsdup(fname);	
	}
	m_iniReg = new IniFileRegistry;
	m_iniReg->setfname(fname);
}

void RegistryWrapper::ReInit(RegistryBackend backend, TCHAR *fname)
{
	m_backend = backend;
	if (fname != NULL) {
		m_sFileName = _tcsdup(fname);
	}
	m_iniReg->setfname(fname);
}

RegistryWrapper::~RegistryWrapper()
{
	delete m_iniReg;
	if (m_sFileName != NULL) {
		free(m_sFileName);
	}
}

LSTATUS RegistryWrapper::CreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegCreateKey(hKey, lpSubKey, phkResult);
	} else {
		// Save to file
		return m_iniReg->RegCreateKey(hKey, lpSubKey, phkResult);
	}
}

LSTATUS RegistryWrapper::OpenKey(HKEY hKey,	LPCTSTR lpSubKey, PHKEY phkResult)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegOpenKey(hKey, lpSubKey, phkResult);
	} else {
		// Save to file
		return m_iniReg->RegOpenKey(hKey, lpSubKey, phkResult);
	}
}

LSTATUS RegistryWrapper::OpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions,
								   REGSAM samDesired, PHKEY phkResult)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	} else {
		// Save to file
		return m_iniReg->RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	}
}

LSTATUS RegistryWrapper::SetValueEx(HKEY hKey,
									LPCSTR lpValueName,
									DWORD Reserved,
									DWORD dwType,
									const BYTE *lpData,
									DWORD cbData)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	} else {
		// Save to file
		return m_iniReg->RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	}
}

LSTATUS RegistryWrapper::SetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType,
								  LPCTSTR lpData, DWORD cbData)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
	} else {
		// Save to file
		return m_iniReg->RegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
	}
}

LSTATUS RegistryWrapper::CreateKeyEx(HKEY hKey,
									 LPCTSTR lpSubKey,
									 DWORD Reserved,
									 LPTSTR lpClass,
									 DWORD dwOptions,
									 REGSAM samDesired,
									 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
									 PHKEY phkResult,
									 LPDWORD lpdwDisposition)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	} else {
		// Save to file
		return m_iniReg->RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	}
}

LSTATUS RegistryWrapper::QueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved,
									  LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	} else {
		// Save to file
		return m_iniReg->RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	}
}

LSTATUS RegistryWrapper::EnumValue(HKEY hKey,
								   DWORD dwIndex,
								   LPTSTR lpValueName,
								   LPDWORD lpcchValueName,
								   LPDWORD lpReserved,
								   LPDWORD lpType,
								   LPBYTE lpData,
								   LPDWORD lpcbData)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegEnumValue(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	} else {
		// Save to file
		return m_iniReg->RegEnumValue(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	}
}

LSTATUS RegistryWrapper::DeleteValue(HKEY hKey, LPCTSTR lpValueName)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegDeleteValue(hKey, lpValueName);
	} else {
		// Save to file
		return m_iniReg->RegDeleteValue(hKey, lpValueName);
	}
}

LSTATUS RegistryWrapper::DeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegDeleteKey(hKey, lpSubKey);
	} else {
		// Save to file
		return m_iniReg->RegDeleteKey(hKey, lpSubKey);
	}
}

LSTATUS RegistryWrapper::CloseKey(HKEY hKey)
{
	if (m_backend == BACKEND_REGISTRY) {
		return RegCloseKey(hKey);
	} else {
		// Save to file
		return m_iniReg->RegCloseKey(hKey);
	}
}
