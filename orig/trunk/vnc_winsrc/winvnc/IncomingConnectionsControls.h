// IncomingConnectionsControls.h: interface for the IncomingConnectionsControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_INCOMINGCONNECTIONSCONTROLS_H__
#define AFX_INCOMINGCONNECTIONSCONTROLS_H__

#pragma once

#include "resource.h"
#include "vncServer.h"

class IncomingConnectionsControls  
{
public:
	IncomingConnectionsControls(HWND hwnd, vncServer *server);
	void Validate(BOOL InitApply);
	void Apply();
	void Init();
	void InitPortSettings(BOOL CheckedButton);
	BOOL SetPasswordSettings(DWORD idEditBox);
	virtual ~IncomingConnectionsControls();	

private:
	inline void Enable(int id, BOOL enable) {
		EnableWindow(GetDlgItem(m_hwnd, id), enable);
	}
	inline void SetChecked(int id, BOOL checked) {
		SendDlgItemMessage(m_hwnd, id, BM_SETCHECK, checked, 0);
	}
	inline BOOL IsChecked(int id) {
		return (SendDlgItemMessage(m_hwnd, id, BM_GETCHECK, 0, 0) == BST_CHECKED);
	}
  vncServer * m_server;
  HWND m_hwnd;
};

#endif 
