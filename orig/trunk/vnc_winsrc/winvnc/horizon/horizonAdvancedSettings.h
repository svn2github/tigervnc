
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id$

class horizonAdvancedSettings ;

#ifndef __HORIZON_ADVANCED_SETTINGS_H
#define __HORIZON_ADVANCED_SETTINGS_H

#include "stdhdrs.h"

#include "horizonSettingsDialog.h"
#include "horizonPollControls.h"
#include "horizonProperties.h"

//
// class definition
//

class horizonAdvancedSettings : public horizonSettingsDialog 
{
public:
	horizonAdvancedSettings() : m_pollcontrols( NULL ), horizonSettingsDialog() {} ;
	~horizonAdvancedSettings() {} ;

	bool Show( void ) ;

	static BOOL CALLBACK DialogProc( HWND hwnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam ) ;
	
protected:
	// handle 
	horizonPollControls* m_pollcontrols ;

} ;

#endif // __HORIZON_ADVANCED_SETTINGS_H
