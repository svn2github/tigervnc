
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id$

class PollingScanLines ;

#ifndef __POLLING_SCANLINES_H
#define __POLLING_SCANLINES_H

#include "vncDesktop.h"
#include "PollingBase.h"

//
// class definition
//

class PollingScanLines : public PollingBase
{
public:
	PollingScanLines( vncDesktop* desktop ) ;
	virtual ~PollingScanLines() {} ;

	void PerformPolling( void ) ;
	int GetCycleDivisor( void ) { return 16 ; } ;

private:
	void PollWindow( HWND hwnd ) ;
	void PollArea( RECT &rect ) ;

	static const int m_pollingOrder[32] ;
	int m_pollingStep ;

} ;

#endif // __POLLING_SCANLINES_H
