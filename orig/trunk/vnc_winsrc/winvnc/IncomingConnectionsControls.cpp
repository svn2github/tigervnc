// IncomingConnectionsControls.cpp: implementation of the IncomingConnectionsControls class.
//
//////////////////////////////////////////////////////////////////////

#include "IncomingConnectionsControls.h"
#include "WinVNC.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IncomingConnectionsControls::IncomingConnectionsControls(HWND hwnd, vncServer *server)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}

void IncomingConnectionsControls::Validate()
{
	BOOL bAccept = IsChecked(IDC_CONNECT_SOCK);
	BOOL bAuto = IsChecked(IDC_PORTNO_AUTO);
	BOOL bDisplay = IsChecked(IDC_SPECDISPLAY);
	BOOL bPorts = IsChecked(IDC_SPECPORT);
	Enable(IDC_PASSWORD_LABEL, bAccept);
	Enable(IDC_PASSWORD_VIEWONLY_LABEL, bAccept);
	Enable(IDC_PASSWORD, bAccept);
	Enable(IDC_PASSWORD_VIEWONLY, bAccept);
	Enable(IDC_PORTNO_AUTO, bAccept);
	Enable(IDC_SPECDISPLAY, bAccept);
	Enable(IDC_SPECPORT, bAccept);
		
	if (bAuto) {
		SetDlgItemText(m_hwnd, IDC_PORTRFB, "");
		SetDlgItemText(m_hwnd, IDC_PORTHTTP, "");
		SetDlgItemText(m_hwnd, IDC_DISPLAYNO, "");
	} else {
		UINT port_rfb = m_server->GetPort();
		UINT port_http = m_server->GetHttpPort();
		int d1 = PORT_TO_DISPLAY(port_rfb);
		int d2 = HPORT_TO_DISPLAY(port_http);
		BOOL bValidDisplay = (d1 == d2 && d1 >= 0 && d1 <= 99);
		if (bValidDisplay) {
			SetDlgItemInt(m_hwnd, IDC_DISPLAYNO, d1, FALSE);
		} else {
			SetDlgItemText(m_hwnd, IDC_DISPLAYNO, "");
		}
		SetDlgItemInt(m_hwnd, IDC_PORTRFB, port_rfb, FALSE);
		SetDlgItemInt(m_hwnd, IDC_PORTHTTP, port_http, FALSE);
	}
	Enable(IDC_DISPLAY_LABEL, bAccept && bDisplay);
	Enable(IDC_DISPLAYNO, bAccept && bDisplay);	
	Enable(IDC_PORTRFB, bAccept && bPorts);
	Enable(IDC_MAIN_LABEL, bAccept && bPorts);
	Enable(IDC_AND_LABEL, bAccept && bPorts);
	Enable(IDC_PORTHTTP, bAccept && bPorts);
	Enable(IDC_HTTP_LABEL, bAccept && bPorts);
	
	HWND hFocus = GetFocus();
	if (hFocus == GetDlgItem(m_hwnd, IDC_CONNECT_SOCK)) {
		SetFocus(GetDlgItem(m_hwnd, IDC_PASSWORD));
		SendMessage(GetDlgItem(m_hwnd, IDC_PASSWORD), EM_SETSEL, 0, (LPARAM)-1);
	}	
	
	if (hFocus == GetDlgItem(m_hwnd, IDC_SPECDISPLAY)) {
		SetFocus(GetDlgItem(m_hwnd, IDC_DISPLAYNO));
		SendMessage(GetDlgItem(m_hwnd, IDC_DISPLAYNO), EM_SETSEL, 0, (LPARAM)-1);
	}

	if (hFocus == GetDlgItem(m_hwnd, IDC_SPECPORT)) {
		SetFocus(GetDlgItem(m_hwnd, IDC_PORTRFB));
		SendMessage(GetDlgItem(m_hwnd, IDC_PORTRFB), EM_SETSEL, 0, (LPARAM)-1);
	}
}
void IncomingConnectionsControls::Apply()
{
	// Save the password if one was entered
	char passwd[MAXPWLEN+1];
	int len = GetDlgItemText(m_hwnd, IDC_PASSWORD, (LPSTR)&passwd, MAXPWLEN+1);
	if (strcmp(passwd, "~~~~~~~~") != 0) {
		if (len == 0) {
			vncPasswd::FromClear crypt;
			m_server->SetPassword(crypt);
		} else {
			vncPasswd::FromText crypt(passwd);
			m_server->SetPassword(crypt);
		}
	}
	
	// Save the password (view only) if one was entered
	// FIXME: Code duplication, see above
	len = GetDlgItemText(m_hwnd, IDC_PASSWORD_VIEWONLY, (LPSTR)&passwd, MAXPWLEN+1);
	if (strcmp(passwd, "~~~~~~~~") != 0) {
		if (len == 0) {
			vncPasswd::FromClear crypt;
			m_server->SetPasswordViewOnly(crypt);
		} else {
			vncPasswd::FromText crypt(passwd);
			m_server->SetPasswordViewOnly(crypt);
		}
	}
	
	// Save the new settings to the server
	m_server->SetAutoPortSelect(IsChecked(IDC_PORTNO_AUTO));
	
	// Save port numbers if we're not auto selecting
	if (!m_server->AutoPortSelect()) {
		if (IsChecked(IDC_SPECDISPLAY)) {
			// Display number was specified
			BOOL ok;
			UINT display = GetDlgItemInt(m_hwnd, IDC_DISPLAYNO, &ok, TRUE);
			if (ok)
				m_server->SetPorts(DISPLAY_TO_PORT(display),
				DISPLAY_TO_HPORT(display));
		} else {
			// Assuming that port numbers were specified
			BOOL ok1, ok2;
			UINT port_rfb = GetDlgItemInt(m_hwnd, IDC_PORTRFB, &ok1, TRUE);
			UINT port_http = GetDlgItemInt(m_hwnd, IDC_PORTHTTP, &ok2, TRUE);
			if (port_rfb != port_http) {
				if (ok1 && ok2)
					m_server->SetPorts(port_rfb, port_http);
			} else {
				MessageBox(NULL, "Warning: RFB and HTTP ports should be different",
						   szAppName, MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}
	
	m_server->SockConnect(IsChecked(IDC_CONNECT_SOCK));
	
	// Wallpaper handling
	m_server->EnableRemoveWallpaper(IsChecked(IDC_REMOVE_WALLPAPER));
	
	// Enabling/disabling file transfers
	m_server->EnableFileTransfers(IsChecked(IDC_ENABLE_FILE_TRANSFERS));
}

void IncomingConnectionsControls::Init()
{
	BOOL bConnectSock = m_server->SockConnected();
	BOOL bAutoPort = m_server->AutoPortSelect();
	UINT port_rfb = m_server->GetPort();
	UINT port_http = m_server->GetHttpPort();
	int d1 = PORT_TO_DISPLAY(port_rfb);
	int d2 = HPORT_TO_DISPLAY(port_http);
	BOOL bValidDisplay = (d1 == d2 && d1 >= 0 && d1 <= 99);
	
	if (bConnectSock) {
		SetFocus(GetDlgItem(m_hwnd, IDC_PASSWORD));
		SendDlgItemMessage(m_hwnd, IDC_PASSWORD, EM_SETSEL, 0, (LPARAM)-1);
	} else {
		SetFocus(GetDlgItem(m_hwnd, IDC_CONNECT_SOCK));
	}
	
	SetChecked(IDC_CONNECT_SOCK, bConnectSock);
	SetChecked(IDC_PORTNO_AUTO, bAutoPort);
	SetChecked(IDC_SPECDISPLAY, (!bAutoPort && bValidDisplay));
	SetChecked(IDC_SPECPORT, (!bAutoPort && !bValidDisplay));
	SetChecked(IDC_ENABLE_FILE_TRANSFERS, m_server->FileTransfersEnabled());
	SetChecked(IDC_REMOVE_WALLPAPER, m_server->RemoveWallpaperEnabled());
	
	SetDlgItemText(m_hwnd, IDC_PASSWORD, "~~~~~~~~");			
	SetDlgItemText(m_hwnd, IDC_PASSWORD_VIEWONLY, "~~~~~~~~");

	SetFocus(GetDlgItem(m_hwnd, IDC_CONNECT_SOCK));
	
	Validate();
}
IncomingConnectionsControls::~IncomingConnectionsControls()
{
	
}
