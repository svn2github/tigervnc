// ConnectionsAccess.cpp: implementation of the ConnectionsAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "ConnectionsAccess.h"
#include "WinVNC.h"
#include "VNCHelp.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConnectionsAccess::ConnectionsAccess(vncServer* server, HWND hwnd)
{
	m_server = server;
	m_hwnd = hwnd;
	m_hwnd_edit_dialog = NULL;
	Init();
}

void ConnectionsAccess::Apply()
{
	int count = GetItemCount();
	if (count < 1) {
		m_server->SetAuthHosts(0);
		return;
	}
	char *authosts = m_server->AuthHosts();
	for (int i = 0; i < count; i++) {
		GetListViewItem(i, ItemString);
		if (strcmp(ItemString[2], "Allow") == 0) {
			if (i == 0) {
				strcpy(authosts, "+");
			} else {
				strcat(authosts, "+");
			}
		}
		if (strcmp(ItemString[2], "Deny") == 0) {
			if (i == 0) {
				strcpy(authosts, "-");
			} else {
				strcat(authosts, "-");
			}
		}
		if (strcmp(ItemString[2], "Query") == 0) {
			if (i == 0) {
				strcpy(authosts, "?");
			} else {
				strcat(authosts, "?");
			}
		}
		strcat(authosts, ItemString[0]);
	}
	m_server->SetAuthHosts(authosts);
}
void ConnectionsAccess::Init()
{
	InitListViewColumns();
	char *authosts = strdup(m_server->AuthHosts());
	strcpy(ItemString[1],"");
	int authHostsPos = 0;
	int startPattern = 0;
	int number = 0;
	if (authosts) {
		while (1) {
			if ((authosts[authHostsPos] == ':') ||
				(authosts[authHostsPos] == '\0')) {
				ItemString[0][authHostsPos - startPattern] = '\0';
				InsertListViewItem(number, ItemString);
				break;
			}
			if (authosts[authHostsPos] == '+') {				
				ItemString[0][authHostsPos - startPattern] = '\0';
				if (authHostsPos != 0) {
					InsertListViewItem(number, ItemString);
					number++;
				}
				strcpy(ItemString[2], "Allow");
				authHostsPos++;
				startPattern = authHostsPos;
				continue;
			}
			if (authosts[authHostsPos] == '-') {
				if (authHostsPos != 0) {
					ItemString[0][authHostsPos - startPattern] = '\0';
					InsertListViewItem(number, ItemString);
					number++;
				}
				strcpy(ItemString[2], "Deny");
				authHostsPos++;
				startPattern = authHostsPos;
				continue;
			}
			if (authosts[authHostsPos] == '?') {
				if (authHostsPos != 0) {
					ItemString[0][authHostsPos - startPattern] = '\0';
					InsertListViewItem(number, ItemString);
					number++;
				}
				strcpy(ItemString[2], "Query");
				authHostsPos++;
				startPattern = authHostsPos;
				continue;
			}
			
			ItemString[0][authHostsPos - startPattern] = authosts[authHostsPos];
			authHostsPos++;
			 
		}
	}
}
void ConnectionsAccess::MoveUp()
{
	if (m_hwnd_edit_dialog != NULL) 
		return;
	int number = GetSelectedItem();
	if (number <= 0)
		return;
	GetListViewItem(number, ItemString);
	InsertListViewItem(number - 1, ItemString);
	DeleteItem(number + 1);
	SetSelectedItem(number - 1);
}
void ConnectionsAccess::MoveDown()
{
	if (m_hwnd_edit_dialog != NULL) 
		return;
	int number = GetSelectedItem();
	if (number == -1 || number == (GetItemCount() - 1))
		return;
	GetListViewItem(number, ItemString);
	InsertListViewItem(number + 2, ItemString);
	DeleteItem(number);
	SetSelectedItem(number + 1);
}
void ConnectionsAccess::Remove()
{
	if (m_hwnd_edit_dialog != NULL) 
		return;
	int number = GetSelectedItem();
	if (number == -1)
		return;
	DeleteItem(number);
}
void ConnectionsAccess::Add()
{
	if (m_hwnd_edit_dialog != NULL) {
		SetForegroundWindow(m_hwnd_edit_dialog);
		return;
	}
	m_edit = FALSE;
	if (DoEditDialog() != IDOK)
		return;
	InsertListViewItem(0, ItemString);
	SetSelectedItem(0);
}
void ConnectionsAccess::Edit()
{
	if (m_hwnd_edit_dialog != NULL) {
		SetForegroundWindow(m_hwnd_edit_dialog);
		return;
	}

	int numbersel = GetSelectedItem();
	if (numbersel == -1)
		return;
	GetListViewItem(numbersel, ItemString);
	m_edit = TRUE;
	if (DoEditDialog() != IDOK)
		return;
	DeleteItem(numbersel);
	InsertListViewItem(numbersel, ItemString);
	SetSelectedItem(numbersel);
}

