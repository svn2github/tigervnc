// PollControls.h: interface for the PollControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef POLLCONTROLS_H__
#define POLLCONTROLS_H__

#pragma once

#include "resource.h"
#include "vncServer.h"

class PollControls  
{
public:
	PollControls(HWND hwnd, vncServer *server);
	void Validate();
	void Apply();	

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

	HWND m_hwnd;
	vncServer *m_server;
};

#endif 
