// VNCHelp.h: interface for the VNCHelp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VNCHELP_H__
#define VNCHELP_H__

#pragma once

class VNCHelp  
{
public:
	VNCHelp();
	DWORD dwCookie ;
	void VNCHelp::Popup( LPARAM lParam);
	BOOL VNCHelp::TransMess( DWORD mess);
	virtual ~VNCHelp();


};

#endif 
