// SharedDesktopArea.h: interface for the SharedDesktopArea class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SHAREDDESKTOPAREA_H__
#define SHAREDDESKTOPAREA_H__

class SharedDesktopArea;

#include "resource.h"

#include "WinVNC.h"
#include "vncServer.h"
#include "vncProperties.h"
#include "MatchWindow.h"

class SharedDesktopArea  
{
public:
	SharedDesktopArea(HWND hwnd, CMatchWindow *matchwindow,
					  vncProperties *vncprop, vncServer *server);
	bool ApplySharedControls();
	void FullScreen();
	void SharedWindow();
	void SharedScreen();
	virtual ~SharedDesktopArea();

protected:
	void SetWindowCaption(HWND hWnd);
	HWND m_hWindowName;

private:
	void Init();
	void SetupMatchWindow();
	static void DrawFrameAroundWindow(HWND hWnd);
	static LRESULT CALLBACK BmpWndProc(HWND, UINT, WPARAM, LPARAM);

	HWND m_hwnd;
	LONG m_OldBmpWndProc;
	BOOL m_bCaptured;
	HWND m_KeepHandle;
	vncServer *m_server;
	vncProperties *m_vncprop;

	CMatchWindow *m_pMatchWindow;
	bool m_deleteMatchWindow;
};

#endif // SHAREDDESKTOPAREA_H__
