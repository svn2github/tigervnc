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

#include "IniFileRegistry.h"

#define PREF_STRING 'S'
#define PREF_DWORD	'D'

IniFileRegistry::IniFileRegistry(const TCHAR *fileName)
{
	m_ihkey = (HKEY)1;
	SetFileName(fileName);
}

IniFileRegistry::~IniFileRegistry()
{
	if (fname != NULL) {
		free(fname);
	}
}

void IniFileRegistry::SetFileName(const TCHAR *fileName)
{
	if (fileName != NULL) {
		fname = _tcsdup(fileName);
	} else {
		fname = _tcsdup("Settings.ini");
	}
}

LSTATUS IniFileRegistry::RegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);

	// Create new hive
	TCHAR *newHive;
	if (iter == hHive.end()) {
		newHive = new TCHAR[_tcslen(lpSubKey) + 11];
		_sntprintf(newHive, 10, _T("%x\\"), hKey);
	} else {
		newHive = new TCHAR[_tcslen((*iter).second) + _tcslen(lpSubKey) + 2]; //
		_tcscpy(newHive, (*iter).second);
		if (lpSubKey != NULL) {
			_tcscat(newHive, _T("\\"));
		}
	}
	if (lpSubKey != NULL) {
		_tcscat(newHive, lpSubKey);
	}
	// Insert new value to map
	hHive[m_ihkey] = newHive;
	// return phkResult
	iter = hHive.find(m_ihkey);
	if (iter == hHive.end()) {
		return ERROR_INVALID_ACCESS;
	} else {
		*phkResult = (*iter).first;
		m_ihkey++;
	}
	return ERROR_SUCCESS;
}

LSTATUS IniFileRegistry::RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey,
								   DWORD Reserved,
								   LPTSTR lpClass,
								   DWORD dwOptions,
								   REGSAM samDesired,
								   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								   PHKEY phkResult,LPDWORD lpdwDisposition)
{
	return RegCreateKey(hKey, lpSubKey, phkResult);
}

LSTATUS IniFileRegistry::RegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	return RegCreateKey(hKey, lpSubKey, phkResult);
}

LSTATUS IniFileRegistry::RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions,
								 REGSAM samDesired, PHKEY phkResult)
{
	return RegCreateKey(hKey, lpSubKey, phkResult);
}

LSTATUS IniFileRegistry::RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
								  DWORD dwType, const BYTE *lpData, DWORD cbData)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);
	if (iter == hHive.end()) {
		return ERROR_PATH_NOT_FOUND;
	}
	char *value, v[16];
	switch (dwType) {
		case REG_SZ:
			value = new char[strlen((char *)lpData) + 2];
			value[0] = PREF_STRING;
			_tcscpy(value + 1, (char *) lpData);
			break;
		case REG_DWORD:
			value = v;
			*value = PREF_DWORD;
			ltoa(*((int *) lpData), value + 1, 10);
			break;
		default:
			return ERROR_INVALID_DATA;
	}
	WritePrivateProfileString((*iter).second, lpValueName, value, fname);
	return ERROR_SUCCESS;
}

// FIXME: The function is not implemented.
LSTATUS IniFileRegistry::RegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType,
								LPCTSTR lpData, DWORD cbData)
{
	return RegSetValueEx(hKey, "lpValueName", NULL, dwType, (BYTE *)lpData, cbData);
}

LSTATUS IniFileRegistry::RegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved,
									LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);
	if (iter == hHive.end()) {
		return ERROR_PATH_NOT_FOUND;
	}

	int bufLength = *lpcbData + 16;
	TCHAR *value = new TCHAR[bufLength];

	bufLength = GetPrivateProfileString((*iter).second, lpValueName, _T(""), value, bufLength, fname);
	if (bufLength == 0) {
		delete[] value;
		return ERROR_FILE_NOT_FOUND;
	}
	switch (value[0]) {
		case PREF_STRING:
			if (bufLength - sizeof(PREF_STRING) + 1 >  *lpcbData) { // '... + 1' for null character. 
				delete[] value;
				return ERROR_MORE_DATA;
			}
			if (lpType != NULL) {*lpType = REG_SZ;}
			_tcscpy((TCHAR *)lpData, value + 1);
			*lpcbData = bufLength - 1;
			break;
		case PREF_DWORD:
			if (*lpcbData < 4) {
				delete[] value;
				return ERROR_MORE_DATA;
			}
			if (lpType != NULL) {*lpType = REG_DWORD;}
			*(long *)lpData = atol(value + 1);
			*lpcbData = 4;
			break;
		default:
			delete[] value;
			return ERROR_INVALID_DATA;
	}
	delete[] value;
	return ERROR_SUCCESS;
}

