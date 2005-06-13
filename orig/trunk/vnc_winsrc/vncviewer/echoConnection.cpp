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

#include "stdio.h"

#include "echoConnection.h"

echoConnection::echoConnection()
{
	m_bInitialized = FALSE;
	m_hEchoInst = NULL;
	m_pEchoProxyInfo = NULL;
	resetEchoProcPtrs();
}

echoConnection::~echoConnection()
{
}

BOOL 
echoConnection::initialize(ECHOPROP *echoProp)
{
	if (!loadLibrary()) return FALSE;
	if (!getEchoProcAddr()) {
		freeLibrary();
		return FALSE;
	}
	if (!m_lpfnInitializeProxyDLL()) return FALSE;
	m_lpfnSetLoggingOptions(FALSE, NULL);

	m_pEchoProxyInfo = (echoProxyInfo *)m_lpfnCreateProxyInfoClassObject();

	if (m_pEchoProxyInfo == NULL) {
		destroy();
		return FALSE;
	}

	m_pEchoProxyInfo->SetIP(echoProp->server);
	m_pEchoProxyInfo->SetPort(echoProp->port);
	m_pEchoProxyInfo->SetMyID(echoProp->username);
	m_pEchoProxyInfo->SetPassword(echoProp->pwd);

	m_bInitialized = TRUE;

	return TRUE;
}

void
echoConnection::destroy()
{
	if (m_pEchoProxyInfo != NULL) {
		m_lpfnDeleteProxyInfoClassObject(m_pEchoProxyInfo);
	}
	freeLibrary();
	m_bInitialized = FALSE;
}

BOOL
echoConnection::loadLibrary()
{
	if (m_hEchoInst == NULL) {
		m_hEchoInst = LoadLibrary("echoware.dll");
		if (m_hEchoInst) return TRUE;
	}
	return FALSE;
}

BOOL
echoConnection::freeLibrary()
{
	if (FreeLibrary(m_hEchoInst)) {
		m_hEchoInst = NULL;
		resetEchoProcPtrs();
		return TRUE;
	} else {
		return FALSE;
	}
}

int 
echoConnection::connect()
{
	if (!m_bInitialized) return -1;

	return m_lpfnConnectProxy(m_pEchoProxyInfo);
}

BOOL
echoConnection::disconnect()
{
	if (!m_bInitialized) return FALSE;

	if (m_pEchoProxyInfo != NULL) {
		if (m_pEchoProxyInfo->GetStatus() != 0) {
			return m_lpfnDisconnectProxy(m_pEchoProxyInfo);
		}
	}
	return FALSE;
}

int
echoConnection::establishNewDataChannel(char *IDOfPartner)
{
	if (strlen(IDOfPartner) >= ID_STRING_SIZE) return -1;

	strcpy((char *)m_pszPartnerID, IDOfPartner);

	return m_lpfnEstablishNewDataChannel(m_pEchoProxyInfo, IDOfPartner);	
}

BOOL
echoConnection::getEchoProcAddr()
{
	BOOL bResult = TRUE;

	m_lpfnGetDllVersion = (LPFN_ECHOWARE_GET_DLLVERSION) 
						   GetProcAddress(m_hEchoInst, "GetDllVersion");

	if (!m_lpfnGetDllVersion) bResult = FALSE;

	m_lpfnInitializeProxyDLL = (LPFN_ECHOWARE_INITIALIZE_PROXYDLL) 
								GetProcAddress(m_hEchoInst, "InitializeProxyDll");

	if (!m_lpfnInitializeProxyDLL) bResult = FALSE;

	m_lpfnSetLoggingOptions = (LPFN_ECHOWARE_SET_LOGGING_OPTIONS) 
							   GetProcAddress(m_hEchoInst, "SetLoggingOptions");

	if (!m_lpfnSetLoggingOptions) bResult = FALSE;

	m_lpfnSetPortForOffLoadingData = (LPFN_ECHOWARE_SET_PORT_FOR_OFFLOADING_DATA) 
									  GetProcAddress(m_hEchoInst, "SetPortForOffLoadingData");

	if (!m_lpfnSetPortForOffLoadingData) bResult = FALSE;

	m_lpfnCreateProxyInfoClassObject = (LPFN_ECHOWARE_CREATE_PROXY_INFO_CLASS_OBJECT) 
										GetProcAddress(m_hEchoInst, "CreateProxyInfoClassObject");

	if (!m_lpfnCreateProxyInfoClassObject) bResult = FALSE;

	m_lpfnDeleteProxyInfoClassObject = (LPFN_ECHOWARE_DELETE_PROXY_INFO_CLASS_OBJECT) 
										GetProcAddress(m_hEchoInst, "DeleteProxyInfoClassObject");

	if (!m_lpfnDeleteProxyInfoClassObject) bResult = FALSE;

	m_lpfnAutoConnect = (LPFN_ECHOWARE_AUTO_CONNECT) 
						 GetProcAddress(m_hEchoInst, "AutoConnect");

	if (!m_lpfnAutoConnect) bResult = FALSE;

	m_lpfnConnectProxy = (LPFN_ECHOWARE_CONNECT_PROXY) 
						  GetProcAddress(m_hEchoInst, "ConnectProxy");

	if (!m_lpfnConnectProxy) bResult = FALSE;

	m_lpfnDisconnectProxy = (LPFN_ECHOWARE_DISCONNECT_PROXY) 
							 GetProcAddress(m_hEchoInst, "DisconnectProxy");

	if (!m_lpfnDisconnectProxy) bResult = FALSE;

	m_lpfnDisconnectAllProxies = (LPFN_ECHOWARE_DISCONNECT_ALL_PROXIES) 
								  GetProcAddress(m_hEchoInst, "DisconnectAllProxies");

	if (!m_lpfnDisconnectAllProxies) bResult = FALSE;

	m_lpfnEstablishNewDataChannel = (LPFN_ECHOWARE_ESTABLISH_NEW_DATA_CHANNEL) 
									 GetProcAddress(m_hEchoInst, "EstablishNewDataChannel");
	
	if (!m_lpfnEstablishNewDataChannel) bResult = FALSE;

	if (!bResult) {
		freeLibrary();
		return FALSE;
	}

	return TRUE;
}

void
echoConnection::resetEchoProcPtrs()
{
	m_lpfnGetDllVersion              = NULL;
	m_lpfnInitializeProxyDLL         = NULL;
	m_lpfnSetLoggingOptions          = NULL;
	m_lpfnSetPortForOffLoadingData   = NULL;
	m_lpfnCreateProxyInfoClassObject = NULL;
	m_lpfnDeleteProxyInfoClassObject = NULL;
	m_lpfnAutoConnect                = NULL;
	m_lpfnConnectProxy               = NULL;
	m_lpfnDisconnectProxy            = NULL;
	m_lpfnDisconnectAllProxies       = NULL;
	m_lpfnEstablishNewDataChannel    = NULL;
}