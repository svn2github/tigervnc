//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//
// Many thanks to Greg Hewgill <greg@hewgill.com> for providing the basis for 
// the full-screen mode.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"

// Parameters for scrolling in full screen mode
#define BUMPSCROLLBORDER 4
#define BUMPSCROLLAMOUNTX 16
#define BUMPSCROLLAMOUNTY 4

bool ClientConnection::InFullScreenMode() 
{
	return m_opts.m_FullScreen; 
};

// You can explicitly change mode by calling this
void ClientConnection::SetFullScreenMode(bool enable)
{	
	m_opts.m_FullScreen = enable;
	RealiseFullScreenMode(false);
}

// If the options have been changed other than by calling 
// SetFullScreenMode, you need to call this to make it happen.
void ClientConnection::RealiseFullScreenMode(bool suppressPrompt)
{
	LONG style = GetWindowLong(m_hwnd1, GWL_STYLE);
	if (m_opts.m_FullScreen) {
		if (!suppressPrompt && !pApp->m_options.m_skipprompt) {
			MessageBox(m_hwnd1, 
				_T("To exit from full-screen mode, use Ctrl-Esc Esc and then\r\n"
				"right-click on the vncviewer taskbar icon to see the menu."),
				_T("VNCviewer full-screen mode"),
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);
		}
		ShowWindow(m_hToolbar, SW_HIDE);
		EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR, MF_BYCOMMAND|MF_GRAYED);
		ShowWindow(m_hwnd1, SW_MAXIMIZE);
		style = GetWindowLong(m_hwnd1, GWL_STYLE);
		style &= ~(WS_DLGFRAME | WS_THICKFRAME);
		
		SetWindowLong(m_hwnd1, GWL_STYLE, style);
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(m_hwnd1, HWND_TOPMOST, -1, -1, cx + 3, cy + 3, SWP_FRAMECHANGED);
		CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN, MF_BYCOMMAND|MF_CHECKED);
		m_DisButton->ShowButton(TRUE);
	} else {
		m_DisButton->ShowButton(FALSE);
		ShowWindow(m_hToolbar, SW_SHOW);
		EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR, MF_BYCOMMAND|MF_ENABLED);
		style |= (WS_DLGFRAME | WS_THICKFRAME);
		
		SetWindowLong(m_hwnd1, GWL_STYLE, style);
		SetWindowPos(m_hwnd1, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(m_hwnd1, SW_NORMAL);		
		CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN, MF_BYCOMMAND|MF_UNCHECKED);
	}
}

bool ClientConnection::BumpScroll(int x, int y)
{
	int dx = 0;
	int dy = 0;
	int rightborder;
	int bottomborder;
	if (!InFullScreenMode()) {
		RECT rect;
		GetClientRect(m_hwnd, &rect);
		rightborder	= rect.right-BUMPSCROLLBORDER;
		bottomborder = rect.bottom-BUMPSCROLLBORDER;
	} else {
		rightborder	= GetSystemMetrics(SM_CXSCREEN)-BUMPSCROLLBORDER;
		bottomborder = GetSystemMetrics(SM_CYSCREEN)-BUMPSCROLLBORDER;
	}
	if (x < BUMPSCROLLBORDER)
		dx = -BUMPSCROLLAMOUNTX * m_opts.m_scale_num / m_opts.m_scale_den;
	if (x >= rightborder)
		dx = +BUMPSCROLLAMOUNTX * m_opts.m_scale_num / m_opts.m_scale_den;;
	if (y < BUMPSCROLLBORDER)
		dy = -BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;;
	if (y >= bottomborder)
		dy = +BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;;
	if (dx || dy) {
		if (ScrollScreen(dx,dy)) {
			// If we haven't physically moved the cursor, artificially
			// generate another mouse event so we keep scrolling.
			POINT p;
			GetCursorPos(&p);			
			ScreenToClient(m_hwnd, &p);			
			if (p.x == x && p.y == y) {
				ClientToScreen(m_hwnd, &p);
				SetCursorPos(p.x,p.y);
			}
			return true;
		} 
	}
	return false;
}

