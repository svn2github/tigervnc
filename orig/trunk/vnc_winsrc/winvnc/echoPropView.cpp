//  Copyright (C) 2005 Dennis Syrovatsky. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
//  TightVNC homepage on the Web: http://www.tightvnc.com/

#include "stdio.h"

#include "echoPropView.h"

echoPropView::echoPropView(echoConCtrl *echoConCtrl, HWND hwnd)
{
	m_pEchoConCtrl = echoConCtrl;
	m_hwnd = hwnd;
	memset(&m_echoProps, 0, sizeof(ECHOPROP));
}

echoPropView::~echoPropView()
{
}


BOOL CALLBACK 
echoPropView::editParamsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	echoPropView *_this = (echoPropView *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, (LONG) lParam);
		_this = (echoPropView *)lParam;
		SetDlgItemText(hwnd, IDC_ECHO_SERVER, _this->m_echoProps.server);
		SetDlgItemText(hwnd, IDC_ECHO_USER, _this->m_echoProps.username);
		SetDlgItemText(hwnd, IDC_ECHO_PORT, _this->m_echoProps.port);
		SetDlgItemText(hwnd, IDC_ECHO_PWD, _this->m_echoProps.pwd);
		SendDlgItemMessage(hwnd, IDC_ECHO_AUTO_CONNECT, (UINT) BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
		return FALSE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
				GetDlgItemText(hwnd, IDC_ECHO_SERVER, _this->m_echoProps.server, ID_STRING_SIZE);
				GetDlgItemText(hwnd, IDC_ECHO_USER, _this->m_echoProps.username, ID_STRING_SIZE);
				GetDlgItemText(hwnd, IDC_ECHO_PORT, _this->m_echoProps.port, ID_STRING_SIZE);
				GetDlgItemText(hwnd, IDC_ECHO_PWD, _this->m_echoProps.pwd, ID_STRING_SIZE);
				if (IsDlgButtonChecked(hwnd, IDC_ECHO_AUTO_CONNECT) == BST_CHECKED) {
					_this->m_echoProps.connectionType = ID_ECHO_CONNECTION_TYPE_AUTO;
				} else {
					_this->m_echoProps.connectionType = ID_ECHO_CONNECTION_TYPE_NONE;
				}
				EndDialog(hwnd, IDOK);
				}
				return TRUE;
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				return TRUE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

void 
echoPropView::Apply()
{
}

void 
echoPropView::Init()
{
	RECT Rect;
	GetClientRect(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), &Rect);
	Rect.right -= GetSystemMetrics(SM_CXHSCROLL);
	int xwidth0 = (int) (0.51 * Rect.right);
	int xwidth1 = (int) (0.26 * Rect.right);
	int xwidth2 = (int) (0.23 * Rect.right);

	addColumn("user@server:port", 0, xwidth0, LVCFMT_LEFT);
	addColumn("Startup Type", 1, xwidth1, LVCFMT_CENTER);
	addColumn("Connected", 2, xwidth2, LVCFMT_CENTER);
	
	setExtendedLVStyle(LVS_EX_FULLROWSELECT);

	addLVItem(m_pEchoConCtrl->getNumEntries());
}

void 
echoPropView::Remove()
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);

	int selCount = ListView_GetSelectedCount(hListView);
	int selItem = ListView_GetSelectionMark(hListView);
	if ((selCount < 1) || (selItem < 0)) return;
	
	selItem = -1;
	selItem = ListView_GetNextItem(hListView, selItem, LVNI_SELECTED);
	do {
		ListView_DeleteItem(hListView, selItem);
		m_pEchoConCtrl->deleteAt(selItem);
		selItem = ListView_GetNextItem(hListView, selItem, LVNI_SELECTED);
	} while (selItem >= 0);
}

void 
echoPropView::Add()
{
	memset(&m_echoProps, 0, sizeof(ECHOPROP));

	bool bResult = true;
	do
	{
		INT_PTR result = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ECHO_PARAMS), 
										m_hwnd, (DLGPROC) editParamsDlgProc, (LONG) this);
		if ((result == IDOK) && (checkEchoParams(&m_echoProps))) {
			int result = m_pEchoConCtrl->add(&m_echoProps);
			if (result == 0) {
				addLVItem(1);
				bResult = false;
			} else {
				if (result == 1) 
					if (MessageBox(NULL, 
								   "This echo connection parameters already exists.\nDo you want edit parameters?", 
								   "Echo connection parameters exists", 
								   MB_YESNO) == IDYES) {
						bResult = true;
					} else {
						bResult = false;
					}
			}
		} else {
			bResult = false;
		}
	} while (bResult);
}

