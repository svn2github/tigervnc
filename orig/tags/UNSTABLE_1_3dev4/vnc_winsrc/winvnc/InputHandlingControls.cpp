// InputHandlingControls.cpp: implementation of the InputHandlingControls class.
//
//////////////////////////////////////////////////////////////////////

#include "InputHandlingControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputHandlingControls::InputHandlingControls(HWND hwnd, vncServer *server)
{
	m_server = server;

	// Remote input settings
	HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(m_server->RemoteInputsEnabled()),
				0);

	// Local input settings
	HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	SendMessage(hDisableLocalInputs,
				BM_SETCHECK,
				m_server->LocalInputsDisabled(),
				0);

	// Local input prioity settings
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	SendMessage(hRemoteDisable,
				BM_SETCHECK,
				m_server->LocalInputPriority(),
				0);

	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	SetDlgItemInt(hwnd, IDC_DISABLE_TIME, m_server->DisableTime(), FALSE);
			
	EnableRemote(hwnd);
	EnableInputs(hwnd);
}
void InputHandlingControls::ApplyInputsControlsContents(HWND hwnd)
{
	// Remote input stuff
	HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	m_server->EnableRemoteInputs(SendMessage(hEnableRemoteInputs,
								BM_GETCHECK, 0, 0) != BST_CHECKED);
	// Local input stuff
	HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	m_server->DisableLocalInputs(SendMessage(hDisableLocalInputs,
								BM_GETCHECK, 0, 0) == BST_CHECKED);
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	m_server->LocalInputPriority(SendMessage(hRemoteDisable,
								BM_GETCHECK, 0, 0) == BST_CHECKED);

	BOOL success;
	UINT disabletime = GetDlgItemInt(hwnd, IDC_DISABLE_TIME, &success, TRUE);
	if (success)
		m_server->SetDisableTime(disabletime);
}

void InputHandlingControls::EnableInputs(HWND hwnd)
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
	EnableTime(hwnd, enabled & !enabl);
				
	if (!enabled)
		SendMessage(hRemoteDisable, BM_SETCHECK, enabled, 0);
}
void InputHandlingControls::EnableRemote(HWND hwnd)
{
	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	bool enabled = (SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) == BST_CHECKED);
	EnableTime(hwnd, enabled);
	if (enabled) {
		SetFocus(hDisableTime);
		SendMessage(hDisableTime, EM_SETSEL, 0, (LPARAM)-1);
	}
}
void InputHandlingControls::EnableTime(HWND hwnd, bool enable)
{
	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	HWND hTimeLebel = GetDlgItem(hwnd, IDC_TIMEOUT_LABEL);
	HWND hSecondLebel = GetDlgItem(hwnd, IDC_SECONDS_LABEL);
	EnableWindow(hDisableTime, enable);
	EnableWindow(hTimeLebel, enable);
	EnableWindow(hSecondLebel, enable);
}
InputHandlingControls::~InputHandlingControls()
{

}
