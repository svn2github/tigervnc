//  Copyright (C) 2003-2004 Dennis Syrovatsky. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
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
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

#include "windows.h"
#include "stdio.h"

#include "FTStatusBox.h"
#include "FileTransferTypes.h"

const char FTStatusBox::delimeter[] = "...";

FTStatusBox::FTStatusBox(HWND statusBox)
{
	m_hStatusBox = statusBox;
	m_dwNumStrings = 0;
}

FTStatusBox::~FTStatusBox()
{
}

bool 
FTStatusBox::setText(char *pText)
{
	LRESULT lRes = SendMessage(m_hStatusBox, (UINT) CB_INSERTSTRING, (WPARAM) 0, (LPARAM) pText);
	
	SetWindowText(m_hStatusBox, pText);

	m_dwNumStrings++;
	if (m_dwNumStrings > FT_MAX_STATUS_STRINGS) {
		int numItems = SendMessage(m_hStatusBox, (UINT) CB_GETCOUNT, (WPARAM) 0, (LPARAM) 0); 
		if (numItems != CB_ERR) {
			m_dwNumStrings--;
			SendMessage(m_hStatusBox, (UINT) CB_DELETESTRING, (WPARAM) numItems - 1, (LPARAM) 0); 
		}
	}

	if ((lRes == CB_ERR) || (lRes == CB_ERRSPACE))
		return false;
	else 
		return true;
}

bool 
FTStatusBox::setStatusText(LPCSTR format,...)
{
	char text[1024];
	va_list args;
	va_start(args, format);
	int nSize = _vsnprintf(text, sizeof(text), format, args);
	return setText(text);
}

/*
bool 
FTStatusBox::makeStatusText(char *prefix, char *path1, char *path2, char *name1, char *name2)
{
	int length = strlen(prefix) + strlen(path1) + strlen(path2) + strlen(name1) + strlen(name2) + 7;
	if (length > 120) {
		char buf[MAX_PATH];
		char _name1[MAX_PATH]; strcpy(_name1, name1);
		char _name2[MAX_PATH]; strcpy(_name2, name2);
		char _path1[MAX_PATH]; strcpy(_path1, path1);
		char _path2[MAX_PATH]; strcpy(_path2, path2);
		strcpy(buf, name);
		for (int i = strlen(buf) - 2; i > 0; i--) {
			if (buf[i] == '\\') {
				strcpy(_name, &buf[i + 1]);
				buf[i] = '\0';
				sprintf(_path1, "%s\\%s", _path1, buf);
				sprintf(_path2, "%s\\%s", _path2, buf);
				break;
			}
		}
		char *path;
		do {
			if (strlen(_path1) > strlen(_path2)) {
				path = _path1;
			} else {
				path = _path2;
			}
			buf[0] = '\0';
			for (int i = strlen(path) - 1, j = 0; i > 0; i--, j++) {
				if ((path[i] == '\\') && (strcmp(buf, delimeter) != 0)) {
					path[i + 1] = '\0';
					strcat(path, delimeter);
					break;
				}
				buf[j] = path[i];
				buf[j + 1] = '\0';
			}
			length = strlen(prefix) + strlen(path1) + strlen(path2) + strlen(name1) + strlen(name2) + 13;
			if ((strlen(_path1) < 7) && (strlen(_path2) < 7)) break;
		} while(length > 120);
		return setStatusText("%s %s\\%s to %s\\%s", prefix, _path1, _name, _path2, _name);
	} else {
		return setStatusText("%s %s\\%s to %s\\%s", prefix, path1, name, path2, name);
	}
}
*/

bool 
FTStatusBox::makeStatusText(char *pPrefix, char *pPath1, char *pPath2, char *pName1, char *pName2)
{
	int len = strlen(pPrefix) + strlen(pPath1) + strlen(pPath2) + strlen(pName1) + strlen(pName2);
	if (len >= FT_MAX_LENGTH_STATUS_STRINGS) {
		char _path1[MAX_PATH]; strcpy(_path1, pPath1);
		char _path2[MAX_PATH]; strcpy(_path2, pPath2);
		char *pPath;

		do {
			if (strlen(_path1) > strlen(_path2)) pPath = _path1; else pPath = _path2;
			for (int i = strlen(pPath) - 2; i > 0; i--) {
				if ((pPath[i] == '\\') && (strcmp(&pPath[i + 1], delimeter) != 0)) {
					pPath[i + 1] = '\0';
					strcat(pPath, delimeter);
					break;
				}
			}
			if (strlen(pPath) < 7) break;
			len = strlen(pPrefix) + strlen(_path1) + strlen(_path2) + 
				  strlen(pName1) + strlen(pName2) + strlen(delimeter);
		} while (len > FT_MAX_LENGTH_STATUS_STRINGS);

		return setStatusText("%s %s\\%s to %s\\%s", pPrefix, _path1, pName1, _path2, pName2);
	} else {
		return setStatusText("%s %s\\%s to %s\\%s", pPrefix, pPath1, pName1, pPath2, pName2);
	}
}

bool 
FTStatusBox::makeStatusText(char *pPrefix, char *pPath, char *pName)
{
	int len = strlen(pPrefix) + strlen(pPath) + strlen(pName);
	if (len >= FT_MAX_LENGTH_STATUS_STRINGS) {
		char _path[MAX_PATH]; strcpy(_path, pPath);
		do {
			for (int i = strlen(_path) - 2; i > 0; i--) {
				if ((_path[i] == '\\') && (strcmp(&_path[i + 1], delimeter) != 0)) {
					_path[i + 1] = '\0';
					strcat(_path, delimeter);
					break;
				}
			}
			if (strlen(_path) < 7) break;
			len = strlen(pPrefix) + strlen(_path) + strlen(pName) + strlen(delimeter);
		} while (len > FT_MAX_LENGTH_STATUS_STRINGS);

		return setStatusText("%s %s\\%s", pPrefix, _path, pName);
	} else {
		return setStatusText("%s %s\\%s", pPrefix, pPath, pName);
	}
}