// Copyright (C) 2004 TightVNC Development Team. All Rights Reserved.
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

// ConnectionsAccess.h: interface for the ConnectionsAccess class.


#if !defined(AFX_CONNECTIONSACCESS_H__1FBA6287_0A9F_4441_A018_1949A5C7A9EE__INCLUDED_)
#define AFX_CONNECTIONSACCESS_H__1FBA6287_0A9F_4441_A018_1949A5C7A9EE__INCLUDED_

#include "resource.h"
#include "vncServer.h"
#include "commctrl.h"

#pragma once

class ConnectionsAccess  
{
public:
	ConnectionsAccess(vncServer * server, HWND hwnd);
	static BOOL CALLBACK EditDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Apply();
	void Init();
	void MoveUp();
	void MoveDown();
	void Remove();
	void Add();
	void Edit();
	virtual ~ConnectionsAccess();
protected:
	void MatchEdit(HWND hwnd, DWORD idedit);
	BOOL MatchPatternComponent(TCHAR component[5]);
	BOOL FormatPattern(BOOL toList, TCHAR strpattern[256], 
						char buf_parts[4][5]);
	DWORD DoEditDialog();
    BOOL InsertListViewItem(int Numbe, TCHAR ItemString[2][256]);
	void GetListViewItem(int Numbe, TCHAR ItemString[2][256]);
    BOOL InitListViewColumns();
	int	GetSelectedItem();
	void SetSelectedItem(int number);
	void DeleteItem(int number);
	int GetItemCount();

	HWND m_hwnd;
    vncServer * m_server;
	TCHAR ItemString[2][256];
	char IPComponent[4][5];
	BOOL m_edit;
};

#endif
