// SharedDesktopArea.h: interface for the SharedDesktopArea class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SHAREDDESKTOPAREA_H__
#define SHAREDDESKTOPAREA_H__

#include "resource.h"
#include "vncServer.h"
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
	HWND hNameAppli;
private:
	void Init();
	static void DrawFrameAroundWindow(HWND hWnd);
	static LRESULT CALLBACK BmpWndProc(HWND, UINT, WPARAM, LPARAM);

	HWND m_hwnd;
	LONG m_OldBmpWndProc;
	BOOL m_bCaptured;
	HWND m_KeepHandle;
	CMatchWindow *m_pMatchWindow;
	vncServer *m_server;
	vncProperties *m_vncprop;
};

#endif 
