// ConnectionsAccess.h: interface for the ConnectionsAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTIONSACCESS_H__1FBA6287_0A9F_4441_A018_1949A5C7A9EE__INCLUDED_)
#define AFX_CONNECTIONSACCESS_H__1FBA6287_0A9F_4441_A018_1949A5C7A9EE__INCLUDED_

#include "resource.h"
#include "vncServer.h"
#include "commctrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ConnectionsAccess  
{
public:
	ConnectionsAccess(vncServer* server, HWND hwnd);
	static BOOL CALLBACK EditDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Apply();
	void Init();
	void MoveUp();
	void MoveDown();
	void Remove();
	void Add();
	void Edit();
	virtual ~ConnectionsAccess();
protected:
	DWORD DoEditDialog();
    BOOL InsertListViewItem(int Numbe, TCHAR ItemString[3][256]);
	void GetListViewItem(int Numbe, TCHAR ItemString[3][256]);
    BOOL InitListViewColumns();
	int	GetSelectedItem();
	void SetSelectedItem(int number);
	void DeleteItem(int number);
	int GetItemCount();

	HWND m_hwnd;
    vncServer* m_server;
	TCHAR ItemString[3][256];
	BOOL m_edit;
	HWND m_hwnd_edit_dialog;
};

#endif // !defined(AFX_CONNECTIONSACCESS_H__1FBA6287_0A9F_4441_A018_1949A5C7A9EE__INCLUDED_)
