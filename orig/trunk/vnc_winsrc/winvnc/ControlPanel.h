// ControlPanel.h: interface for the ControlPanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLPANEL_H__3F8F2C78_5C46_4A06_96B4_66EF1D01F5C7__INCLUDED_)
#define AFX_CONTROLPANEL_H__3F8F2C78_5C46_4A06_96B4_66EF1D01F5C7__INCLUDED_

#include "resource.h"
#include "vncServer.h"
#include "commctrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ControlPanel  
{
public:
	ControlPanel(vncServer* server, HWND hwndMenu);
	virtual ~ControlPanel();
	static BOOL CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void UpdateListView();
	bool showDialog();
    void initDialog();
	void setDisable();
protected:
    inline void Enable(int id, BOOL enable) {
		EnableWindow(GetDlgItem(m_hwnd, id), enable);
	}
	inline void SetChecked(int id, BOOL checked) {
		SendDlgItemMessage(m_hwnd, id, BM_SETCHECK, checked, 0);
	}
	inline BOOL IsChecked(int id) {
		return (SendDlgItemMessage(m_hwnd, id, BM_GETCHECK, 0, 0) == BST_CHECKED);
	}
    void getSelectedConn(vncClientList* selconn);
    BOOL InsertListViewItem(int Numbe, TCHAR ItemString[3][80]);
    BOOL InitListViewColumns();

	HWND m_hwnd, m_hwndMenu;
    vncServer* m_server;
    vncClientList m_clients;
};

#endif // !defined(AFX_CONTROLPANEL_H__3F8F2C78_5C46_4A06_96B4_66EF1D01F5C7__INCLUDED_)
