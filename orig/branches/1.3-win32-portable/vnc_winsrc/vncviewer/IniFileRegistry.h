//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

#ifndef __INIFILEREGISTRY_H__
#define __INIFILEREGISTRY_H__

#include <map>
#include "stdhdrs.h"

struct ValueKey
{
	char *hive;
	char *valueName;
};

class IniFileRegistry
{
public:
	IniFileRegistry(const TCHAR *fileName = NULL);
	~IniFileRegistry();

	void SetFileName(const TCHAR *fileName);

	LSTATUS RegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
	LSTATUS RegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
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

protected:
	HKEY m_ihkey;
	TCHAR *fname;

	std::map<HKEY, TCHAR *> hHive;

};

#endif // __INIFILEREGISTRY_H__