//  Copyright (C) 2002 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
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
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncProperties.cpp

// Implementation of the Properties dialog!

#include "stdhdrs.h"
#include "lmcons.h"
#include "vncService.h"

#include "WinVNC.h"
#include "vncProperties.h"
#include "vncAdvancedProperties.h"
#include "vncServer.h"
#include "vncPasswd.h"

const char WINVNC_REGISTRY_KEY [] = "Software\\ORL\\WinVNC3";
const char NO_PASSWORD_WARN [] = "WARNING : Running WinVNC without setting a password is "
								"a dangerous security risk!\n"
								"Until you set a password, WinVNC will not accept incoming connections.";
const char NO_OVERRIDE_ERR [] = "This machine has been preconfigured with WinVNC settings, "
								"which cannot be overridden by individual users.  "
								"The preconfigured settings may be modified only by a System Administrator.";
const char NO_PASSWD_NO_OVERRIDE_ERR [] =
								"No password has been set & this machine has been "
								"preconfigured to prevent users from setting their own.\n"
								"You must contact a System Administrator to configure WinVNC properly.";
const char NO_PASSWD_NO_OVERRIDE_WARN [] =
								"WARNING : This machine has been preconfigured to allow un-authenticated\n"
								"connections to be accepted and to prevent users from enabling authentication.";
const char NO_PASSWD_NO_LOGON_WARN [] =
								"WARNING : This machine has no default password set.  WinVNC will present the "
								"Default Properties dialog now to allow one to be entered.";
const char NO_CURRENT_USER_ERR [] = "The WinVNC settings for the current user are unavailable at present.";
const char CANNOT_EDIT_DEFAULT_PREFS [] = "You do not have sufficient priviliges to edit the default local WinVNC settings.";

// Constructor & Destructor
vncProperties::vncProperties()
{
	m_allowproperties = TRUE;
	m_allowshutdown = TRUE;
	m_dlgvisible = FALSE;
	m_usersettings = TRUE;
	m_inadvanced = FALSE;
	m_bCaptured= FALSE;
	m_KeepHandle = NULL;
	m_pMatchWindow = NULL;

}

vncProperties::~vncProperties()
{
}

// Initialisation
BOOL
vncProperties::Init(vncServer *server)
{
	// Save the server pointer
	m_server = server;
	
	m_inadvanced = FALSE;

//#ifdef HORIZONLIVE
//	strcpy(m_pref_LiveShareKey,"");
//#endif
	
	// Load the settings from the registry
	Load(TRUE);

	if (m_pMatchWindow == NULL) 
	{
		RECT temp;
		GetWindowRect(GetDesktopWindow(), &temp);
		m_pMatchWindow=new CMatchWindow(m_server,temp.left+5,temp.top+5,temp.right/2,temp.bottom/2);
		m_pMatchWindow->CanModify(TRUE);
	}

#ifdef HORIZONLIVE
	
	char username[UNLEN+1];
	if (!vncService::CurrentUser(username, sizeof(username)))
		return FALSE;
	if (!vncService::GetNoSettings())
	{
		if (strcmp(username, "") == 0) 
			Show(TRUE, FALSE);
		else 
			Show(TRUE, TRUE);
	}

#else
	// If the password is empty then always show a dialog
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	{
	    vncPasswd::ToText plain(passwd);
	    if (strlen(plain) == 0)
			if (!m_allowproperties) {
				if(m_server->AuthRequired()) {
					MessageBox(NULL, NO_PASSWD_NO_OVERRIDE_ERR,
								"WinVNC Error",
								MB_OK | MB_ICONSTOP);
					PostQuitMessage(0);
				} else {
					MessageBox(NULL, NO_PASSWD_NO_OVERRIDE_WARN,
								"WinVNC Error",
								MB_OK | MB_ICONEXCLAMATION);
				}
			} else {
				// If null passwords are not allowed, ensure that one is entered!
				if (m_server->AuthRequired()) {
					char username[UNLEN+1];
					if (!vncService::CurrentUser(username, sizeof(username)))
						return FALSE;
					if (strcmp(username, "") == 0) {
						MessageBox(NULL, NO_CURRENT_USER_ERR,
									"WinVNC Error",
									MB_OK | MB_ICONEXCLAMATION);
						Show(TRUE, FALSE);
					} else {
						Show(TRUE, TRUE);
					}
				}
			}
	}

#endif

	return TRUE;
}

