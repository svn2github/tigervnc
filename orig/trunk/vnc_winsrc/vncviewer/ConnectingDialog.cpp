//  Copyright (C) 2003 Constantin Kaplinsky. All Rights Reserved.
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

// ConnectingDialog

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ConnectingDialog.h"

ConnectingDialog::ConnectingDialog(HINSTANCE hInst, const char *vnchost)
{
	if (vnchost != NULL) {
		m_vnchost = strdup(vnchost);
	} else {
		m_vnchost = NULL;
	}
	m_hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONNECTING_DIALOG),
							   NULL, (DLGPROC)ConnectingDlgProc,
							   (LPARAM)this);
	SetStatus("Initialization");
	if (m_hwnd != NULL) {
		ShowWindow(m_hwnd, SW_SHOW);
		UpdateWindow(m_hwnd);
	}
}

ConnectingDialog::~ConnectingDialog()
{
	Close();
	if (m_vnchost != NULL) {
		free(m_vnchost);
	}
}

void
ConnectingDialog::SetStatus(const char *msg)
{
	if (m_hwnd != NULL) {
		char buf[256];
		sprintf(buf, "Status: %.240s.", msg);
		SetDlgItemText(m_hwnd, IDC_STATUS_STATIC, buf);
	}
}

void
ConnectingDialog::Close()
{
	if (m_hwnd != NULL) {
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

LRESULT CALLBACK
ConnectingDialog::ConnectingDlgProc(HWND hwnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam)
{
	ConnectingDialog *_this =
		(ConnectingDialog *)GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		SetForegroundWindow(hwnd);
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		_this = (ConnectingDialog *)lParam;
		if (_this->m_vnchost != NULL) {
			char buf[256];
			if (_this->m_vnchost[0] != '\0') {
				sprintf(buf, "Connecting to %.200s ...", _this->m_vnchost);
			} else {
				sprintf(buf, "Accepting reverse connection...");
			}
			SetDlgItemText(hwnd, IDC_CONNECTING_STATIC, buf);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCLOSE:
			_this->Close();
			return TRUE;
		}
		break;
	case WM_CLOSE:
		_this->Close();
		return TRUE;
	}
	return FALSE;
}

