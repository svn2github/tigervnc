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
	SetDlgItemInt(m_hwnd, IDC_POLL_TIMER, m_server->GetPollingTimer(), FALSE);

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

	Enable(IDC_POLL_TIMER,        full_polling || window_polling);
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

	BOOL success;
	UINT pollingTimer = GetDlgItemInt(m_hwnd, IDC_POLL_TIMER, &success, TRUE);
	if (success)
		m_server->SetPollingTimer(pollingTimer);
}