// Dialog box handling functions
void
vncProperties::Show(BOOL show, BOOL usersettings)
{
	if (show)
	{
		m_inadvanced = FALSE;
		if (!m_allowproperties)
		{
			// If the user isn't allowed to override the settings then tell them
			MessageBox(NULL, NO_OVERRIDE_ERR, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// Verify that we know who is logged on
		if (usersettings) {
			char username[UNLEN+1];
			if (!vncService::CurrentUser(username, sizeof(username)))
				return;
//			if (strcmp(username, "") == 0) {
//				MessageBox(NULL, NO_CURRENT_USER_ERR, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
//				return;
//			}
		} else {
			// We're trying to edit the default local settings - verify that we can
			HKEY hkLocal, hkDefault;
			BOOL canEditDefaultPrefs = 1;
			DWORD dw;
			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			else if (RegCreateKeyEx(hkLocal,
				"Default",
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_WRITE | KEY_READ, NULL, &hkDefault, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			if (hkLocal) RegCloseKey(hkLocal);
			if (hkDefault) RegCloseKey(hkDefault);

			if (!canEditDefaultPrefs) {
				MessageBox(NULL, CANNOT_EDIT_DEFAULT_PREFS, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
				return;
			}
		}

		// Now, if the dialog is not already displayed, show it!
		if (!m_dlgvisible)
		{
			if (usersettings)
				vnclog.Print(LL_INTINFO, VNCLOG("show per-user Properties\n"));
			else
				vnclog.Print(LL_INTINFO, VNCLOG("show default system Properties\n"));

			// Load in the settings relevant to the user or system
			Load(usersettings);

#ifdef HORIZONLIVE
			m_returncode_valid = FALSE;
			// Do the dialog box
			int result = DialogBoxParam(hAppInstance,
			    MAKEINTRESOURCE(IDD_LIVESHAREPROP), 
			    NULL,
			    (DLGPROC) DialogProc,
			    (LONG) this);

			if (!m_returncode_valid)
			    result = IDCANCEL;

			vnclog.Print(LL_INTINFO, VNCLOG("dialog result = %d\n"), result);

			if (result == -1)
			{
				// Dialog box failed, so quit
				PostQuitMessage(0);
				return;
			}
#else
			for (;;)
			{
				m_returncode_valid = FALSE;

				// Do the dialog box
				int result = DialogBoxParam(hAppInstance,
				    MAKEINTRESOURCE(IDD_PROPERTIES), 
				    NULL,
				    (DLGPROC) DialogProc,
				    (LONG) this);

				if (!m_returncode_valid)
				    result = IDCANCEL;

				vnclog.Print(LL_INTINFO, VNCLOG("dialog result = %d\n"), result);

				if (result == -1)
				{
					// Dialog box failed, so quit
					PostQuitMessage(0);
					return;
				}

				// We're allowed to exit if the password is not empty
				char passwd[MAXPWLEN];
				m_server->GetPassword(passwd);
				{
				    vncPasswd::ToText plain(passwd);
				    if ((strlen(plain) != 0) || !m_server->AuthRequired())
					break;
				}

				vnclog.Print(LL_INTERR, VNCLOG("warning - empty password\n"));

				// The password is empty, so if OK was used then redisplay the box,
				// otherwise, if CANCEL was used, close down WinVNC
				if (result == IDCANCEL)
				{
				    vnclog.Print(LL_INTERR, VNCLOG("no password - QUITTING\n"));
				    PostQuitMessage(0);
				    return;
				}

				// If we reached here then OK was used & there is no password!
				int result2 = MessageBox(NULL, NO_PASSWORD_WARN,
				    "WinVNC Warning", MB_OK | MB_ICONEXCLAMATION);

				omni_thread::sleep(4);
			}
#endif
			// Load in all the settings
			Load(TRUE);
		}
	}
}

BOOL CALLBACK
vncProperties::DialogProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (vncProperties *) lParam;
			
			HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
			_this->m_OldBmpWndProc=GetWindowLong(bmp_hWnd,GWL_WNDPROC);
			SetWindowLong(bmp_hWnd,GWL_WNDPROC,(LONG)BmpWndProc);
			SetWindowLong(bmp_hWnd, GWL_USERDATA, (LONG)_this);
			HBITMAP hNewImage,hOldImage;
			_this->hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);

#ifdef HORIZONLIVE
			// Set the dialog box's title
			SetWindowText(hwnd, "LiveShare Settings");
			
			HWND hLiveShare = GetDlgItem(hwnd, IDC_LIVESHARE);
			::SetWindowText(hLiveShare, _this->m_pref_LiveShareKey);                                                                                                  
			if (_this->m_server->AuthClientCount() != 0)
				EnableWindow(hLiveShare,false);

#else
			// Set the dialog box's title to indicate which Properties we're editting
			if (_this->m_usersettings) {
				SetWindowText(hwnd, "WinVNC: Current User Properties");
			} else {
				SetWindowText(hwnd, "WinVNC: Default Local System Properties");
			}

			// Initialise the properties controls
			HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
			BOOL bConnectSock = _this->m_server->SockConnected();
			SendMessage(hConnectSock, BM_SETCHECK, bConnectSock, 0);

			// Set the content of the password field to a predefined string.
		    SetDlgItemText(hwnd, IDC_PASSWORD, "~~~~~~~~");
			EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), bConnectSock);

			// Set display/ports settings
			_this->InitPortSettings(hwnd);

			// Remote input settings
			HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
			SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(_this->m_server->RemoteInputsEnabled()),
				0);

			// Local input settings
			HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
			SendMessage(hDisableLocalInputs,
				BM_SETCHECK,
				_this->m_server->LocalInputsDisabled(),
				0);

			// Local input prioity settings
			HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
			SendMessage(hRemoteDisable,
				BM_SETCHECK,
				_this->m_server->LocalInputPriority(),
				0);

			HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
			SetDlgItemInt(hwnd, IDC_DISABLE_TIME, _this->m_server->DisableTime(), FALSE);
			
			if ( !_this->m_server->RemoteInputsEnabled() ||
				 _this->m_server->LocalInputsDisabled() ) {
				EnableWindow(hRemoteDisable, FALSE);
				EnableWindow(hDisableTime, FALSE);
			}
			
			if (!_this->m_server->LocalInputPriority())
				EnableWindow(hDisableTime, FALSE);

			// Set the polling options
			HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
			SendMessage(hPollFullScreen,
				BM_SETCHECK,
				_this->m_server->PollFullScreen(),
				0);

			HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
			SendMessage(hPollForeground,
				BM_SETCHECK,
				_this->m_server->PollForeground(),
				0);

			HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
			SendMessage(hPollUnderCursor,
				BM_SETCHECK,
				_this->m_server->PollUnderCursor(),
				0);

			HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
			SendMessage(hPollConsoleOnly,
				BM_SETCHECK,
				_this->m_server->PollConsoleOnly(),
				0);
			EnableWindow(hPollConsoleOnly,
				_this->m_server->PollUnderCursor() || _this->m_server->PollForeground()
				);

			HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
			SendMessage(hPollOnEventOnly,
				BM_SETCHECK,
				_this->m_server->PollOnEventOnly(),
				0);
			EnableWindow(hPollOnEventOnly,
				_this->m_server->PollUnderCursor() || _this->m_server->PollForeground()
				);

			HWND hPollingTimer = GetDlgItem(hwnd, IDC_POLL_TIMER);
			SetDlgItemInt(hwnd, IDC_POLL_TIMER, _this->m_server->GetPollingTimer(), FALSE);

#endif
	
			if (_this->m_pref_FullScreen) {
				// hide shared area window
				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Hide();

				// disable window select stuff
				EnableWindow(bmp_hWnd,FALSE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);
				::SetWindowText(_this->hNameAppli,"Full Desktop Selected");                                                                                                  
			}

			if(_this->m_pref_WindowShared) {

				EnableWindow(bmp_hWnd,TRUE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);	
				_this->SetWindowCaption(_this->m_server->GetWindowShared());

				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Hide();
	
			}
			
			if (_this->m_pref_ScreenAreaShared) { 
				EnableWindow(bmp_hWnd,FALSE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);
				::SetWindowText(_this->hNameAppli,"Screen Area Selected");

				if (_this->m_pMatchWindow == NULL) {
					RECT temp;
					temp = _this->m_server->getSharedRect();
					_this->m_pMatchWindow=new CMatchWindow(_this->m_server,temp.left+5,temp.top+5,temp.right,temp.bottom);
					_this->m_pMatchWindow->CanModify(TRUE);
				}
				_this->m_pMatchWindow->Show();
			}
				

			HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
            SendMessage(hFullScreen,    
			    BM_SETCHECK,
                _this->m_pref_FullScreen,   0);

			EnableWindow(_this->hNameAppli,(_this->m_pref_WindowShared));

			HWND hWindowCaption = GetDlgItem(hwnd, IDC_WINDOW);
			SendMessage(hWindowCaption,
				BM_SETCHECK, 
				_this->m_pref_WindowShared,0);

			HWND hScreenCaption = GetDlgItem(hwnd, IDC_SCREEN);
			SendMessage(hScreenCaption,
				BM_SETCHECK, 
				(_this->m_pref_ScreenAreaShared),0);

			SetForegroundWindow(hwnd);

			_this->m_dlgvisible = TRUE;

			return TRUE;
		}

	case WM_COMMAND:
		if (_this->m_inadvanced)
			return FALSE;
		switch (LOWORD(wParam))
		{

		case IDOK:
		case IDC_APPLY:
			{
#if !defined HORIZONLIVE				
				// Save the password if one was entered
				char passwd[MAXPWLEN+1];
				int len = GetDlgItemText(hwnd, IDC_PASSWORD, (LPSTR)&passwd, MAXPWLEN+1);
				if (strcmp(passwd, "~~~~~~~~") != 0) {
					if (len == 0) {
						vncPasswd::FromClear crypt;
						_this->m_server->SetPassword(crypt);
					} else {
						vncPasswd::FromText crypt(passwd);
						_this->m_server->SetPassword(crypt);
					}
				}

				// Save the new settings to the server
				int state = SendDlgItemMessage(hwnd, IDC_PORTNO_AUTO, BM_GETCHECK, 0, 0);
				_this->m_server->SetAutoPortSelect(state == BST_CHECKED);

				// Save port numbers if we're not auto selecting
				if (!_this->m_server->AutoPortSelect()) {
					if ( SendDlgItemMessage(hwnd, IDC_SPECDISPLAY,
											BM_GETCHECK, 0, 0) == BST_CHECKED ) {
						// Display number was specified
						BOOL ok;
						UINT display = GetDlgItemInt(hwnd, IDC_DISPLAYNO, &ok, TRUE);
						if (ok)
							_this->m_server->SetPorts(DISPLAY_TO_PORT(display),
													  DISPLAY_TO_HPORT(display));
					} else {
						// Assuming that port numbers were specified
						BOOL ok1, ok2;
						UINT port_rfb = GetDlgItemInt(hwnd, IDC_PORTRFB, &ok1, TRUE);
						UINT port_http = GetDlgItemInt(hwnd, IDC_PORTHTTP, &ok2, TRUE);
						if (ok1 && ok2)
							_this->m_server->SetPorts(port_rfb, port_http);
					}
				}

				HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
				_this->m_server->SockConnect(
					SendMessage(hConnectSock, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Update display/port controls on pressing the "Apply" button
				if (LOWORD(wParam) == IDC_APPLY)
					_this->InitPortSettings(hwnd);

				// Remote input stuff
				HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
				_this->m_server->EnableRemoteInputs(
					SendMessage(hEnableRemoteInputs, BM_GETCHECK, 0, 0) != BST_CHECKED
					);

				// Local input stuff
				HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
				_this->m_server->DisableLocalInputs(
					SendMessage(hDisableLocalInputs, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Handle the polling stuff
				HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
				_this->m_server->PollFullScreen(
					SendMessage(hPollFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
				_this->m_server->PollForeground(
					SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
				_this->m_server->PollUnderCursor(
					SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
				_this->m_server->PollConsoleOnly(
					SendMessage(hPollConsoleOnly, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
				_this->m_server->PollOnEventOnly(
					SendMessage(hPollOnEventOnly, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
				_this->m_server->LocalInputPriority(
					SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) == BST_CHECKED
				);

				BOOL success;
				UINT disabletime = GetDlgItemInt(hwnd, IDC_DISABLE_TIME, &success, TRUE);
				if (success)
					_this->m_server->SetDisableTime(disabletime);

				UINT pollingtimer = GetDlgItemInt(hwnd, IDC_POLL_TIMER, &success, TRUE);
				if (success)
					_this->m_server->SetPollingTimer(pollingtimer);

								
#else
				_this->m_server->SetLiveShareKey(_this->m_pref_LiveShareKey);

#endif

				// check that Shared window not null
				if ( _this->m_pref_WindowShared && (_this->m_server->GetWindowShared() == NULL) )
				{
					MessageBox(NULL,"You have not yet selected a window to share.\nPlease first select a window with the 'Window Target'\nicon and try again.", "No Window Selected", MB_OK | MB_ICONEXCLAMATION);
					return true;
				}

				// Handle the share one window stuff
			    HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
			    _this->m_server->FullScreen(
					SendMessage(hFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED);
				  				
				HWND hWindowCapture = GetDlgItem(hwnd, IDC_WINDOW);
			    _this->m_server->WindowShared(
					SendMessage(hWindowCapture, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hScreenArea = GetDlgItem(hwnd, IDC_SCREEN);
			    _this->m_server->ScreenAreaShared(
					SendMessage(hScreenArea, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				if ( _this->m_pref_ScreenAreaShared) {
					int left,right,top,bottom;
		            if (_this->m_pMatchWindow!=NULL) {
						_this->m_pMatchWindow->GetPosition(left,top,right,bottom);
						_this->m_server->SetMatchSizeFields(left,top,right,bottom);
					}
				}

				if (_this->m_pref_FullScreen) {
					RECT temp;
					GetWindowRect(GetDesktopWindow(), &temp);
					_this->m_server->SetMatchSizeFields(temp.left, temp.top, temp.right, temp.bottom);
				}

				// And to the registry
				_this->Save();

				// Was ok pressed?
				if (LOWORD(wParam) == IDOK)
				{

# ifdef HORIZONLIVE					
					if (_this->m_server->AuthClientCount() == 0)
					{
						char hostemp [_MAX_PATH];
						char *portp;
						int port;
						strcpy(hostemp, _this->m_pref_LiveShareKey);
						// Calculate the Display and Port offset.
						port = INCOMING_PORT_OFFSET;
						portp = strchr(hostemp, ':');
						if (portp)
						{
							*portp++ = '\0';
							port += atoi(portp);
						}

						// Attempt to create a new socket
						VSocket *tmpsock;
						tmpsock = new VSocket;
						if (!tmpsock)
							return TRUE;

						// Connect out to the specified host on the VNCviewer listen port
						// To be really good, we should allow a display number here but
						// for now we'll just assume we're connecting to display zero
						tmpsock->Create();
						if (tmpsock->Connect(hostemp, port)) {
							// Add the new client to this server
							_this->m_server->AddClient(tmpsock, TRUE, TRUE);
							
						} else {
							// Print up an error message
							MessageBox(NULL, 
							"LiveShare was unable to begin sharing your computer.\nPlease verify that you have entered the correct LiveShare Key and try again.",
							"LiveShare Connection Error",
							MB_OK | MB_ICONEXCLAMATION );
							delete tmpsock;
							return true;
						}
					}
#endif					
					
					// Yes, so close the dialog
					vnclog.Print(LL_INTINFO, VNCLOG("enddialog (OK)\n"));

					_this->m_returncode_valid = TRUE;

					EndDialog(hwnd, IDOK);
					_this->m_dlgvisible = FALSE;
				}

				return TRUE;
			}

		case IDCANCEL:
			vnclog.Print(LL_INTINFO, VNCLOG("enddialog (CANCEL)\n"));

			_this->m_returncode_valid = TRUE;

			EndDialog(hwnd, IDCANCEL);
			_this->m_dlgvisible = FALSE;
			return TRUE;

		case IDADVANCED:
			vnclog.Print(LL_INTINFO, VNCLOG("newdialog (ADVANCED)\n"));
			{
				EnableWindow(hwnd, FALSE);
				_this->m_inadvanced = TRUE;
				vncAdvancedProperties *aprop = new vncAdvancedProperties();
				if (aprop->Init(_this->m_server))
				{
					aprop->Show(TRUE, _this->m_usersettings);
				}
				//aprop->DoDialog();
				SetForegroundWindow(hwnd);
				_this->m_inadvanced = FALSE;
				EnableWindow(hwnd, TRUE);
				omni_thread::sleep(0, 200000000);
			}
			return TRUE;

#if !defined HORIZONLIVE		
		case IDC_CONNECT_SOCK:
			// The user has clicked on the socket connect tickbox
			{
				BOOL bConnectSock =
					(SendDlgItemMessage(hwnd, IDC_CONNECT_SOCK,
										BM_GETCHECK, 0, 0) == BST_CHECKED);

				EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), bConnectSock);

				HWND hPortNoAuto = GetDlgItem(hwnd, IDC_PORTNO_AUTO);
				EnableWindow(hPortNoAuto, bConnectSock);
				HWND hSpecDisplay = GetDlgItem(hwnd, IDC_SPECDISPLAY);
				EnableWindow(hSpecDisplay, bConnectSock);
				HWND hSpecPort = GetDlgItem(hwnd, IDC_SPECPORT);
				EnableWindow(hSpecPort, bConnectSock);

				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), bConnectSock &&
					(SendMessage(hSpecDisplay, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
			}
			return TRUE;

		case IDC_POLL_FOREGROUND:
		case IDC_POLL_UNDER_CURSOR:
			// User has clicked on one of the polling mode buttons
			// affected by the pollconsole and pollonevent options
			{
				// Get the poll-mode buttons
				HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
				HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);

				// Determine whether to enable the modifier options
				BOOL enabled = (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED) ||
					(SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
				EnableWindow(hPollConsoleOnly, enabled);

				HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
				EnableWindow(hPollOnEventOnly, enabled);
			}
			return TRUE;

		case IDC_PORTNO_AUTO:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				SetDlgItemText(hwnd, IDC_PORTRFB, "");
				SetDlgItemText(hwnd, IDC_PORTHTTP, "");
			}
			return TRUE;

		case IDC_SPECDISPLAY:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				int display = PORT_TO_DISPLAY(_this->m_server->GetPort());
				if (display < 0 || display > 99)
					display = 0;
				SetDlgItemInt(hwnd, IDC_DISPLAYNO, display, FALSE);
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_DISPLAYNO));
				SendDlgItemMessage(hwnd, IDC_DISPLAYNO, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		case IDC_SPECPORT:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), TRUE);

				int d1 = PORT_TO_DISPLAY(_this->m_server->GetPort());
				int d2 = HPORT_TO_DISPLAY(_this->m_server->GetHttpPort());
				if (d1 == d2 && d1 >= 0 && d1 <= 99) {
					SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
				} else {
					SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				}
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_PORTRFB));
				SendDlgItemMessage(hwnd, IDC_PORTRFB, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		case IDC_DISABLE_INPUTS:
		case IDC_DISABLE_LOCAL_INPUTS:
			{
			
				HWND hDisableInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
				HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	
				// Determine whether to enable the modifier options
				BOOL enabled = (SendMessage(hDisableInputs, BM_GETCHECK, 0, 0) != BST_CHECKED) &&
				(SendMessage(hDisableLocalInputs, BM_GETCHECK, 0, 0) != BST_CHECKED);

				HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
				HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
				
				BOOL enabl = (SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) != BST_CHECKED);
								
				EnableWindow(hRemoteDisable, enabled);
				EnableWindow(hDisableTime, enabled & !enabl);
				
				if (!enabled)
					SendMessage(hRemoteDisable, BM_SETCHECK, enabled, 0);
				

			}
			return TRUE;
	
		case IDC_REMOTE_DISABLE:
			{
				HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
				HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
				BOOL enabled = (SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) == BST_CHECKED);
				EnableWindow(hDisableTime, enabled);
				if (enabled) {
					SetFocus(hDisableTime);
					SendMessage(hDisableTime, EM_SETSEL, 0, (LPARAM)-1);
				}
			}
			return TRUE;
	
				

#else
		case IDC_LIVESHARE:
			{
				char entered_key [_MAX_PATH];
				char cleaned_key [_MAX_PATH];
				char *pos = cleaned_key;

				GetDlgItemText(hwnd, IDC_LIVESHARE, entered_key, _MAX_PATH);

				// Clean out low ASCII chars from the LiveShare Key
				for(int i=0; i<strlen(entered_key); i++)
					if(entered_key[i] > 0x2C)
						*pos++ = entered_key[i];

				*pos = '\0';
				
				strcpy(_this->m_pref_LiveShareKey, cleaned_key);
			}
			return TRUE;

#endif

		case IDC_FULLSCREEN:
			{	
				HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
			    HBITMAP hNewImage,hOldImage;

				_this->m_pref_FullScreen = TRUE;
				_this->m_pref_WindowShared = FALSE;
				_this->m_pref_ScreenAreaShared = FALSE;
				_this->hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);
				EnableWindow(_this->hNameAppli, FALSE);
				::SetWindowText(_this->hNameAppli,"Full Desktop Selected");
			
				EnableWindow(bmp_hWnd,FALSE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);
				
				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Hide();
			}
		
			return TRUE;
	
		case IDC_WINDOW:
			{
				HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
				HBITMAP hNewImage,hOldImage;
				EnableWindow(bmp_hWnd,TRUE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);	
				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Hide();
			   
				_this->SetWindowCaption(_this->m_server->GetWindowShared());
				EnableWindow(GetDlgItem(hwnd,IDC_NAME_APPLI),TRUE);
				_this->m_pref_FullScreen = FALSE;
				_this->m_pref_WindowShared = TRUE;
				_this->m_pref_ScreenAreaShared = FALSE;
			
			}
			return TRUE;

		case IDC_SCREEN:
			{
			
				if (_this->m_pMatchWindow == NULL) 
				{
					RECT temp;
					temp = _this->m_server->getSharedRect();
					_this->m_pMatchWindow=new CMatchWindow(_this->m_server,temp.left+5,temp.top+5,temp.right,temp.bottom);
					_this->m_pMatchWindow->CanModify(TRUE);
				}

				HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
			    HBITMAP hNewImage,hOldImage;
				::SetWindowText(_this->hNameAppli,"Screen Area Selected");
				_this->m_pref_WindowShared = FALSE;
				EnableWindow(bmp_hWnd,FALSE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);
				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Show();
				EnableWindow(GetDlgItem(hwnd,IDC_NAME_APPLI),FALSE);
				_this->m_pref_FullScreen = FALSE;
				_this->m_pref_WindowShared = FALSE;
				_this->m_pref_ScreenAreaShared = TRUE;
			}
			return TRUE;

		}

		break;
	}
	return 0;
}

// Set display/port settings to the correct state
void
vncProperties::InitPortSettings(HWND hwnd)
{
	BOOL bConnectSock = m_server->SockConnected();
	BOOL bAutoPort = m_server->AutoPortSelect();
	UINT port_rfb = m_server->GetPort();
	UINT port_http = m_server->GetHttpPort();
	int d1 = PORT_TO_DISPLAY(port_rfb);
	int d2 = HPORT_TO_DISPLAY(port_http);
	BOOL bValidDisplay = (d1 == d2 && d1 >= 0 && d1 <= 99);

	CheckDlgButton(hwnd, IDC_PORTNO_AUTO,
		(bAutoPort) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECDISPLAY,
		(!bAutoPort && bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECPORT,
		(!bAutoPort && !bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);

	EnableWindow(GetDlgItem(hwnd, IDC_PORTNO_AUTO), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECDISPLAY), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECPORT), bConnectSock);

	if (bValidDisplay) {
		SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
	} else {
		SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
	}
	SetDlgItemInt(hwnd, IDC_PORTRFB, port_rfb, FALSE);
	SetDlgItemInt(hwnd, IDC_PORTHTTP, port_http, FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO),
		bConnectSock && !bAutoPort && bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB),
		bConnectSock && !bAutoPort && !bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP),
		bConnectSock && !bAutoPort && !bValidDisplay);
}

// Functions to load & save the settings
LONG
vncProperties::LoadInt(HKEY key, LPCSTR valname, LONG defval)
{
	LONG pref;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

	if (RegQueryValueEx(key,
		valname,
		NULL,
		&type,
		(LPBYTE) &pref,
		&prefsize) != ERROR_SUCCESS)
		return defval;

	if (type != REG_DWORD)
		return defval;

	if (prefsize != sizeof(pref))
		return defval;

	return pref;
}

void
vncProperties::LoadPassword(HKEY key, char *buffer)
{
	DWORD type = REG_BINARY;
	int slen=MAXPWLEN;
	char inouttext[MAXPWLEN];

	// Retrieve the encrypted password
	if (RegQueryValueEx(key,
		"Password",
		NULL,
		&type,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen) != ERROR_SUCCESS)
		return;

	if (slen > MAXPWLEN)
		return;

	memcpy(buffer, inouttext, MAXPWLEN);
}

char *
vncProperties::LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 0;
	BYTE *buffer = 0;

	// Get the length of the AuthHosts string
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		NULL,
		&buflen) != ERROR_SUCCESS)
		return 0;

	if (type != REG_SZ)
		return 0;
	buffer = new BYTE[buflen];
	if (buffer == 0)
		return 0;

	// Get the AuthHosts string data
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		buffer,
		&buflen) != ERROR_SUCCESS) {
		delete [] buffer;
		return 0;
	}

	// Verify the type
	if (type != REG_SZ) {
		delete [] buffer;
		return 0;
	}

	return (char *)buffer;
}