LSTATUS IniFileRegistry::RegEnumValue(HKEY hKey,
								 DWORD dwIndex,
								 LPTSTR lpValueName,
								 LPDWORD lpcchValueName,
								 LPDWORD lpReserved,
								 LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);

	if (iter == hHive.end()) {
		return ERROR_PATH_NOT_FOUND;
	}

	char *line, *valueName, *valueData; 
	char values[32767]; // Max size of string
	int sizeValues = 32767;

	if ((sizeValues = GetPrivateProfileSection((*iter).second, values, sizeValues, fname)) == 0) {
		return ERROR_NO_MORE_ITEMS;
	}

	// Pointer to a necessary line 
	// // Not tested
	line = values;
	for (DWORD iLine = 0; (iLine < dwIndex) && (*line != '\0'); iLine++) {
		for (; *line != '\0'; line++) {} // Ponter to the next line
		line++;
	}
	if (*line == '\0') {
		// dwIndex out of range
		return ERROR_NO_MORE_ITEMS;
	}
	// End // Not tested

	// Copy value name to lpValueName
	int i = 0;
	for (valueName = line, *lpcchValueName = 0; (*valueName != '\0') && (*valueName != '='); valueName++, i++) {
		*(lpValueName + i) = *valueName;
		(*lpcchValueName)++;
	}
	*(lpValueName + i) = '\0';

	// Copying of data if it is necessary
	if (lpData != NULL) {
		// Pointer to the value data
		if ((valueData = strchr(line, '=')) != NULL) {
			valueData++;
			switch (valueData[0]) {
			case PREF_STRING:
				if (lpType != NULL) {*lpType = REG_SZ;}
				strcpy((char *)lpData, valueData + 1);
				*lpcbData = strlen(valueData + 1) + 1;
				break;
			case PREF_DWORD:
				if (lpType != NULL) {*lpType = REG_DWORD;}
				*(long *)lpData = atol(valueData + 1);
				*lpcbData = 4;
				break;
			default:
				return ERROR_INVALID_DATA;
			}
		}
	}
	return ERROR_SUCCESS;
}

LSTATUS IniFileRegistry::RegDeleteValue(HKEY hKey, LPCTSTR lpValueName)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);

	if (iter == hHive.end()) {
		return ERROR_INVALID_DATA;
	}

	if (WritePrivateProfileString((*iter).second, lpValueName, NULL, fname) == 0) {
		return ERROR_INVALID_DATA;
	}

	return ERROR_SUCCESS;
}

LSTATUS IniFileRegistry::RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	std::map<HKEY, TCHAR *>::iterator iter = hHive.find(hKey);

	// Delete of the section
	// Create new hive
	TCHAR *newHive;
	if (iter == hHive.end()) {
		newHive = new TCHAR[_tcslen(lpSubKey) + 1];
		_sntprintf(newHive, 10, _T("%x\\"), hKey);
	} else {
		newHive = new TCHAR[_tcslen((*iter).second) + _tcslen(lpSubKey) + 2]; // + 2 for '\' and '\0'
		_tcscpy(newHive, (*iter).second);
		if (lpSubKey != NULL) {
			_tcscat(newHive, _T("\\"));
		}
	}
	if (lpSubKey != NULL) {
		_tcscat(newHive, lpSubKey);
	}

	if (WritePrivateProfileString(newHive, NULL, NULL, fname) == 0) {
		return ERROR_INVALID_DATA;
	}

	return ERROR_SUCCESS;
}

LSTATUS IniFileRegistry::RegCloseKey(HKEY hKey)
{
	return 0;
}
