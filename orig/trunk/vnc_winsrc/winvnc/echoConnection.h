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

#ifndef ECHO_CONNECT_H
#define ECHO_CONNECT_H

#include "echoTypes.h"
#include "interfacedllproxyinfo.h"

class echoConnection
{
public:
	echoConnection();
	~echoConnection();

	BOOL initialize(unsigned int port);
	void destroy();
	void deleteAllProxyObjects();


	BOOL addEchoConnection(ECHOPROP *echoProps);
	BOOL delEchoConnection(ECHOPROP *echoProps);
	BOOL changeEchoConnection(ECHOPROP *oldEchoProps, ECHOPROP *newEchoProps);

	int connectTo(ECHOPROP *echoProp);
	BOOL disconnect(ECHOPROP *echoProp);
	BOOL disconnectAll();
	
	int getStatus(ECHOPROP *echoProp);

private:

	class echoProxyInfo : public IDllProxyInfo
	{
	public:
		echoProxyInfo();

		void SetName(char* name);
		void SetIpPort(char* ipport);
		void SetIP(char* ipport);
		void SetPort(char* port);
		void SetPassword(char* pass);
		void SetStatus(int Status, BOOL IsStoring);
		void SetSocket(void* p);
		BOOL SetMyID(char* MyID);
		void SetReconnectProxy(BOOL bReconnectProxy);

		char* GetName();
		char* GetIpPort();
		char* GetIP();
		char* GetPort();
		char* GetPassword();
		int GetStatus();
		char* GetMyID();
		BOOL GetReconnectProxy();
    
		BOOL SetSocketTimeout(int connectTimeout, int ReceiveTimeout, int SendTimeout);
	};

	HMODULE m_hEchoInst;

	void resetEchoObjInfo();
	void resetEchoProcPtrs();

	LPFN_ECHOWARE_GET_DLLVERSION                 m_lpfnGetDllVersion;
	LPFN_ECHOWARE_INITIALIZE_PROXYDLL            m_lpfnInitializeProxyDLL;
	LPFN_ECHOWARE_SET_LOGGING_OPTIONS            m_lpfnSetLoggingOptions;
	LPFN_ECHOWARE_SET_PORT_FOR_OFFLOADING_DATA   m_lpfnSetPortForOffLoadingData;
	LPFN_ECHOWARE_CREATE_PROXY_INFO_CLASS_OBJECT m_lpfnCreateProxyInfoClassObject;
	LPFN_ECHOWARE_DELETE_PROXY_INFO_CLASS_OBJECT m_lpfnDeleteProxyInfoClassObject;
	LPFN_ECHOWARE_AUTO_CONNECT                   m_lpfnAutoConnect;
	LPFN_ECHOWARE_CONNECT_PROXY                  m_lpfnConnectProxy;
	LPFN_ECHOWARE_DISCONNECT_PROXY               m_lpfnDisconnectProxy;
	LPFN_ECHOWARE_DISCONNECT_ALL_PROXIES         m_lpfnDisconnectAllProxies;
	LPFN_ECHOWARE_ESTABLISH_NEW_DATA_CHANNEL     m_lpfnEstablishNewDataChannel;

	BOOL loadLibrary();
	BOOL freeLibrary();

	BOOL getEchoProcAddr();

	BOOL initEchoConnection(int port);
	echoProxyInfo *getEchoObject(ECHOPROP *echoProp);
	int connectTo(echoProxyInfo *pProxyInfo);

	unsigned int m_callbackPort;
//	unsigned int m_echoConParamNumber;

	echoProxyInfo *m_pEchoProxyInfo[MAX_ECHO_SERVERS];

	BOOL m_bInitialized;
	
};

#endif // ECHO_CONNECT_H
