// PollControls.cpp: implementation of the PollControls class.
//
//////////////////////////////////////////////////////////////////////

#include "PollControls.h"

PollControls::PollControls(HWND hwnd, vncServer *server)
{	
	m_server = server;

	HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
	SendMessage(hPollFullScreen,
				BM_SETCHECK,
				m_server->PollFullScreen(),
				0);

	HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
	SendMessage(hPollForeground,
				BM_SETCHECK,
				m_server->PollForeground(),
				0);
	
	HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
	SendMessage(hPollUnderCursor,
				BM_SETCHECK,
				m_server->PollUnderCursor(),
				0);
	
	HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
	SendMessage(hPollConsoleOnly,
				BM_SETCHECK,
				m_server->PollConsoleOnly(),
				0);
	
	HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
	SendMessage(hPollOnEventOnly,
				BM_SETCHECK,
				m_server->PollOnEventOnly(),
				0);
	
	HWND hPollingTimer = GetDlgItem(hwnd, IDC_POLL_TIMER);
	SetDlgItemInt(hwnd, IDC_POLL_TIMER, m_server->GetPollingTimer(), FALSE);
	
	HWND hDontSetHooks = GetDlgItem(hwnd, IDC_DONT_SET_HOOKS);
	SendMessage(hDontSetHooks, BM_SETCHECK, m_server->DontSetHooks(), 0);
	EnablePollFullScreen(hwnd);
	EnablePollCustom(hwnd);
	
}
void PollControls::ApplyControlsContents(HWND hwnd)
{
	HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
	m_server->PollFullScreen(SendMessage(hPollFullScreen,
							BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
	m_server->PollForeground(SendMessage(hPollForeground,
							BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
	m_server->PollUnderCursor(SendMessage(hPollUnderCursor,
							BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
	m_server->PollConsoleOnly(SendMessage(hPollConsoleOnly,
							BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
	m_server->PollOnEventOnly(SendMessage(hPollOnEventOnly,
							BM_GETCHECK, 0, 0) == BST_CHECKED);

	// This should appear AFTER calling m_server->PollFullScreen(...)
	HWND hDontSetHooks = GetDlgItem(hwnd, IDC_DONT_SET_HOOKS);
	m_server->DontSetHooks(SendMessage(hDontSetHooks,
							BM_GETCHECK, 0, 0) == BST_CHECKED);
	BOOL success;
	UINT pollingtimer = GetDlgItemInt(hwnd, IDC_POLL_TIMER, &success, TRUE);
	if (success)
		m_server->SetPollingTimer(pollingtimer);
}

void PollControls::EnablePollCustom(HWND hwnd)
{
	HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
	HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
	HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);

	// Determine whether to enable the modifier options
	BOOL enabled = (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED) ||
					(SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
	EnableWindow(hPollConsoleOnly, enabled);

	HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
	EnableWindow(hPollOnEventOnly, enabled);
	
	EnablePollingTimer(hwnd, enabled || (SendMessage(hPollFullScreen,
						BM_GETCHECK, 0, 0) == BST_CHECKED));
}
void PollControls::EnablePollFullScreen(HWND hwnd)
{
	
	HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
	HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
	HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
				
	BOOL full_polling = (SendMessage(hPollFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED);
	EnableWindow(hPollForeground, !full_polling);
	EnableWindow(hPollUnderCursor, !full_polling);
				
	BOOL window_polling = (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED) ||
					(SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
	EnableWindow(hPollConsoleOnly, !full_polling && window_polling);

	HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
	EnableWindow(hPollOnEventOnly, !full_polling && window_polling);
	
	EnablePollingTimer(hwnd, full_polling || window_polling);

	HWND hDontSetHooks = GetDlgItem(hwnd, IDC_DONT_SET_HOOKS);
	EnableWindow(hDontSetHooks, full_polling);
}
void PollControls::EnablePollingTimer(HWND hwnd, bool enable)
{
	HWND hPollingTimer = GetDlgItem(hwnd, IDC_POLL_TIMER);
	HWND hStaticPCycle = GetDlgItem(hwnd, IDC_STATIC_PCYCLE);
	HWND hStaticMs = GetDlgItem(hwnd, IDC_STATIC_MS);
	EnableWindow(hPollingTimer, enable);
	EnableWindow(hStaticPCycle, enable);
	EnableWindow(hStaticMs, enable);
}
PollControls::~PollControls()
{

}