//Methods of DisableButton class.
DisableButton::DisableButton(VNCviewerApp *pApp, ClientConnection * CConn)
{
	m_pApp = pApp;
	m_CConn = CConn;
	m_ButtonDown = FALSE;
	m_hOldCap = NULL;

	//Creating button window.
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)DisableButton::DisableProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "DisableButtonClass";
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	m_hwndButton = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,			//dwExStyle
		"DisableButtonClass",		// pointer to registered class name
  		"Disable Button",			// pointer to window name
  		WS_POPUP,					// window style
		12,							// horizontal position of window
		13,							// vertical position of window
		24,							// window width
		24,							// window height
		NULL,						// handle to parent or owner window
		NULL,						// handle to menu, or child-window identifier
		NULL,						// handle to application instance
		NULL						// pointer to window-creation data
	);
  	
	SetWindowLong(m_hwndButton, GWL_USERDATA, (LONG)this);
	ShowButton(FALSE);

	// Load the bitmap resource. 	
	m_hbmp = LoadBitmap(m_pApp->m_instance, MAKEINTRESOURCE(IDB_DISABLE_FS));
	
	// Create a device context (DC) to hold the bitmap. 
	// The bitmap is copied from this DC to the window's DC 
	// whenever it must be drawn. 	
	HDC hdc = GetDC(m_hwndButton); 
	m_hdcCompat = CreateCompatibleDC(hdc);
	SelectObject(m_hdcCompat, m_hbmp);
	ReleaseDC(m_hwndButton, hdc);
}

void DisableButton::ShowButton(BOOL show)
{
	if (show) {
		ShowWindow(m_hwndButton, SW_SHOWNA);
	} else {
		ShowWindow(m_hwndButton, SW_HIDE);
	}
}

LRESULT CALLBACK DisableButton::DisableProc(HWND hwnd, UINT iMsg,
											WPARAM wParam, LPARAM lParam)
{
	DisableButton *_this = (DisableButton*)GetWindowLong(hwnd, GWL_USERDATA);

	switch (iMsg) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			BitBlt(ps.hdc, 0, 0, 24, 24, _this->m_hdcCompat, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINTS ptsMousePoint = MAKEPOINTS(lParam);
			POINT ptMousePoint;
			ptMousePoint.x = ptsMousePoint.x;
			ptMousePoint.y = ptsMousePoint.y;
			ClientToScreen(hwnd, &ptMousePoint);
			RECT wrect;
			GetWindowRect(hwnd, &wrect);
			HWND hOld = GetCapture();
			if (hOld != hwnd) {
				_this->m_hOldCap = hOld;
				GetClipCursor(&_this->m_rectOldCur);
				SetCapture(hwnd);
			} else if (!PtInRect(&wrect, ptMousePoint)) {
				SetCapture(_this->m_hOldCap);
				ClipCursor(&_this->m_rectOldCur);
			}

			if (_this->m_ButtonDown) {
				POINT newpos;
				newpos.x = wrect.left - (_this->m_MousePoint.x - ptMousePoint.x);
				newpos.y = wrect.top - (_this->m_MousePoint.y - ptMousePoint.y);
				if (newpos.x != wrect.left || newpos.y != wrect.top)
					MoveWindow(hwnd,newpos.x, newpos.y, 24,24, TRUE);
				_this->m_MousePoint.x = ptMousePoint.x;
				_this->m_MousePoint.y = ptMousePoint.y;
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		_this->m_CConn->SetFullScreenMode(false);
		break;
	case WM_LBUTTONDOWN:
		_this->m_ButtonDown = TRUE;
		POINTS ptsMousePoint = MAKEPOINTS(lParam);
		_this->m_MousePoint.x = ptsMousePoint.x;
		_this->m_MousePoint.y = ptsMousePoint.y;
		ClientToScreen(hwnd, &_this->m_MousePoint);
		break;
	case WM_LBUTTONUP:
		_this->m_ButtonDown = FALSE;
		break;
	case WM_DESTROY: 
		DeleteDC(_this->m_hdcCompat);
		DeleteObject(_this->m_hbmp);
		break;
	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}

DisableButton::~DisableButton()
{
	DestroyWindow(m_hwndButton);
}
