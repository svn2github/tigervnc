// QuitFullScreenWindow.h: interface for the QuitFullScreenWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUITFULLSCREENWINDOW_H__50F762D8_6192_4B9F_B08C_66AC74F4B626__INCLUDED_)
#define AFX_QUITFULLSCREENWINDOW_H__50F762D8_6192_4B9F_B08C_66AC74F4B626__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vncviewer.h"
#include "ClientConnection.h"

class QuitFullScreenWindow
{
public:
	QuitFullScreenWindow(VNCviewerApp *pApp, ClientConnection * CConn);
	void ShowButton(BOOL show);
	virtual ~QuitFullScreenWindow();
protected:
	static LRESULT CALLBACK QuitProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	
	VNCviewerApp *m_pApp;
	ClientConnection * m_CConn;
	HWND m_hwndButton;
	HWND m_hOldCap;
	POINT m_MousePoint;
	HDC m_hdcCompat;
	HBITMAP m_hbmp;
	BOOL m_ButtonDown;
	RECT m_rectOldCur;
};

#endif // !defined(AFX_QUITFULLSCREENWINDOW_H__50F762D8_6192_4B9F_B08C_66AC74F4B626__INCLUDED_)
