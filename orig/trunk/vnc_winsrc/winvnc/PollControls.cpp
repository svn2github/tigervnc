// PollControls.cpp: implementation of the PollControls class.
//
//////////////////////////////////////////////////////////////////////

#include "PollControls.h"

PollControls::PollControls(HWND hwnd, vncServer *server)
{	
	m_hwnd = hwnd;
	m_server = server;

	SetChecked(IDC_POLL_FULLSCREEN, m_server->PollFullScreen());
	SetChecked(IDC_POLL_FOREGROUND, m_server->PollForeground());
	SetChecked(IDC_POLL_UNDER_CURSOR, m_server->PollUnderCursor());
	SetChecked(IDC_CONSOLE_ONLY, m_server->PollConsoleOnly());
	SetChecked(IDC_ONEVENT_ONLY, m_server->PollOnEventOnly());
	SetChecked(IDC_DONT_SET_HOOKS, m_server->DontSetHooks());
	SetChecked(IDC_DONT_USE_DRIVER, m_server->DontUseDriver());
	SetDlgItemInt(m_hwnd, IDC_POLLING_CYCLE, m_server->GetPollingCycle(), FALSE);

	if (m_server->DesktopActive()) {
		if (m_server->DriverActive()) {
			SetDlgItemText(hwnd, IDC_STATIC_DRVINFO, "Driver is in use");
		} else {
			SetDlgItemText(hwnd, IDC_STATIC_DRVINFO, "Driver is not used");
		}
	}

	Validate();
}

void PollControls::Validate()
{
	BOOL full_polling =
		IsChecked(IDC_POLL_FULLSCREEN);
	BOOL window_polling =
		IsChecked(IDC_POLL_FOREGROUND) || IsChecked(IDC_POLL_UNDER_CURSOR);	

	Enable(IDC_POLL_FOREGROUND,   !full_polling);
	Enable(IDC_POLL_UNDER_CURSOR, !full_polling);

	Enable(IDC_CONSOLE_ONLY,      !full_polling && window_polling);
	Enable(IDC_ONEVENT_ONLY,      !full_polling && window_polling);

	Enable(IDC_POLLING_CYCLE,     full_polling || window_polling);
	Enable(IDC_STATIC_PCYCLE,     full_polling || window_polling);
	Enable(IDC_STATIC_MS,         full_polling || window_polling);

	Enable(IDC_DONT_SET_HOOKS,    full_polling);
}

void PollControls::Apply()
{
	m_server->PollFullScreen(IsChecked(IDC_POLL_FULLSCREEN));
	m_server->PollForeground(IsChecked(IDC_POLL_FOREGROUND));
	m_server->PollUnderCursor(IsChecked(IDC_POLL_UNDER_CURSOR));
	m_server->PollConsoleOnly(IsChecked(IDC_CONSOLE_ONLY));
	m_server->PollOnEventOnly(IsChecked(IDC_ONEVENT_ONLY));

	// This should appear AFTER calling m_server->PollFullScreen(...)
	m_server->DontSetHooks(IsChecked(IDC_DONT_SET_HOOKS));
	m_server->DontUseDriver(IsChecked(IDC_DONT_USE_DRIVER));

	BOOL success;
	UINT pollingCycle = GetDlgItemInt(m_hwnd, IDC_POLLING_CYCLE, &success, TRUE);
	if (success)
		m_server->SetPollingCycle(pollingCycle);
}