void
vncProperties::Load(BOOL usersettings)
{
	char username[UNLEN+1];
	HKEY hkLocal, hkLocalUser, hkDefault;
	DWORD dw;

	// NEW (R3) PREFERENCES ALGORITHM
	// 1.	Look in HKEY_LOCAL_MACHINE/Software/ORL/WinVNC3/%username%
	//		for sysadmin-defined, user-specific settings.
	// 2.	If not found, fall back to %username%=Default
	// 3.	If AllowOverrides is set then load settings from
	//		HKEY_CURRENT_USER/Software/ORL/WinVNC3

	// GET THE CORRECT KEY TO READ FROM

	// Get the user name / service name
	if (!vncService::CurrentUser((char *)&username, sizeof(username)))
		return;

	// If there is no user logged on them default to SYSTEM
	if (strcmp(username, "") == 0)
		strcpy((char *)&username, "SYSTEM");

	// Try to get the machine registry key for WinVNC
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		WINVNC_REGISTRY_KEY,
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		return;

	// Now try to get the per-user local key
	if (RegOpenKeyEx(hkLocal,
		username,
		0, KEY_READ,
		&hkLocalUser) != ERROR_SUCCESS)
		hkLocalUser = NULL;

	// Get the default key
	if (RegCreateKeyEx(hkLocal,
		"Default",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL,
		&hkDefault,
		&dw) != ERROR_SUCCESS)
		hkDefault = NULL;

	// LOAD THE MACHINE-LEVEL PREFS

	// Logging/debugging prefs
	vnclog.Print(LL_INTINFO, VNCLOG("loading local-only settings\n"));
	vnclog.SetMode(LoadInt(hkLocal, "DebugMode", 0));
	vnclog.SetLevel(LoadInt(hkLocal, "DebugLevel", 0));

#ifdef HORIZONLIVE
	m_pref_SockConnect=false;
	m_pref_EnableRemoteInputs=TRUE;
	m_pref_PollUnderCursor=FALSE;
	m_pref_PollForeground=TRUE;
	m_pref_PollFullScreen=TRUE;
	m_pref_PollConsoleOnly=TRUE;
	m_pref_PollOnEventOnly=FALSE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;
	m_pref_FullScreen = FALSE;
	m_pref_WindowShared = TRUE;
	m_pref_ScreenAreaShared = FALSE;
#else
	// Disable Tray Icon
	m_server->SetDisableTrayIcon(LoadInt(hkLocal, "DisableTrayIcon", false));

	// Authentication required, loopback allowed, loopbackOnly
	m_server->SetLoopbackOnly(LoadInt(hkLocal, "LoopbackOnly", false));
	if (m_server->LoopbackOnly())
		m_server->SetLoopbackOk(true);
	else
		m_server->SetLoopbackOk(LoadInt(hkLocal, "AllowLoopback", false));
	m_server->SetHttpdEnabled(LoadInt(hkLocal, "EnableHTTPDaemon", true),
							  LoadInt(hkLocal, "EnableURLParams", false));
	m_server->SetAuthRequired(LoadInt(hkLocal, "AuthRequired", true));
	m_server->SetConnectPriority(LoadInt(hkLocal, "ConnectPriority", 2));
	if (!m_server->LoopbackOnly())
	{
		char *authhosts = LoadString(hkLocal, "AuthHosts");
		if (authhosts != 0) {
			m_server->SetAuthHosts(authhosts);
			delete [] authhosts;
		} else {
			m_server->SetAuthHosts(0);
		}
	} else {
		m_server->SetAuthHosts(0);
	}

	// LOAD THE USER PREFERENCES

	// Set the default user prefs
	vnclog.Print(LL_INTINFO, VNCLOG("clearing user settings\n"));
	m_pref_AutoPortSelect=TRUE;
	m_pref_PortNumber=RFB_PORT_OFFSET;
	m_pref_SockConnect=TRUE;
	m_pref_CORBAConn=FALSE;
	{
	    vncPasswd::FromClear crypt;
	    memcpy(m_pref_passwd, crypt, MAXPWLEN);
	}
	m_pref_QuerySetting=2;
	m_pref_QueryTimeout=30;
	m_pref_QueryAccept=FALSE;
	m_pref_QueryAllowNoPass=FALSE;
	m_pref_IdleTimeout=0;
	m_pref_EnableRemoteInputs=TRUE;
	m_pref_DisableLocalInputs=FALSE;
	m_pref_LockSettings=-1;
	m_pref_PollUnderCursor=FALSE;
	m_pref_PollForeground=TRUE;
	m_pref_PollFullScreen=FALSE;
	m_pref_PollConsoleOnly=TRUE;
	m_pref_PollOnEventOnly=FALSE;
	m_pref_RemoveWallpaper=TRUE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;
	m_pref_FullScreen = TRUE;
	m_pref_WindowShared = FALSE;
	m_pref_ScreenAreaShared = FALSE;
	m_pref_PriorityTime = 3;
	m_pref_LocalInputPriority = FALSE;

#endif

	// Load the local prefs for this user
	if (hkDefault != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("loading DEFAULT local settings\n"));
		LoadUserPrefs(hkDefault);
#if !defined HORIZONLIVE
		m_allowshutdown = LoadInt(hkDefault, "AllowShutdown", m_allowshutdown);
		m_allowproperties = LoadInt(hkDefault, "AllowProperties", m_allowproperties);
#endif	
	}

	// Are we being asked to load the user settings, or just the default local system settings?
	if (usersettings) {
		// We want the user settings, so load them!

		if (hkLocalUser != NULL)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
			LoadUserPrefs(hkLocalUser);
#if !defined HORIZONLIVE
			m_allowshutdown = LoadInt(hkLocalUser, "AllowShutdown", m_allowshutdown);
			m_allowproperties = LoadInt(hkLocalUser, "AllowProperties", m_allowproperties);
#endif
		}

		// Now override the system settings with the user's settings
		// If the username is SYSTEM then don't try to load them, because there aren't any...
		if (m_allowproperties && (strcmp(username, "SYSTEM") != 0))
		{
			HKEY hkGlobalUser;
			if (RegCreateKeyEx(HKEY_CURRENT_USER,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkGlobalUser, &dw) == ERROR_SUCCESS)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" global settings\n"), username);
				LoadUserPrefs(hkGlobalUser);
				RegCloseKey(hkGlobalUser);

				// Close the user registry hive so it can unload if required
				RegCloseKey(HKEY_CURRENT_USER);
			}
		}
	} else {
		vnclog.Print(LL_INTINFO, VNCLOG("bypassing user-specific settings (both local and global)\n"));
	}

	if (hkLocalUser != NULL) RegCloseKey(hkLocalUser);
	if (hkDefault != NULL) RegCloseKey(hkDefault);
	RegCloseKey(hkLocal);

	// Make the loaded settings active..
	ApplyUserPrefs();

	// Note whether we loaded the user settings or just the default system settings
	m_usersettings = usersettings;
}