void 
echoPropView::Edit()
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);

	int selItem = ListView_GetSelectionMark(hListView);

	ECHOPROP echoProp;

	if (m_pEchoConCtrl->getEntriesAt(selItem, &echoProp)) {
		strcpy(m_echoProps.server, echoProp.server);
		strcpy(m_echoProps.port, echoProp.port);
		strcpy(m_echoProps.username, echoProp.username);
		strcpy(m_echoProps.pwd, echoProp.pwd);
		m_echoProps.connectionType = echoProp.connectionType;
	} else {
		return;
	}

	INT_PTR result = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ECHO_PARAMS), 
									m_hwnd, (DLGPROC) editParamsDlgProc, (LONG) this);

	if ((result == IDOK) && (checkEchoParams(&m_echoProps))) {
		m_pEchoConCtrl->change(&echoProp, &m_echoProps);
	}
	ListView_RedrawItems(hListView, 0, ListView_GetItemCount(hListView));
	UpdateWindow(hListView);
}

void 
echoPropView::addColumn(char *iText, int iOrder, int xWidth, int alignFmt)
{
	LVCOLUMN lvc; 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	lvc.fmt = alignFmt;
	lvc.iSubItem = iOrder;
	lvc.pszText = iText;	
	lvc.cchTextMax = 32;
	lvc.cx = xWidth;
	lvc.iOrder = iOrder;
	ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), iOrder, &lvc);
}

void
echoPropView::addLVItem(int num)
{
	for (int i = 0; i < num; i++) {
		LVITEM LVItem;
		LVItem.mask = LVIF_TEXT | LVIF_STATE; 
		LVItem.state = 0; 
		LVItem.stateMask = 0; 
		LVItem.iItem = 0;
		LVItem.iSubItem = 0;
		LVItem.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), &LVItem);
	}
}

void 
echoPropView::setExtendedLVStyle(DWORD styles)
{
	ListView_SetExtendedListViewStyleEx(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), styles, styles);
}

void 
echoPropView::onGetDispInfo(NMLVDISPINFO *pDI)
{
	ECHOPROP echoProps;
	if (!m_pEchoConCtrl->getEntriesAt(pDI->item.iItem, &echoProps)) return;
	
	switch (pDI->item.iSubItem)
	{
    case 0:
		{
			char buf[3*ID_STRING_SIZE];
			sprintf(buf, "%s@%s:%s", echoProps.username, echoProps.server, echoProps.port);
			pDI->item.pszText = buf;
		}
		break;
	case 1:
		if (echoProps.connectionType == ID_ECHO_CONNECTION_TYPE_AUTO) {
			pDI->item.pszText = "Auto";
		} else {
			pDI->item.pszText = "Disabled";
		}
		break;
	case 2:
		{
			int status = m_pEchoConCtrl->getConnectionStatus(&echoProps);
			if (status > 0) {
				pDI->item.pszText = "Yes";
			} else {
				pDI->item.pszText = "No";
			}
		}
		break;
	default:
		break;
	}
}

bool
echoPropView::checkEchoParams(ECHOPROP *echoProps)
{
	if ((strlen(echoProps->server) == 0) || 
		(strlen(echoProps->username) == 0) ||
		(strlen(echoProps->port) == 0) ||
		(strlen(echoProps->pwd) == 0)) return false;

	return true;
}

void 
echoPropView::DisableCheck()
{
	if (IsDlgButtonChecked(m_hwnd, IDC_ECHO_CON_DISABLE) == BST_CHECKED) {
		if (MessageBox(m_hwnd, 
					   "Some echo connections may be established.\nDo you really disable echo connection now?", 
					   "Echo connections", 
					   MB_YESNO) != IDYES) {
			CheckDlgButton(m_hwnd, IDC_ECHO_CON_DISABLE, BST_UNCHECKED);
		} else {
			enableWindows(FALSE);
			setCursor(IDC_WAIT);
			m_pEchoConCtrl->allowEchoConnection(false);
			setCursor(IDC_ARROW);
		}
	} else {
		enableWindows(TRUE);
		setCursor(IDC_WAIT);
		m_pEchoConCtrl->allowEchoConnection(true);
		setCursor(IDC_ARROW);
	}			
}

void
echoPropView::enableWindows(BOOL enable)
{
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ADD), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_REMOVE), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_EDIT), enable); 
}

void
echoPropView::setCursor(LPCTSTR cursor)
{
	HCURSOR hC = LoadCursor(NULL, cursor);
	SetCursor(hC);
}
