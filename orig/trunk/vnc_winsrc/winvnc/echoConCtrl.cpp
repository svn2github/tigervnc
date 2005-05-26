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

#include "echoConCtrl.h"

echoConCtrl::echoConCtrl(vncServer *server)
{
	m_NumEntries = 0;
	m_pEntries = NULL;

	m_pServer = server;

	m_pEchoConnection = new echoConnection();
}

echoConCtrl::~echoConCtrl()
{
	destroy();	
}

void 
echoConCtrl::initialize(unsigned int port)
{
	if (m_pEchoConnection->initialize(port)) {
		ECHOPROP echoProp;
		for (int i = 0; i < getNumEntries(); i++) {
			getEntriesAt(i, &echoProp);
			m_pEchoConnection->addEchoConnection(&echoProp);
		}
	}
}

void 
echoConCtrl::destroy()
{
	m_pEchoConnection->destroy();
	delete m_pEchoConnection;
	free();
}

unsigned int 
echoConCtrl::add(ECHOPROP *echoProp)
{
	if (isExists(echoProp) != -1) return 1;

	ECHOPROP *pTemporary = new ECHOPROP[m_NumEntries + 1];
	if (m_NumEntries != 0) 
		memcpy(pTemporary, m_pEntries, m_NumEntries * sizeof(ECHOPROP));

	strcpy(pTemporary[m_NumEntries].server, echoProp->server);
	strcpy(pTemporary[m_NumEntries].port, echoProp->port);
	strcpy(pTemporary[m_NumEntries].username, echoProp->username);
	strcpy(pTemporary[m_NumEntries].pwd, echoProp->pwd);
	pTemporary[m_NumEntries].connectionType = echoProp->connectionType;
	
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}

	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries++;

	m_pEchoConnection->addEchoConnection(echoProp); 

	return 0;
}

bool
echoConCtrl::change(ECHOPROP *oldEchoProp, ECHOPROP *newEchoProp)
{
	int posNew = isExists(newEchoProp);
	int posOld = isExists(oldEchoProp);

	if ((posNew >= 0) && (posOld != posNew)) return false;

	if (posOld == -1) return false;

	if (posOld >= 0) {
		strcpy(m_pEntries[posOld].server, newEchoProp->server);
		strcpy(m_pEntries[posOld].port, newEchoProp->port);
		strcpy(m_pEntries[posOld].username, newEchoProp->username);
		strcpy(m_pEntries[posOld].pwd, newEchoProp->pwd);
		m_pEntries[posOld].connectionType = newEchoProp->connectionType;
	}

	return true;
}

void 
echoConCtrl::deleteAt(int number)
{
	if ((number >= m_NumEntries) || (number < 0)) return;
	
	ECHOPROP *pTemporary = new ECHOPROP[m_NumEntries - 1];
	
	if (number == 0) {
		memcpy(pTemporary, &m_pEntries[1], (m_NumEntries - 1) * sizeof(ECHOPROP));
	} else {
		memcpy(pTemporary, m_pEntries, number * sizeof(ECHOPROP));
		if (number != (m_NumEntries - 1)) memcpy(&pTemporary[number], &m_pEntries[number + 1], (m_NumEntries - number - 1) * sizeof(ECHOPROP));
	}
	
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries = m_NumEntries - 1;
}

void 
echoConCtrl::free()
{
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_NumEntries = 0;
}

bool 
echoConCtrl::getEntriesAt(int number, ECHOPROP *echoProp)
{
	if ((number >= m_NumEntries) || (number < 0)) return false;
	
	strcpy(echoProp->server, m_pEntries[number].server);
	strcpy(echoProp->port, m_pEntries[number].port);
	strcpy(echoProp->username, m_pEntries[number].username);
	strcpy(echoProp->pwd, m_pEntries[number].pwd);
	echoProp->connectionType = m_pEntries[number].connectionType;

	return true;
}

int
echoConCtrl::isExists(ECHOPROP *echoProp)
{
	ECHOPROP prop;
	for (int i = 0; i < m_NumEntries; i++) {
		if (getEntriesAt(i, &prop)) {
			if ((strcmp(echoProp->server, prop.server) == 0) &&
				(strcmp(echoProp->port, prop.port) == 0) &&
				(strcmp(echoProp->username, prop.username) == 0)) {
				if (strcmp(echoProp->pwd, prop.pwd) != 0) {
					return -2;
				}
				if (echoProp->connectionType != prop.connectionType) {
					return -3;
				}
				return i;
			}
		}
	}

	return -1;
}

int 
echoConCtrl::getConnectionStatus(ECHOPROP *echoProp)
{
	return m_pEchoConnection->getStatus(echoProp);
}

void 
echoConCtrl::allowEchoConnection(bool status)
{
	m_pServer->enableEchoConnection(status);
	if (!status) {
		m_pEchoConnection->disconnectAll();
	} else {
		ECHOPROP echoProp;
		for (int i = 0; i < getNumEntries(); i++) {
			getEntriesAt(i, &echoProp);
			if (echoProp.connectionType > 0) m_pEchoConnection->connectTo(&echoProp);
		}
	}
}