void
vncProperties::LoadUserPrefs(HKEY appkey)
{
	// LOAD USER PREFS FROM THE SELECTED KEY

#if !defined HORIZONLIVE
	// Connection prefs
	m_pref_SockConnect=LoadInt(appkey, "SocketConnect", m_pref_SockConnect);
	m_pref_AutoPortSelect=LoadInt(appkey, "AutoPortSelect", m_pref_AutoPortSelect);
	m_pref_PortNumber=LoadInt(appkey, "PortNumber", m_pref_PortNumber);
	m_pref_HttpPortNumber=LoadInt(appkey, "HTTPPortNumber",
		DISPLAY_TO_HPORT(PORT_TO_DISPLAY(m_pref_PortNumber)));
	m_pref_BeepConnect=LoadInt(appkey, "BeepConnect", m_pref_BeepConnect);
	m_pref_BeepDisconnect=LoadInt(appkey, "BeepDisconnect", m_pref_BeepDisconnect);
	m_pref_IdleTimeout=LoadInt(appkey, "IdleTimeout", m_pref_IdleTimeout);
	
	m_pref_RemoveWallpaper=LoadInt(appkey, "RemoveWallpaper", m_pref_RemoveWallpaper);

	m_pref_LocalInputPriority=LoadInt(appkey, "LocalInputsPriority", m_pref_LocalInputPriority);
	m_pref_PriorityTime =LoadInt(appkey, "LocalInputsPriorityTime", m_pref_PriorityTime);

	
	// Connection querying settings
	m_pref_QuerySetting=LoadInt(appkey, "QuerySetting", m_pref_QuerySetting);
	m_pref_QueryTimeout=LoadInt(appkey, "QueryTimeout", m_pref_QueryTimeout);
	m_pref_QueryAccept=LoadInt(appkey, "QueryAccept", m_pref_QueryAccept);
	m_pref_QueryAllowNoPass=LoadInt(appkey, "QueryAllowNoPass", m_pref_QueryAllowNoPass);

	// Load the password
	LoadPassword(appkey, m_pref_passwd);
	
	// CORBA Settings
	m_pref_CORBAConn=LoadInt(appkey, "CORBAConnect", m_pref_CORBAConn);

	// Remote access prefs
	m_pref_EnableRemoteInputs=LoadInt(appkey, "InputsEnabled", m_pref_EnableRemoteInputs);
	m_pref_LockSettings=LoadInt(appkey, "LockSetting", m_pref_LockSettings);
	m_pref_DisableLocalInputs=LoadInt(appkey, "LocalInputsDisabled", m_pref_DisableLocalInputs);
#else
	strcpy(m_pref_LiveShareKey, m_server->GetLiveShareKey());

#endif
	// Polling prefs
	m_pref_PollUnderCursor=LoadInt(appkey, "PollUnderCursor", m_pref_PollUnderCursor);
	m_pref_PollForeground=LoadInt(appkey, "PollForeground", m_pref_PollForeground);
	m_pref_PollFullScreen=LoadInt(appkey, "PollFullScreen", m_pref_PollFullScreen);
	m_pref_PollConsoleOnly=LoadInt(appkey, "OnlyPollConsole", m_pref_PollConsoleOnly);
	m_pref_PollOnEventOnly=LoadInt(appkey, "OnlyPollOnEvent", m_pref_PollOnEventOnly);

	// screen area sharing prefs
	m_pref_FullScreen = m_server->FullScreen();
	m_pref_WindowShared = m_server->WindowShared();
	m_pref_ScreenAreaShared = m_server->ScreenAreaShared();
	
}

