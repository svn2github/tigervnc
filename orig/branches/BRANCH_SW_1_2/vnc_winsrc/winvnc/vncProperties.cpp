//  Copyright (C) 2001 HorizonLive.com, Inc. All Rights Reserved.
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
	if (m_pMatchWindow!=NULL)
		delete m_pMatchWindow;

}

// Initialisation
BOOL
vncProperties::Init(vncServer *server)
{
	// Save the server pointer
	m_server = server;
	
	m_inadvanced = FALSE;
	
	// Load the settings from the registry
	Load(TRUE);

	// If the password is empty then always show a dialog
/*	char passwd[MAXPWLEN];
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
						MessageBox(NULL, NO_PASSWD_NO_LOGON_WARN,
									"WinVNC Error",
									MB_OK | MB_ICONEXCLAMATION);
						Show(TRUE, FALSE);
					} else {
						Show(TRUE, TRUE);
					}
				}
			}
	}
	*/		
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
			if (strcmp(username, "") == 0) {
				MessageBox(NULL, NO_CURRENT_USER_ERR, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
				return;
			}
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

			// Set the dialog box's title to indicate which Properties we're editting
				SetWindowText(hwnd, "LiveShare Settings");
			

			
			HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
			_this->m_OldBmpWndProc=GetWindowLong(bmp_hWnd,GWL_WNDPROC);
			SetWindowLong(bmp_hWnd,GWL_WNDPROC,(LONG)BmpWndProc);
			SetWindowLong(bmp_hWnd, GWL_USERDATA, (LONG)_this);
		    HBITMAP hNewImage,hOldImage;

			_this->hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);

			if (_this->m_pref_FullScreen) {
				
				// hide shared area window
				if (_this->m_pMatchWindow!=NULL) 
					_this->m_pMatchWindow->Hide();

				// disable window select stuff
				EnableWindow(bmp_hWnd,FALSE);
				hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
				hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
				DeleteObject(hOldImage);
				::SetWindowText(_this->hNameAppli,"Full screen shared");                                                                                                  
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
				::SetWindowText(_this->hNameAppli,"Screen Shared");                                                                                          

				if (_this->m_pMatchWindow == NULL) {
					RECT temp;
					temp = _this->m_server->getSharedRect();
					_this->m_pMatchWindow=new CMatchWindow(_this->m_server,temp.left+5,temp.top+5,temp.right,temp.bottom);
					_this->m_pMatchWindow->CanModify(TRUE);
				}
				_this->m_pMatchWindow->Show();
			}
				

			// Initialise the properties controls
			// Remote input settings
			HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
			SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(_this->m_server->RemoteInputsEnabled()),
				0);

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

				// Remote input stuff
				HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
				_this->m_server->EnableRemoteInputs(
					SendMessage(hEnableRemoteInputs, BM_GETCHECK, 0, 0) != BST_CHECKED
					);
				_this->m_server->SetKeyboardEnabled(
					SendMessage(hEnableRemoteInputs, BM_GETCHECK, 0, 0) != BST_CHECKED
					);

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
				_this->EnableControls(FALSE, hwnd, _this);
				_this->m_inadvanced = TRUE;
				vncAdvancedProperties *aprop = new vncAdvancedProperties();
				if (aprop->Init(_this->m_server))
				{
					aprop->Show(TRUE, _this->m_usersettings);
				}
				//aprop->DoDialog();
				SetForegroundWindow(hwnd);
				_this->m_inadvanced = FALSE;
				_this->EnableControls(TRUE, hwnd, _this);
				omni_thread::sleep(0, 200000000);
			}
			return TRUE;


		case IDC_FULLSCREEN:
			{	
				HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
			    HBITMAP hNewImage,hOldImage;

				_this->m_pref_FullScreen = TRUE;
				_this->m_pref_WindowShared = FALSE;
				_this->m_pref_ScreenAreaShared = FALSE;
				_this->hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);
				EnableWindow(_this->hNameAppli, FALSE);
				::SetWindowText(_this->hNameAppli,"Full Screen Shared");
			
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
				::SetWindowText(_this->hNameAppli,"Screen Area Shared");
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

