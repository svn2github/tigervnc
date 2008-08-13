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

#ifndef __APPCLOSERWINDOW_H__
#define __APPCLOSERWINDOW_H__

#include "stdhdrs.h"

class AppCloserWindow
{
public:
	AppCloserWindow(HINSTANCE hInst, const TCHAR *windowClassName, const TCHAR *msgName);
	~AppCloserWindow();

	HWND getWindow() const { return m_hwnd; }

	bool WndProc(UINT message, WPARAM wParam, LPARAM lParam) const;

protected:
	static ATOM RegClass(HINSTANCE hInst, LPCSTR lpzClassName);

	UINT m_closeMsg;
	HWND m_hwnd;
};

#endif // __APPCLOSERWINDOW_H__