void
vncProperties::ApplyUserPrefs()
{
	// APPLY THE CACHED PREFERENCES TO THE SERVER
#if !defined HORIZONLIVE
	// Update the connection querying settings
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_server->SetQueryAccept(m_pref_QueryAccept);
	m_server->SetQueryAllowNoPass(m_pref_QueryAllowNoPass);
	m_server->SetAutoIdleDisconnectTimeout(m_pref_IdleTimeout);
	m_server->EnableRemoveWallpaper(m_pref_RemoveWallpaper);

	// Update the password
	m_server->SetPassword(m_pref_passwd);

	// Now change the listening port settings
	m_server->SetAutoPortSelect(m_pref_AutoPortSelect);
	if (!m_pref_AutoPortSelect)
		m_server->SetPorts(m_pref_PortNumber, m_pref_HttpPortNumber);
	
	// Set the beep options
	m_server->SetBeepConnect(m_pref_BeepConnect);
	m_server->SetBeepDisconnect(m_pref_BeepDisconnect);
	
	// Set the CORBA connection status
	m_server->CORBAConnect(m_pref_CORBAConn);

	// Remote access prefs
	m_server->EnableRemoteInputs(m_pref_EnableRemoteInputs);
	m_server->SetLockSettings(m_pref_LockSettings);
	m_server->DisableLocalInputs(m_pref_DisableLocalInputs);
	m_server->LocalInputPriority(m_pref_LocalInputPriority);
	m_server->SetDisableTime(m_pref_PriorityTime);

#endif

	m_server->SockConnect(m_pref_SockConnect);
	// Polling prefs
	m_server->PollUnderCursor(m_pref_PollUnderCursor);
	m_server->PollForeground(m_pref_PollForeground);
	m_server->PollFullScreen(m_pref_PollFullScreen);
	m_server->PollConsoleOnly(m_pref_PollConsoleOnly);
	m_server->PollOnEventOnly(m_pref_PollOnEventOnly);
	
	m_server->FullScreen(m_pref_FullScreen);
	m_server->WindowShared(m_pref_WindowShared);
	m_server->ScreenAreaShared(m_pref_ScreenAreaShared);

}

