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

#include "AppCloserWindow.h"

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
								WPARAM wParam, LPARAM lParam)
{
	AppCloserWindow *_this;
	_this = (AppCloserWindow *) GetWindowLong(hWnd, GWL_USERDATA);
	if (_this != NULL) {
		if (_this->WndProc(message, wParam, lParam)) {
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

AppCloserWindow::AppCloserWindow(HINSTANCE hInst, const TCHAR *windowClassName, const TCHAR *msgName)
: m_closeMsg(0), m_hwnd(0)
{
	if (msgName == NULL || windowClassName == NULL) {
		return;
	}

	// Create global window message for TightVNC control
	m_closeMsg = RegisterWindowMessage(msgName);

	if (RegClass(hInst, windowClassName) == 0) {
		return;
	}
	m_hwnd = CreateWindow(windowClassName, "", 0, 0, 0, 1, 1, NULL, NULL, hInst, NULL);
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);
}

AppCloserWindow::~AppCloserWindow(void)
{
}

ATOM AppCloserWindow::RegClass(HINSTANCE hInst, LPCSTR lpzClassName)
{
	WNDCLASS wcWindowClass;
	memset(&wcWindowClass, 0, sizeof(wcWindowClass));

	wcWindowClass.lpfnWndProc = ::WndProc;
	wcWindowClass.hInstance = hInst;
	wcWindowClass.lpszClassName = lpzClassName;

	return RegisterClass(&wcWindowClass);
}

bool AppCloserWindow::WndProc(UINT message, WPARAM wParam, LPARAM lParam) const
{
	if (message == m_closeMsg) {
		PostQuitMessage(0);
		return true;
	}
	return false;
}
