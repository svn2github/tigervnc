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
	strcpy(ItemString[0],"");
	strcpy(ItemString[1],"");
	Init();
}

void ConnectionsAccess::Apply()
{
	int count = GetItemCount();
	if (count < 1) {
		m_server->SetAuthHosts(0);
		return;
	}
	char *authosts = strdup("");
	for (int i = count - 1; i >= 0; i--) {

		GetListViewItem(i, ItemString);
		FormatPattern(FALSE, ItemString[0]);

		if (strcmp(ItemString[1], "Allow") == 0)
			strcat(authosts, "+");

		if (strcmp(ItemString[1], "Deny") == 0)			
			strcat(authosts, "-");

		if (strcmp(ItemString[1], "Query") == 0)			
			strcat(authosts, "?");

		strcat(authosts, ItemString[0]);
		if (i != 0)
			strcat(authosts, ":");

	}
	m_server->SetAuthHosts(authosts);
	//delete [] authosts;
}
void ConnectionsAccess::Init()
{
	InitListViewColumns();
	char *authosts = strdup(m_server->AuthHosts());
	strcpy(ItemString[0],"");
	strcpy(ItemString[1],"");
	int authHostsPos = 0;
	int startPattern = 0;
	if (strlen(authosts) > 0) {
		while (1) {
			if ((authosts[authHostsPos] == ':') || 
				(authosts[authHostsPos] == '\0')) {			
				ItemString[0][authHostsPos - startPattern - 1] = '\0';
				if (FormatPattern(TRUE, ItemString[0]))
					InsertListViewItem(0, ItemString);
				if (authosts[authHostsPos] == '\0')
					break;
			} else if (authosts[authHostsPos] == '+') {				
				strcpy(ItemString[1], "Allow");
				startPattern = authHostsPos;				
			} else if (authosts[authHostsPos] == '-') {
				strcpy(ItemString[1], "Deny");
				startPattern = authHostsPos;
			} else if (authosts[authHostsPos] == '?') {
				strcpy(ItemString[1], "Query");
				startPattern = authHostsPos;
			} else {			
				ItemString[0][authHostsPos - startPattern - 1] = authosts[authHostsPos];
			}
			authHostsPos++;			 
		}
	}
	delete [] authosts;
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
	char szText[256];      
	LVCOLUMN lvc; 
	int iCol;
    
	TCHAR *ColumnsStrings[] = {
		"IP pattern",
		"Action"
	};
	
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	
	for (iCol = 0; iCol < 2; iCol++) { 
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

BOOL ConnectionsAccess::FormatPattern(BOOL toList, TCHAR strpattern[256])
{
	int point = 0;
	int prev = 0;
	int i = 0;
	int lenpattern = strlen(strpattern);

	if (strpattern[0] == '.')
		strpattern[0] = '0';
	if (strpattern[lenpattern - 1] == '.')
		strpattern[0] = '\0';

	for (i = lenpattern; i >= 0; i--) {
		if (strpattern[i] == '*' && i == 0) {
			strpattern[i] = '\0';
			break;
		}
		if (strpattern[i] == '*' && strpattern[i - 1] == '.')
			strpattern[i - 1] = '\0';
	}
	BOOL cor = TRUE;
	lenpattern = strlen(strpattern);
	char *buf = strdup(strpattern);
	int j = 0;
	for (i = 0; i < lenpattern; i++) {
		if (buf[i] == '.') {
			strpattern[j] = buf[i];
			j++;
			cor = TRUE;
			if (i == 0 || i == lenpattern - 1 || i - prev == 1) {
				delete [] buf;
				return FALSE;
			}
			prev = i;
			point++;
		} else if (buf[i] != '0' || buf[i + 1] == '.' || buf[i + 1] == '\0') {
			cor = FALSE;
		}
		if (!cor) {
			strpattern[j] = buf[i];
			j++;
		}
	}
	delete []buf;
	strpattern[j] = '\0';
	lenpattern = strlen(strpattern);

	for (i = 0; i < 3 - point; i++) {
			strpattern[lenpattern++] = '.';
			strpattern[lenpattern++] = '0';
			strpattern[lenpattern] = '\0';
		}
	
	unsigned long adrr = inet_addr(strpattern);
	if (adrr == INADDR_NONE)
		return FALSE;
	
	strpattern[lenpattern - (3 - point) * 2] = '\0';

	lenpattern = strlen(strpattern);
	
	if (toList) {
		for (i = 0; i < 3 - point; i++) {
			if (strcmp(strpattern, "") == 0)
				strcpy(strpattern, "*");
			strcat(strpattern, ".*");
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
			SetFocus(GetDlgItem(hwnd, IDC_HOST_PATTERN));
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

			if (!_this->FormatPattern(TRUE, _this->ItemString[0])) {
				MessageBox(hwnd,
						   "The pattern format is incorrect. It should be entered\n"
						   "as A.B.C.D or A.B.C or A.B or A, where each element\n"
						   "should be an unsigned number from the range 0..255",
						   "Error", MB_ICONSTOP | MB_OK);
				return TRUE;
			}
			if (SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[1], "Allow");
			if (SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[1], "Deny");
			if (SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_GETCHECK, 0, 0) == BST_CHECKED)
				strcpy(_this->ItemString[1], "Query");
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