void
vncProperties::SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void
vncProperties::SavePassword(HKEY key, char *buffer)
{
	RegSetValueEx(key, "Password", 0, REG_BINARY, (LPBYTE) buffer, MAXPWLEN);
}

void
vncProperties::Save()
{
	HKEY appkey;
	DWORD dw;

	if (!m_allowproperties)
		return;

	// NEW (R3) PREFERENCES ALGORITHM
	// The user's prefs are only saved if the user is allowed to override
	// the machine-local settings specified for them.  Otherwise, the
	// properties entry on the tray icon menu will be greyed out.

	// GET THE CORRECT KEY TO READ FROM

	// Have we loaded user settings, or system settings?
	if (m_usersettings) {
		// Verify that we know who is logged on
		char username[UNLEN+1];
		if (!vncService::CurrentUser((char *)&username, sizeof(username)))
			return;
		if (strcmp(username, "") == 0)
			return;

		// Try to get the per-user, global registry key for WinVNC
		if (RegCreateKeyEx(HKEY_CURRENT_USER,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS)
			return;
	} else {
		// Try to get the default local registry key for WinVNC
		HKEY hkLocal;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS) {
			MessageBox(NULL, "MB1", "WVNC", MB_OK);
			return;
		}
		if (RegCreateKeyEx(hkLocal,
			"Default",
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS) {
			RegCloseKey(hkLocal);
			return;
		}
		RegCloseKey(hkLocal);
	}

	// SAVE PER-USER PREFS IF ALLOWED
	SaveUserPrefs(appkey);

	RegCloseKey(appkey);

	// Close the user registry hive, to allow it to unload if reqd
	RegCloseKey(HKEY_CURRENT_USER);
}

void
vncProperties::SaveUserPrefs(HKEY appkey)
{
#if !defined HORIZONLIVE

	// SAVE THE PER USER PREFS
	vnclog.Print(LL_INTINFO, VNCLOG("saving current settings to registry\n"));

	// Connection prefs
	SaveInt(appkey, "SocketConnect", m_server->SockConnected());
	SaveInt(appkey, "AutoPortSelect", m_server->AutoPortSelect());
	if (!m_server->AutoPortSelect()) {
		SaveInt(appkey, "PortNumber", m_server->GetPort());
		SaveInt(appkey, "HTTPPortNumber", m_server->GetHttpPort());
	}
	SaveInt(appkey, "InputsEnabled", m_server->RemoteInputsEnabled());
	SaveInt(appkey, "LocalInputsDisabled", m_server->LocalInputsDisabled());
	SaveInt(appkey, "IdleTimeout", m_server->AutoIdleDisconnectTimeout());
	
	SaveInt(appkey, "LocalInputsPriority", m_server->LocalInputPriority());
	SaveInt(appkey, "LocalInputsPriorityTime", m_server->DisableTime());
	
	// Connection querying settings
	SaveInt(appkey, "QuerySetting", m_server->QuerySetting());
	SaveInt(appkey, "QueryTimeout", m_server->QueryTimeout());

	// Save the password
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	SavePassword(appkey, passwd);

#if(defined(_CORBA))
	// Don't save the CORBA enabled flag if CORBA is not compiled in!
	SaveInt(appkey, "CORBAConnect", m_server->CORBAConnected());
#endif

	// Polling prefs
	SaveInt(appkey, "PollUnderCursor", m_server->PollUnderCursor());
	SaveInt(appkey, "PollForeground", m_server->PollForeground());
	SaveInt(appkey, "PollFullScreen", m_server->PollFullScreen());

	SaveInt(appkey, "OnlyPollConsole", m_server->PollConsoleOnly());
	SaveInt(appkey, "OnlyPollOnEvent", m_server->PollOnEventOnly());
#endif
}

LRESULT CALLBACK vncProperties::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hNewImage,hOldImage;
	HCURSOR hNewCursor,hOldCursor;
	vncProperties* pDialog=(vncProperties*) GetWindowLong(hWnd,GWL_USERDATA);

	switch (message)
	{
	
	case WM_SETCURSOR :
		if (HIWORD(lParam)==WM_LBUTTONDOWN)
		{
			SetCapture(hWnd);
			hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP2));
			hOldImage=(HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
			DeleteObject(hOldImage);
			hNewCursor=(HCURSOR)LoadImage(hAppInstance,MAKEINTRESOURCE(IDC_CURSOR1),IMAGE_CURSOR,32,32,LR_DEFAULTCOLOR);
			hOldCursor=SetCursor(hNewCursor);
			DestroyCursor(hOldCursor);
			pDialog->m_bCaptured=TRUE;
		}
		break;
	
	case WM_LBUTTONUP:
		ReleaseCapture();
		
		if (pDialog->m_KeepHandle!=NULL)
		{
			// We need to remove frame
			DrawFrameAroundWindow(pDialog->m_KeepHandle);
			pDialog->m_server->SetWindowShared(pDialog->m_KeepHandle);
			// No more need
			pDialog->m_KeepHandle=NULL;
		} else {
			pDialog->m_server->SetWindowShared(NULL);
		}
		
		hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage=(HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		hNewCursor=LoadCursor(hAppInstance,MAKEINTRESOURCE(IDC_ARROW));
		hOldCursor=SetCursor(hNewCursor);
		DestroyCursor(hOldCursor);
		pDialog->m_bCaptured=FALSE;
		break;
	
	case WM_MOUSEMOVE:
		if (pDialog->m_bCaptured)
		{
			HWND hParent=::GetParent(hWnd);
			POINTS pnt;
			POINT pnt1;
			pnt=MAKEPOINTS(lParam);
			pnt1.x=pnt.x;
			pnt1.y=pnt.y;
			ClientToScreen(hWnd,&pnt1);
			HWND hMouseWnd=::WindowFromPoint(pnt1);
			if (pDialog->m_KeepHandle!=hMouseWnd)
			{
				// New Windows Handle
				// Was KeepHndle A Real Window ?
				if (pDialog->m_KeepHandle!=NULL)
				{
					// We need to remove frame
					vncProperties::DrawFrameAroundWindow(pDialog->m_KeepHandle);
				}
				pDialog->m_KeepHandle=hMouseWnd;
				if (!IsChild(hParent,hMouseWnd) && (hMouseWnd!=hParent))
				{
					pDialog->SetWindowCaption(hMouseWnd);
					vncProperties::DrawFrameAroundWindow(hMouseWnd);
				} else {	// My Window
					pDialog->m_KeepHandle=NULL;
					pDialog->SetWindowCaption(NULL);					}
				}
			}
			break;
	case WM_PAINT:
	case STM_SETIMAGE:
		
		return CallWindowProc( (WNDPROC) pDialog->m_OldBmpWndProc,  hWnd, message, wParam, lParam);
	
	default:
		return DefWindowProc( hWnd, message, wParam, lParam);
	}
	return 0;
}