DWORD ConnectionsAccess::DoEditDialog()
{
	//if m_edit_dialog_visible
	return DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_CONN_HOST), 
		NULL, (DLGPROC) EditDlgProc, (LONG) this);
}
int ConnectionsAccess::GetItemCount()
{
	return ListView_GetItemCount(GetDlgItem(m_hwnd, IDC_LIST_HOSTS));
}
void ConnectionsAccess::SetSelectedItem(int number)
{
	ListView_SetItemState(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
				number, LVIS_SELECTED, LVIS_SELECTED);
}
void ConnectionsAccess::DeleteItem(int number)
{
	ListView_DeleteItem(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), number);
}
int ConnectionsAccess::GetSelectedItem()
{
	int count = ListView_GetItemCount(GetDlgItem(m_hwnd, IDC_LIST_HOSTS));
	if (count < 1)
		return -1;
	int numbersel = -1;
	for (int i = 0; i < count; i++) {
		if (ListView_GetItemState(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
									i, LVIS_SELECTED) == LVIS_SELECTED)
			numbersel = i;
	}
		return numbersel;
}
void ConnectionsAccess::GetListViewItem(int Numbe, TCHAR ItemString[3][256])
{
	for (int i =0;i < 3; i++) {
		ListView_GetItemText(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
							Numbe, i, ItemString[i], 256);							
	}
}

BOOL ConnectionsAccess::InsertListViewItem(int Numbe, TCHAR ItemString[3][256])
{
	int i;
	LVITEM lvI;
	lvI.mask = LVIF_TEXT| LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = Numbe; 
	lvI.iSubItem = 0; 
	lvI.pszText = ItemString[0]; 									  
	
	if(ListView_InsertItem(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), &lvI) == -1)
		return NULL;
	
	for (i =1;i < 3; i++) {	
		ListView_SetItemText(
			GetDlgItem(m_hwnd, IDC_LIST_HOSTS), 
			Numbe, i, ItemString[i]);
	}
	return TRUE;
}

BOOL ConnectionsAccess::InitListViewColumns() 
{ 
	ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), 
		LVS_EX_FULLROWSELECT);
	char szText[256];      
	LVCOLUMN lvc; 
	int iCol;
    
	TCHAR *ColumnsStrings[] = {
		"IP Adress Pattern",
		"Mask",
		"Access Status"
	};
	
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	
	for (iCol = 0; iCol < 3; iCol++) { 
		lvc.iSubItem = iCol;
		lvc.pszText = szText;	
		lvc.cx = 97;           
		lvc.fmt = LVCFMT_LEFT;
		
		strcpy(szText, ColumnsStrings[iCol]); 
		if (ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), iCol, &lvc) == -1) 
			return FALSE; 
	} 
	return TRUE; 
}

BOOL CALLBACK ConnectionsAccess::EditDlgProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	ConnectionsAccess *_this = (ConnectionsAccess *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			ConnectionsAccess *_this = (ConnectionsAccess *) lParam;
			_this->m_hwnd_edit_dialog = hwnd;
			if (_this->m_edit) {
				SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_SETCHECK,
					(strcmp(_this->ItemString[2], "Allow") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_SETCHECK,
					(strcmp(_this->ItemString[2], "Deny") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_SETCHECK,
					(strcmp(_this->ItemString[2], "Query") == 0), 0);
				SetDlgItemText(hwnd, IDC_HOST_PATTERN, _this->ItemString[0]);
			} else {
				SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_SETCHECK, TRUE, 0);
			}
			return FALSE;
		}
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd, IDC_HOST_PATTERN, _this->ItemString[0], 80);
			if (SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[2], "Allow");
			if (SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[2], "Deny");
			if (SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[2], "Query");
			EndDialog(hwnd, IDOK);
			_this->m_hwnd_edit_dialog = NULL;
			return TRUE;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			_this->m_hwnd_edit_dialog = NULL;
			return TRUE;		
		}
		return 0;
	}
	return 0;
}

ConnectionsAccess::~ConnectionsAccess()
{
	if (m_hwnd_edit_dialog != NULL) {
		EndDialog(m_hwnd_edit_dialog, IDCANCEL);
		m_hwnd_edit_dialog = NULL;
	}
}
