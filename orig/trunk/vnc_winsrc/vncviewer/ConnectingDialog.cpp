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

class ConnDialogThread : public omni_thread
{
public:
	void Init(HINSTANCE hInst, const char *vnchost = NULL);
	virtual ~ConnDialogThread();
	virtual void *run_undetached(void *);
	void SetStatus(const char *msg);
	void Close();

private:
	void ShowStatus(const char *msg);
	static LRESULT CALLBACK DlgProc(HWND hwnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam);
	HINSTANCE m_hInst;
	HWND m_hwnd;
	char *m_vnchost;
	char *m_status;
	omni_mutex m_mutex;
};

void
ConnDialogThread::Init(HINSTANCE hInst, const char *vnchost)
{
	m_hInst = hInst;
	m_hwnd = NULL;
	if (vnchost != NULL) {
		m_vnchost = strdup(vnchost);
	} else {
		m_vnchost = NULL;
	}
	m_status = NULL;
	start_undetached();
}

ConnDialogThread::~ConnDialogThread()
{
	if (m_vnchost != NULL)
		free(m_vnchost);
	if (m_status != NULL)
		free(m_status);
}

void *
ConnDialogThread::run_undetached(void *)
{
	DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_CONNECTING_DIALOG),
				   NULL, (DLGPROC)DlgProc, (LPARAM)this);
	m_hwnd = NULL;
	return NULL;
}

void
ConnDialogThread::SetStatus(const char *msg)
{
	omni_mutex_lock l(m_mutex);
	if (m_hwnd != NULL) {
		ShowStatus(msg);
	} else {
		if (m_status != NULL)
			free(m_status);
		m_status = strdup(msg);
	}
}

void
ConnDialogThread::Close()
{
	if (m_hwnd != NULL)
		SendMessage(m_hwnd, WM_CLOSE, 0, 0);
	try {
		void *p;
		join(&p);
	} catch (omni_thread_invalid) {}
}

void
ConnDialogThread::ShowStatus(const char *msg)
{
	char buf[256];
	sprintf(buf, "Status: %.240s.", msg);
	SetDlgItemText(m_hwnd, IDC_STATUS_STATIC, buf);
	if (m_status != NULL) {
		free(m_status);
		m_status = NULL;
	}
}

LRESULT CALLBACK
ConnDialogThread::DlgProc(HWND hwnd, UINT uMsg,
						  WPARAM wParam, LPARAM lParam)
{
	ConnDialogThread *_this =
		(ConnDialogThread *)GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		SetForegroundWindow(hwnd);
		_this = (ConnDialogThread *)lParam;
		{
			omni_mutex_lock l(_this->m_mutex);
			_this->m_hwnd = hwnd;
			if (_this->m_status != NULL)
				_this->ShowStatus(_this->m_status);
		}
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
			EndDialog(hwnd, IDCLOSE);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, IDCLOSE);
		return TRUE;
	}
	return FALSE;
}

//
// ConnectingDialog class implementation.
//

ConnectingDialog::ConnectingDialog(HINSTANCE hInst, const char *vnchost)
{
	m_thread = new ConnDialogThread;
	m_thread->Init(hInst, vnchost);
}

ConnectingDialog::~ConnectingDialog()
{
	m_thread->Close();
}

void
ConnectingDialog::SetStatus(const char *msg)
{
	m_thread->SetStatus(msg);
}