void
vncProperties::EnableControls(BOOL state, HWND hwnd, vncProperties *_this)
{
	EnableWindow(hwnd, state);
	//return;

	HWND hOk = GetDlgItem(hwnd, IDOK);
	EnableWindow(hOk, state);
	HWND hCancel = GetDlgItem(hwnd, IDCANCEL);
	EnableWindow(hCancel, state);
	HWND hApply = GetDlgItem(hwnd, IDC_APPLY);
	EnableWindow(hApply, state);
	HWND hAdvanced = GetDlgItem(hwnd, IDADVANCED);
	EnableWindow(hAdvanced, state);

	HWND hCon = GetDlgItem(hwnd, IDC_CONNECT_BORDER);
	EnableWindow(hCon, state);
	HWND hUp = GetDlgItem(hwnd, IDC_UPDATE_BORDER);
	EnableWindow(hUp, state);
	

	// Remote input settings
	HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	EnableWindow(hEnableRemoteInputs, state);

/*	// Set the polling options
	HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
	EnableWindow(hPollFullScreen, state);

	HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
	EnableWindow(hPollForeground, state);

	HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
	EnableWindow(hPollUnderCursor, state);

	HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
	EnableWindow(hPollConsoleOnly,
		state && 
		((SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) || 
		 (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED))
		);

	HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
	EnableWindow(hPollOnEventOnly,
		state && 
		((SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) || 
		 (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED))
		);
*/
	HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd,state);

	HWND hFullScreen = GetDlgItem(hwnd,IDC_FULLSCREEN);
	EnableWindow(hFullScreen,state);

	HWND hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);
	EnableWindow(hNameAppli, state);

	HWND hWindowShared = GetDlgItem(hwnd,IDC_WINDOW);
	EnableWindow(hWindowShared,state);

	HWND hScreenArea = GetDlgItem(hwnd,IDC_SCREEN);
	EnableWindow(hScreenArea,state);


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


	// LOAD THE USER PREFERENCES

	// Set the default user prefs
	vnclog.Print(LL_INTINFO, VNCLOG("clearing user settings\n"));
	m_pref_SockConnect=false;

	/*{
	    vncPasswd::FromClear crypt;
	    memcpy(m_pref_passwd, crypt, MAXPWLEN);
	}*/
	m_pref_EnableRemoteInputs=TRUE;
	m_pref_PollUnderCursor=FALSE;
	m_pref_PollForeground=TRUE;
	m_pref_PollFullScreen=FALSE;
	m_pref_PollConsoleOnly=TRUE;
	m_pref_PollOnEventOnly=FALSE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;
	m_pref_FullScreen = FALSE;
	m_pref_WindowShared = FALSE;
	m_pref_ScreenAreaShared = TRUE;
	
	// Load the local prefs for this user
	if (hkDefault != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("loading DEFAULT local settings\n"));
		LoadUserPrefs(hkDefault);
	}

	// Are we being asked to load the user settings, or just the default local system settings?
	if (usersettings) {
		// We want the user settings, so load them!

		if (hkLocalUser != NULL)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
			LoadUserPrefs(hkLocalUser);
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

	// Remote access prefs
	m_pref_EnableRemoteInputs=LoadInt(appkey, "InputsEnabled", m_pref_EnableRemoteInputs);
	
	// Polling prefs
	m_pref_PollUnderCursor=LoadInt(appkey, "PollUnderCursor", m_pref_PollUnderCursor);
	m_pref_PollForeground=LoadInt(appkey, "PollForeground", m_pref_PollForeground);
	m_pref_PollFullScreen=LoadInt(appkey, "PollFullScreen", m_pref_PollFullScreen);
	m_pref_PollConsoleOnly=LoadInt(appkey, "OnlyPollConsole", m_pref_PollConsoleOnly);
	m_pref_PollOnEventOnly=LoadInt(appkey, "OnlyPollOnEvent", m_pref_PollOnEventOnly);

	//m_pref_FullScreen = LoadInt(appkey, "FullScreenShared", m_pref_FullScreen);
	//m_pref_WindowShared = LoadInt(appkey, "WindowShared", m_pref_WindowShared);
	//m_pref_ScreenAreaShared = LoadInt(appkey, "ScreenAreaShared", m_pref_ScreenAreaShared);
	m_server->FullScreen();
	m_server->WindowShared();
	m_server->ScreenAreaShared();

}

void
vncProperties::ApplyUserPrefs()
{
	// APPLY THE CACHED PREFERENCES TO THE SERVER

	m_server->SockConnect(m_pref_SockConnect);

	m_server->EnableRemoteInputs(m_pref_EnableRemoteInputs);
		
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
	// SAVE THE PER USER PREFS
	vnclog.Print(LL_INTINFO, VNCLOG("saving current settings to registry\n"));

	// Connection prefs
	SaveInt(appkey, "InputsEnabled", m_server->RemoteInputsEnabled());
	//SaveInt(appkey, "FullScreenShared", m_server->FullScreen());
	//SaveInt(appkey, "WindowShared", m_server->WindowShared());
	//SaveInt(appkey, "ScreenAreaShared", m_server->ScreenAreaShared());
	

}


LRESULT CALLBACK vncProperties::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	HBITMAP hNewImage,hOldImage;
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
					}
					else
					{	// My Window
						pDialog->m_KeepHandle=NULL;
						pDialog->SetWindowCaption(NULL);

					}
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

	if (hWnd == NULL) {
		strcpy(strWindowText, "NO WINDOW");
	} else {
		GetWindowText(hWnd, strWindowText, sizeof(strWindowText));
		if (!strWindowText[0]) {
			int bytes = sprintf(strWindowText, "0x%x ", hWnd);
			GetClassName(hWnd, strWindowText + bytes,
						 sizeof(strWindowText) - bytes);
		}
	}
	::SetWindowText(hNameAppli, strWindowText);
}

