#include "SaveOption.h"

SaveOption *svOpt;

SaveOption::SaveOption(SaveOptTo sot, TCHAR *fname){
	sOptTo = sot;
	if (fname == NULL) {
		sFileName = _tcsdup("Settings.ini");
	} else {
		sFileName = _tcsdup(fname);	
	}
	vReg = new VirtualReg;
	vReg->setfname(fname);
}

void SaveOption::ReInit(SaveOptTo sot, TCHAR *fname) {
	sOptTo = sot;
	if (fname != NULL) {
		sFileName = _tcsdup(fname);
	}
	vReg->setfname(fname);
}

SaveOption::~SaveOption() {
	delete vReg;
	if (sFileName != NULL) {
		free(sFileName);
	}
}

LSTATUS SaveOption::soRegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult) {
	if (sOptTo == sReg) {
		return RegCreateKey(hKey, lpSubKey, phkResult);
	} else {
		// Save to file
		return vReg->RegCreateKey(hKey, lpSubKey, phkResult);
	}
}

LSTATUS SaveOption::soRegOpenKey(HKEY hKey,	LPCTSTR lpSubKey, PHKEY phkResult) {
	if (sOptTo == sReg) {
		return RegOpenKey(hKey, lpSubKey, phkResult);
	} else {
		// Save to file
		return vReg->RegOpenKey(hKey, lpSubKey, phkResult);
	}
}

LSTATUS SaveOption::soRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
	if (sOptTo == sReg) {
		return RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	} else {
		// Save to file
		return vReg->RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	}
}

LSTATUS SaveOption::soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData) {
	if (sOptTo == sReg) {
		return RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	} else {
		// Save to file
		return vReg->RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	}
}

LSTATUS SaveOption::soRegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData) {
	if (sOptTo == sReg) {
		return RegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
	} else {
		// Save to file
		return vReg->RegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
	}
}

LSTATUS SaveOption::soRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
									 REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,LPDWORD lpdwDisposition) {
	if (sOptTo == sReg) {
		return RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	} else {
		// Save to file
		return vReg->RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	}
}

LSTATUS SaveOption::soRegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
									  LPBYTE lpData, LPDWORD lpcbData) {
	if (sOptTo == sReg) {
		return RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	} else {
		// Save to file
		return vReg->RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	}
}

LSTATUS SaveOption::soRegEnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved,
								   LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	if (sOptTo == sReg) {
		return RegEnumValue(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	} else {
		// Save to file
		return vReg->RegEnumValue(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	}
}

LSTATUS SaveOption::soRegDeleteValue(HKEY hKey, LPCTSTR lpValueName) {
	if (sOptTo == sReg) {
		return RegDeleteValue(hKey, lpValueName);
	} else {
		// Save to file
		return vReg->RegDeleteValue(hKey, lpValueName);
	}
}

LSTATUS SaveOption::soRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey) {
	if (sOptTo == sReg) {
		return RegDeleteKey(hKey, lpSubKey);
	} else {
		// Save to file
		return vReg->RegDeleteKey(hKey, lpSubKey);
	}
}

LSTATUS SaveOption::soRegCloseKey(HKEY hKey) {
	if (sOptTo == sReg) {
		return RegCloseKey(hKey);
	} else {
		// Save to file
		return vReg->RegCloseKey(hKey);
	}
}