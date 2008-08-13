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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam)
{
	AppCloserWindow *_this;
	_this = (AppCloserWindow *) GetWindowLong(hWnd, GWL_USERDATA);
	if (_this != NULL) {
		if (_this->WndProcForm(_this, message, wParam, lParam)) {
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

AppCloserWindow::AppCloserWindow(HINSTANCE hInst, TCHAR *WindowClassName)
{
	// Create global window message for TightVNC control
	wm_ExitCode = RegisterWindowMessage(_T(GLOBAL_REG_MESSAGE));

	f_hinst = hInst;
	if (RegClass(f_hinst, WindowClassName) == 0) {
		return;
	}
	f_hwnd = CreateWindow(WindowClassName, "", 
		0, 0, 0, 1, 1, NULL, NULL, f_hinst, NULL);
	SetWindowLong(f_hwnd, GWL_USERDATA, (LONG) this);
}


AppCloserWindow::~AppCloserWindow(void)
{

}

ATOM AppCloserWindow::RegClass(HINSTANCE hInst, LPSTR lpzClassName)
{
	WNDCLASS wcWindowClass = {0};
	wcWindowClass.lpfnWndProc = WndProc;
	wcWindowClass.style = NULL; 
	wcWindowClass.hInstance = hInst; 
	wcWindowClass.lpszClassName = lpzClassName; 
	wcWindowClass.hCursor = NULL; 
	wcWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;  
	//
	return RegisterClass(&wcWindowClass);
}

bool AppCloserWindow::WndProcForm(AppCloserWindow *_this, UINT message,
							 WPARAM wParam, LPARAM lParam)
{
	if (message == wm_ExitCode) {
		PostQuitMessage(0);
		return true;
	}
	return false;
}
