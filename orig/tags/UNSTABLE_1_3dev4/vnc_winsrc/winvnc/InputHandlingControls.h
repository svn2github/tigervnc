// InputHandlingControls.h: interface for the InputHandlingControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INPUTHANDLINGCONTROLS_H__
#define INPUTHANDLINGCONTROLS_H__


#pragma once

#include "resource.h"
#include "vncServer.h"

class InputHandlingControls  
{
public:
	InputHandlingControls(HWND hwnd, vncServer *server);
	void ApplyInputsControlsContents(HWND hwnd);
	void EnableInputs(HWND hwnd);
	void EnableRemote(HWND hwnd);
	virtual ~InputHandlingControls();
private:
	void EnableTime(HWND hwnd, bool enable);
	vncServer *		m_server;
};

#endif 
