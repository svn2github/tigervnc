// SharedDesktopArea.h: interface for the SharedDesktopArea class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SHAREDDESKTOPAREA_H__
#define SHAREDDESKTOPAREA_H__


#pragma once



#include "resource.h"
#include "vncServer.h"
#include "MatchWindow.h"

class SharedDesktopArea  
{
public:
	SharedDesktopArea(HWND hwnd, CMatchWindow *Matchwindow,
						vncProperties *vncprop, vncServer *server);
	bool ApplySharedControls(HWND hwnd);
	void FullScreen(HWND hwnd);
	void SharedWindow(HWND hwnd);
	void SharedScreen(HWND hwnd);
	virtual ~SharedDesktopArea();
protected:
	void SetWindowCaption(HWND hWnd);
	HWND hNameAppli;
private:
	void Init(HWND hwnd);
	static  void DrawFrameAroundWindow(HWND hWnd);
	static  LRESULT CALLBACK BmpWndProc(HWND, UINT, WPARAM, LPARAM);
	LONG m_OldBmpWndProc;
	BOOL m_bCaptured;
	HWND m_KeepHandle;
	CMatchWindow * m_pMatchWindow;
	vncServer *		m_server;
	vncProperties * m_vncprop;
};

#endif 
