//  Copyright (C) 2005 Dennis Syrovatsky. All Rights Reserved.
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

#if !defined(ECHOCONCTRL_H)
#define ECHOCONCTRL_H

#include "vncServer.h"
#include "echoConnection.h"
#include "echoTypes.h"

class echoConCtrl  
{
public:
	echoConCtrl(vncServer *server);
	virtual ~echoConCtrl();

	void initialize(unsigned int port);
	void destroy();

	unsigned int add(ECHOPROP *echoProp);
	bool change(ECHOPROP *oldEchoProp, ECHOPROP *newEchoProp);


	void deleteAt(int number);

	bool getEntriesAt(int number, ECHOPROP *echoProp);

	int isExists(ECHOPROP *echoProp);

	int getNumEntries() { return m_NumEntries; };

	int getConnectionStatus(ECHOPROP *echoProp);
	void allowEchoConnection(bool status);

private:
	vncServer *m_pServer;
	echoConnection *m_pEchoConnection;

	ECHOPROP * m_pEntries;
	int m_NumEntries;

	void free();
};

#endif // !defined(ECHOCONCTRL_H)
