// ConnectionsAccess.cpp: implementation of the ConnectionsAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "ConnectionsAccess.h"
#include "WinVNC.h"
#include "VNCHelp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConnectionsAccess::ConnectionsAccess(vncServer * server, HWND hwnd)
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

	// Allocate 17 bytes per each pattern (IP, action and separator)
	char *auth_hosts = (char *)malloc(count * 17);
	auth_hosts[0] = '\0';

	for (int i = count - 1; i >= 0; i--) {
		GetListViewItem(i, ItemString);
		if (!FormatPattern(FALSE, ItemString[0]))
			continue;

		if (strcmp(ItemString[1], "Allow") == 0) {
			strcat(auth_hosts, "+");
		} else if (strcmp(ItemString[1], "Deny") == 0) {
			strcat(auth_hosts, "-");
		} else if (strcmp(ItemString[1], "Query") == 0) {
			strcat(auth_hosts, "?");
		}

		strcat(auth_hosts, ItemString[0]);
		if (i != 0)
			strcat(auth_hosts, ":");
	}

	m_server->SetAuthHosts(auth_hosts);
	free(auth_hosts);
}

void ConnectionsAccess::Init()
{
	InitListViewColumns();
	char *auth_hosts = m_server->AuthHosts();

	char *pattern = auth_hosts;
	for (;;) {
		int len = strcspn(pattern, ":");
		if (strspn(pattern, "+-?") == 1 && len <= 256) {
			memcpy(ItemString[0], &pattern[1], len - 1);
			ItemString[0][len - 1] = '\0';
			switch (pattern[0]) {
			case '+': strcpy(ItemString[1], "Allow"); break;
			case '-': strcpy(ItemString[1], "Deny"); break;
			case '?': strcpy(ItemString[1], "Query"); break;
			}
			if (FormatPattern(TRUE, ItemString[0]))
				InsertListViewItem(0, ItemString);
		}
		if (pattern[len] == '\0')
			break;
		pattern += len + 1;
	}

	free(auth_hosts);
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

void ConnectionsAccess::GetListViewItem(int Numbe, TCHAR ItemString[2][256])
{
	for (int i = 0; i < 2; i++) {
		strcpy(ItemString[i], "");
		ListView_GetItemText(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
							Numbe, i, ItemString[i], 256);							
	}
}

BOOL ConnectionsAccess::InsertListViewItem(int Numbe, TCHAR ItemString[2][256])
{
	LVITEM lvI;
	lvI.mask = LVIF_TEXT| LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = Numbe; 
	lvI.iSubItem = 0; 
	lvI.pszText = ItemString[0]; 									  
	
	if(ListView_InsertItem(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), &lvI) == -1)
		return NULL;
		
	ListView_SetItemText(
			GetDlgItem(m_hwnd, IDC_LIST_HOSTS), 
			Numbe, 1, ItemString[1]);
	
	return TRUE;
}

BOOL ConnectionsAccess::InitListViewColumns()
{
	ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
									  LVS_EX_FULLROWSELECT);
	TCHAR *ColumnsStrings[] = {
		"IP pattern",
		"Action"
	};

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int iCol = 0; iCol < 2; iCol++) {
		lvc.iSubItem = iCol;
		lvc.pszText = ColumnsStrings[iCol];
		lvc.cx = 97;
		lvc.fmt = LVCFMT_LEFT;

		if (ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), iCol, &lvc) == -1)
			return FALSE;
	}
	return TRUE;
}

BOOL ConnectionsAccess::FormatPattern(BOOL toList, TCHAR strpattern[256])
{
	char *cut_ptr = strpattern;
	int num_components = 0;
	int num_asterisks = 0;

	if (strpattern[0] != '\0') {
		char *component = strpattern;
		for (;;) {
			int len = strcspn(component, ".");
			if (++num_components > 4) {
				return FALSE;		// too many components in IP pattern
			}
			if (len == 1 && component[0] == '*') {
				num_asterisks++;
			} else if (num_asterisks != 0) {
				return FALSE;		// non-'*' found after there was '*'
			} else {
				if ( len < 1 || len > 3 ||
					strspn(component, "0123456789") != len ||
					atoi(component) > 255 ) {
					return FALSE;	// not a number from the range 0..255
				}
				cut_ptr = &component[len];
			}
			if (component[len] == '\0')
				break;
			component += len + 1;
		}
	}

	// Now we are sure the format is correct.

	*cut_ptr = '\0';
	if (toList) {
		if (cut_ptr == strpattern) {
			strcpy(strpattern, "*.*.*.*");
		} else {
			num_asterisks = 4 - (num_components - num_asterisks);
			strncat(strpattern, ".*.*.*", num_asterisks * 2);
		}
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
					(strcmp(_this->ItemString[1], "Allow") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_SETCHECK,
					(strcmp(_this->ItemString[1], "Deny") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_SETCHECK,
					(strcmp(_this->ItemString[1], "Query") == 0), 0);
				_this->FormatPattern(FALSE, _this->ItemString[0]);
				SetDlgItemText(hwnd, IDC_HOST_PATTERN, _this->ItemString[0]);
			} else {
				SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_SETCHECK, TRUE, 0);
			}
			return TRUE;
		}
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd, IDC_HOST_PATTERN, _this->ItemString[0], 80);

			if (!_this->FormatPattern(TRUE, _this->ItemString[0])) {
				MessageBox(hwnd,
						   "The pattern format is incorrect. It should be entered\n"
						   "as A.B.C.D or A.B.C or A.B or A, where each element\n"
						   "should be an unsigned number from the range 0..255",
						   "Error", MB_ICONSTOP | MB_OK);
				return TRUE;
			}
			if (SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				strcpy(_this->ItemString[1], "Allow");
			} else if (SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				strcpy(_this->ItemString[1], "Deny");
			} else if (SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				strcpy(_this->ItemString[1], "Query");
			}
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
