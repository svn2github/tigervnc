// AdministrationControls.h: interface for the AdministrationControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_ADMINISTRATIONCONTROLS_H__
#define AFX_ADMINISTRATIONCONTROLS_H__


#pragma once

#include "resource.h"
#include "vncServer.h"

class AdministrationControls  
{
public:
	AdministrationControls(HWND hwnd, vncServer * server);
	void Validate();
	void Apply();
	void Init();
	virtual ~AdministrationControls();
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