void vncProperties::DrawFrameAroundWindow(HWND hWnd)
{
	HDC hWindowDc=::GetWindowDC(hWnd);
	HBRUSH hBrush=CreateSolidBrush(RGB(0,0,0));
	HRGN Rgn=CreateRectRgn(0,0,1,1);
	int iRectResult=GetWindowRgn(hWnd,Rgn);
	if (iRectResult==ERROR || iRectResult==NULLREGION || Rgn==NULL)
	{
		RECT rect;
		GetWindowRect(hWnd,&rect);
		OffsetRect(&rect,-rect.left,-rect.top);
		Rgn=CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	}
	
	SetROP2(hWindowDc,R2_MERGEPENNOT);
	FrameRgn(hWindowDc,Rgn,hBrush,3,3);
	::DeleteObject(Rgn);
	::DeleteObject(hBrush);
    ::ReleaseDC(hWnd,hWindowDc);
}

void vncProperties::SetWindowCaption(HWND hWnd)
{
	char strWindowText[256];
	if (hWnd == NULL) 
	{
		strcpy(strWindowText, "* No Window Selected *");
	} else {
		GetWindowText(hWnd, strWindowText, sizeof(strWindowText));
		if (!strWindowText[0]) 
		{
			int bytes = sprintf(strWindowText, "0x%x ", hWnd);
			GetClassName(hWnd, strWindowText + bytes,
				sizeof(strWindowText) - bytes);
		}
	}
	::SetWindowText(hNameAppli, strWindowText);
}

