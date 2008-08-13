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

#ifndef __REGISTRYWRAPPER_H__
#define __REGISTRYWRAPPER_H__

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
		REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
	LSTATUS QueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
		LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS EnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved,
		LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	LSTATUS DeleteValue(HKEY hKey, LPCTSTR lpValueName);
	LSTATUS DeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	LSTATUS CloseKey(HKEY hKey);

protected:
	RegistryBackend m_backend;
	IniFileRegistry *m_iniReg;
};

// Global link
extern RegistryWrapper *registry;

#endif // __REGISTRYWRAPPER_H__
