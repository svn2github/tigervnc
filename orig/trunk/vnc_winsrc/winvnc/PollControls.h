// PollControls.h: interface for the PollControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef POLLCONTROLS_H__
#define POLLCONTROLS_H__

#pragma once

#include "resource.h"
#include "vncServer.h"

class PollControls  
{
public:
	PollControls(HWND hwnd, vncServer *server);
	void ApplyControlsContents(HWND hwnd);	
	void EnablePollCustom(HWND hwnd);
	void EnablePollFullScreen(HWND hwnd);
	virtual ~PollControls();
private:
	void EnablePollingTimer(HWND hwnd, bool enable);
	vncServer *		m_server;
};

#endif 
