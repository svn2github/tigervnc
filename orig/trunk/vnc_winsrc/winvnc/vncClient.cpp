//  Copyright (C) 2001-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2002 Vladimir Vologzhanin. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


// vncClient.cpp

// The per-client object.  This object takes care of all per-client stuff,
// such as socket input and buffering of updates.

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the vncDesktop and
// vncServer to communicate with.

// Includes
#include "stdhdrs.h"
#include <omnithread.h>
#include "resource.h"

// Custom
#include "vncClient.h"
#include "VSocket.h"
#include "vncDesktop.h"
#include "vncRegion.h"
#include "vncBuffer.h"
#include "vncService.h"
#include "vncPasswd.h"
#include "vncAcceptDialog.h"
#include "vncKeymap.h"

//
// Normally, using macros is no good, but this macro saves us from
// writing constants twice -- it constructs signature names from codes.
// Note that "code_sym" argument should be a single symbol, not an expression.
//

#define SetCapInfo(cap_ptr, code_sym, vendor)			\
{														\
	rfbCapabilityInfo *pcap = (cap_ptr);				\
	pcap->code = Swap32IfLE(code_sym);					\
	memcpy(pcap->vendorSignature, (vendor),				\
		   sz_rfbCapabilityInfoVendor);					\
	memcpy(pcap->nameSignature, sig_##code_sym,			\
		   sz_rfbCapabilityInfoName);					\
}

// vncClient thread class

class vncClientThread : public omni_thread
{
public:
	char * ConvertPath(char *path);
	HANDLE m_hFiletoWrite;
	char m_FullFilename[255 + 1];
	
	// Init
	virtual BOOL Init(vncClient *client,
		vncServer *server,
		VSocket *socket,
		BOOL auth,
		BOOL shared);

	// Sub-Init routines
	virtual BOOL InitVersion();
	virtual BOOL InitTunneling();
	virtual BOOL InitAuthenticate();
	virtual void SendConnFailedMessage(const char *msg);
	virtual BOOL SendNoAuthMessage();
	virtual BOOL SendAuthenticationCaps();
	virtual BOOL SendInteractionCaps();

	// The main thread function
	virtual void run(void *arg);

protected:
	virtual ~vncClientThread();

	// Fields
protected:
	VSocket *m_socket;
	vncServer *m_server;
	vncClient *m_client;
	BOOL m_auth;
	BOOL m_shared;
};

vncClientThread::~vncClientThread()
{
	// If we have a client object then delete it
	if (m_client != NULL)
		delete m_client;
}

BOOL
vncClientThread::Init(vncClient *client, vncServer *server, VSocket *socket, BOOL auth, BOOL shared)
{
	// Save the server pointer and window handle
	m_server = server;
	m_socket = socket;
	m_client = client;
	m_auth = auth;
	m_shared = shared;

	// Start the thread
	start();

	return TRUE;
}

BOOL
vncClientThread::InitVersion()
{
	// Generate the server's protocol version
	rfbProtocolVersionMsg protocolMsg;
	sprintf((char *)protocolMsg,
		rfbProtocolVersionFormat,
		rfbProtocolMajorVersion,
		rfbProtocolMinorVersion);

	// Send the protocol message
	if (!m_socket->SendExact((char *)&protocolMsg, sz_rfbProtocolVersionMsg))
		return FALSE;

	// Now, get the client's protocol version
	rfbProtocolVersionMsg protocol_ver;
	protocol_ver[12] = 0;
	if (!m_socket->ReadExact((char *)&protocol_ver, sz_rfbProtocolVersionMsg))
		return FALSE;

	// Check the protocol version
	int major, minor;
	sscanf((char *)&protocol_ver, rfbProtocolVersionFormat, &major, &minor);
	if (major != rfbProtocolMajorVersion) {
		vnclog.Print(LL_CONNERR, VNCLOG("protocol versions do not match\n"));
		return FALSE;
	}

	// Save the minor number of the protocol version
	m_client->m_protocol_minor_version = minor;

	return TRUE;
}

//
// Setup tunneling (protocol version 3.130).
//

BOOL
vncClientThread::InitTunneling()
{
	int nTypes = 0;

	// Advertise our tunneling capabilities (currently, nothing to advertise).
	rfbTunnelingCapsMsg caps;
	caps.connFailed = (CARD8)false;
	caps.nTunnelTypes = Swap16IfLE(nTypes);
	return m_socket->SendExact((char *)&caps, sz_rfbTunnelingCapsMsg);

	// Read tunneling type requested by the client (currently, not necessary).
	if (nTypes) {
		CARD32 tunnelType;
		if (!m_socket->ReadExact((char *)&tunnelType, sizeof(tunnelType)))
			return FALSE;
		tunnelType = Swap32IfLE(tunnelType);
		// We cannot do tunneling yet.
		if (tunnelType != 0)
			return FALSE;
	}

	return TRUE;
}

BOOL
vncClientThread::InitAuthenticate()
{
	// Retrieve local passwords
	char password[MAXPWLEN];
	m_server->GetPassword(password);
	vncPasswd::ToText plain(password);
	m_server->GetPasswordViewOnly(password);
	vncPasswd::ToText plain_viewonly(password);

#ifndef HORIZONLIVE
	// By default we disallow passwordless workstations!
	if ((strlen(plain) == 0) && m_server->AuthRequired())
	{
		vnclog.Print(LL_CONNERR,
			VNCLOG("no password specified for server - client rejected\n"));

		// Send an error message to the client
		SendConnFailedMessage("This server does not have a valid password enabled.  "
							  "Until a password is set, incoming connections cannot "
							  "be accepted.");
		return FALSE;
	}
#endif

	// By default we filter out local loop connections, because they're pointless
#ifndef HORIZONLIVE
	if (!m_server->LoopbackOk())
	{
		char *localname = strdup(m_socket->GetSockName());
		char *remotename = strdup(m_socket->GetPeerName());

		// Check that the local & remote names are different!
		if ((localname != NULL) && (remotename != NULL))
		{
			BOOL ok = strcmp(localname, remotename) != 0;

			free(localname);
			free(remotename);

			if (!ok) {
				vnclog.Print(LL_CONNERR, VNCLOG("loopback connection attempted - client rejected\n"));
				
				// Send an error message to the client
				SendConnFailedMessage("Local loop-back connections are disabled.");
				return FALSE;
			}
		}
	}
#endif

	// Verify the peer host name against the AuthHosts string
	vncServer::AcceptQueryReject verified;
	if (m_auth) {
		verified = vncServer::aqrAccept;
	} else {
		verified = m_server->VerifyHost(m_socket->GetPeerName());
	}

	// If necessary, query the connection with a timed dialog
	BOOL skip_auth = FALSE;
	if (verified == vncServer::aqrQuery) {
		vncAcceptDialog *acceptDlg =
			new vncAcceptDialog(m_server->QueryTimeout(),
								m_server->QueryAccept(),
								m_server->QueryAllowNoPass(),
								m_socket->GetPeerName());
		if (acceptDlg == NULL) {
			if (m_server->QueryAccept()) {
				verified = vncServer::aqrAccept;
			} else {
				verified = vncServer::aqrReject;
			}
		} else {
			int action = acceptDlg->DoDialog();
			if (action > 0) {
				verified = vncServer::aqrAccept;
				if (action == 2)
					skip_auth = TRUE;	// accept without authentication
			} else {
				verified = vncServer::aqrReject;
			}
			delete acceptDlg;
		}
	}

	// The connection should be rejected, either due to AuthHosts settings,
	// or because of the "Reject" action performed in the query dialog
	if (verified == vncServer::aqrReject) {
		vnclog.Print(LL_CONNERR, VNCLOG("Client connection rejected\n"));
		SendConnFailedMessage("Your connection has been rejected.");
		return FALSE;
	}

	// Authenticate the connection, if required
	if (m_auth || strlen(plain) == 0 || skip_auth)
	{
		// Send no-auth-required message
		if (!SendNoAuthMessage())
			return FALSE;
	}
	else
	{
		// Send auth-required message.
		if (m_client->m_protocol_minor_version >= 130) {

			// Advertise our authentication capabilities (protocol version 3.130).
			if (!SendAuthenticationCaps())
				return FALSE;
			vnclog.Print(LL_INTINFO, VNCLOG("sent authentication capability list\n"));

			// Read the actual authentication type from the client.
			CARD32 auth_type;
			if (!m_socket->ReadExact((char *)&auth_type, sizeof(auth_type)))
				return FALSE;
			auth_type = Swap32IfLE(auth_type);
			if (auth_type != rfbVncAuth) {
				vnclog.Print(LL_CONNERR, VNCLOG("unknown authentication scheme requested\n"));
				return FALSE;
			}

		} else {

			// In the protocol version 3.3, just send 32-bit value of rfbVncAuth.
			CARD32 auth_val = Swap32IfLE(rfbVncAuth);
			if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
				return FALSE;

		}

		BOOL auth_ok = FALSE;
		{
			// Now create a 16-byte challenge
			char challenge[16];
			char challenge_viewonly[16];

			vncRandomBytes((BYTE *)&challenge);
			memcpy(challenge_viewonly, challenge, 16);

			// Send the challenge to the client
			if (!m_socket->SendExact(challenge, sizeof(challenge)))
				return FALSE;

			// Read the response
			char response[16];
			if (!m_socket->ReadExact(response, sizeof(response)))
				return FALSE;

			// Encrypt the challenge bytes
			vncEncryptBytes((BYTE *)&challenge, plain);

			// Compare them to the response
			if (memcmp(challenge, response, sizeof(response)) == 0) {
				auth_ok = TRUE;
			} else {
				// Check against the view-only password
				vncEncryptBytes((BYTE *)&challenge_viewonly, plain_viewonly);
				if (memcmp(challenge_viewonly, response, sizeof(response)) == 0) {
					m_client->m_pointerenabled = FALSE;
					m_client->m_keyboardenabled = FALSE;
					auth_ok = TRUE;
				}
			}
		}

		// Did the authentication work?
		CARD32 authmsg;
		if (!auth_ok)
		{
			vnclog.Print(LL_CONNERR, VNCLOG("authentication failed\n"));

			authmsg = Swap32IfLE(rfbVncAuthFailed);
			m_socket->SendExact((char *)&authmsg, sizeof(authmsg));
			return FALSE;
		}
		else
		{
			// Tell the client we're ok
			authmsg = Swap32IfLE(rfbVncAuthOK);
			if (!m_socket->SendExact((char *)&authmsg, sizeof(authmsg)))
				return FALSE;
		}
	}

	// Read the client's initialisation message
	rfbClientInitMsg client_ini;
	if (!m_socket->ReadExact((char *)&client_ini, sz_rfbClientInitMsg))
		return FALSE;

	// If the client wishes to have exclusive access then remove other clients
	if (!client_ini.shared && !m_shared)
	{
		// Which client takes priority, existing or incoming?
		if (m_server->ConnectPriority() < 1)
		{
			// Incoming
			vnclog.Print(LL_INTINFO, VNCLOG("non-shared connection - disconnecting old clients\n"));
			m_server->KillAuthClients();
		} else if (m_server->ConnectPriority() > 1)
		{
			// Existing
			if (m_server->AuthClientCount() > 0)
			{
				vnclog.Print(LL_CLIENTS, VNCLOG("connections already exist - client rejected\n"));
				return FALSE;
			}
		}
	}

	// Tell the server that this client is ok
	return m_server->Authenticated(m_client->GetClientId());
}

//
// Send a "connection failed" message. In the protocol version 3.130, this
// will be sent as a rfbTunnelingCapsMsg or a rfbAuthenticationCapsMsg
// structure (rfbConnFailedMsg structure is a sort of union of those).
// In the protocol 3.3, an "authentication scheme" with the value of
// rfbConnFailed will be sent.
//

void
vncClientThread::SendConnFailedMessage(const char *msg)
{
	if (m_client->m_protocol_minor_version >= 130) {
		rfbConnFailedMsg failMsg;
		failMsg.connFailed = (CARD8)true;
		failMsg.reasonLength = Swap32IfLE(strlen(msg));
		if (!m_socket->SendExact((char *)&failMsg, sz_rfbConnFailedMsg)) {
			return;
		}
	} else {
		CARD32 authValue = Swap32IfLE(rfbConnFailed);
		CARD32 errlen = Swap32IfLE(strlen(msg));
		if (!m_socket->SendExact((char *)&authValue, sizeof(authValue)) ||
			!m_socket->SendExact((char *)&errlen, sizeof(errlen))) {
			return;
		}
	}
	m_socket->SendExact(msg, strlen(msg));
}

//
// Send "no authentication required" message. In the protocol version 3.130,
// this will be sent as a rfbAuthenticationCapsMsg structure. In the protocol
// 3.3, an "authentication scheme" with the value of rfbNoAuth will be sent.
//

BOOL
vncClientThread::SendNoAuthMessage()
{
	if (m_client->m_protocol_minor_version >= 130) {
		rfbAuthenticationCapsMsg caps;
		caps.connFailed = (CARD8)false;
		caps.nAuthTypes = Swap16IfLE(0);
		if (!m_socket->SendExact((char *)&caps, sz_rfbAuthenticationCapsMsg)) {
			return FALSE;
		}
	} else {
		CARD32 authValue = Swap32IfLE(rfbNoAuth);
		if (!m_socket->SendExact((char *)&authValue, sizeof(authValue))) {
			return FALSE;
		}
	}
	return TRUE;
}

//
// Advertise our authentication capabilities (protocol version 3.130).
//

BOOL
vncClientThread::SendAuthenticationCaps()
{
	rfbAuthenticationCapsMsg caps;
	caps.connFailed = (CARD8)false;
	caps.nAuthTypes = Swap16IfLE(1);
	if (!m_socket->SendExact((char *)&caps, sz_rfbAuthenticationCapsMsg))
		return FALSE;

	// Inform the client that we support only the standard VNC authentication.
	rfbCapabilityInfo cap;
	SetCapInfo(&cap, rfbVncAuth, rfbStandardVendor);
	return m_socket->SendExact((char *)&cap, sz_rfbCapabilityInfo);
}

//
// Advertise our messaging capabilities (protocol version 3.130).
//

BOOL
vncClientThread::SendInteractionCaps()
{
	// Update these constants on changing capability lists!
	const int MAX_SMSG_CAPS = 4;
	const int MAX_CMSG_CAPS = 6;
	const int MAX_ENC_CAPS = 14;

	int i;

	// Supported server->client message types
	rfbCapabilityInfo smsg_list[MAX_SMSG_CAPS];
	i = 0;
#ifndef HORIZONLIVE
	if (m_server->FileTransfersEnabled() &&
		(m_client->m_keyboardenabled || m_client->m_pointerenabled)) {
		SetCapInfo(&smsg_list[i++], rfbFileListData,       rfbTightVncVendor);
		SetCapInfo(&smsg_list[i++], rfbFileDownloadData,   rfbTightVncVendor);
		SetCapInfo(&smsg_list[i++], rfbFileUploadCancel,   rfbTightVncVendor);
		SetCapInfo(&smsg_list[i++], rfbFileDownloadFailed, rfbTightVncVendor);
	}
#endif
	int nServerMsgs = i;
	if (nServerMsgs > MAX_SMSG_CAPS) {
		vnclog.Print(LL_INTERR,
					 VNCLOG("assertion failed, nServerMsgs > MAX_SMSG_CAPS\n"));
		return FALSE;
	}

	// Supported client->server message types
	rfbCapabilityInfo cmsg_list[MAX_CMSG_CAPS];
	i = 0;
#ifndef HORIZONLIVE
	if (m_server->FileTransfersEnabled() &&
		(m_client->m_keyboardenabled || m_client->m_pointerenabled)) {
		SetCapInfo(&cmsg_list[i++], rfbFileListRequest,    rfbTightVncVendor);
		SetCapInfo(&cmsg_list[i++], rfbFileDownloadRequest,rfbTightVncVendor);
		SetCapInfo(&cmsg_list[i++], rfbFileUploadRequest,  rfbTightVncVendor);
		SetCapInfo(&cmsg_list[i++], rfbFileUploadData,     rfbTightVncVendor);
		SetCapInfo(&cmsg_list[i++], rfbFileDownloadCancel, rfbTightVncVendor);
		SetCapInfo(&cmsg_list[i++], rfbFileUploadFailed,   rfbTightVncVendor);
	}
#endif
	int nClientMsgs = i;
	if (nClientMsgs > MAX_CMSG_CAPS) {
		vnclog.Print(LL_INTERR,
					 VNCLOG("assertion failed, nClientMsgs > MAX_CMSG_CAPS\n"));
		return FALSE;
	}

	// Encoding types
	rfbCapabilityInfo enc_list[MAX_ENC_CAPS];
	i = 0;
	SetCapInfo(&enc_list[i++],  rfbEncodingCopyRect,       rfbStandardVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingRRE,            rfbStandardVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingCoRRE,          rfbStandardVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingHextile,        rfbStandardVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingZlib,           rfbTridiaVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingZlibHex,        rfbTridiaVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingTight,          rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingCompressLevel0, rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingQualityLevel0,  rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingXCursor,        rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingRichCursor,     rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingPointerPos,     rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingLastRect,       rfbTightVncVendor);
	SetCapInfo(&enc_list[i++],  rfbEncodingNewFBSize,      rfbTightVncVendor);
	int nEncodings = i;
	if (nEncodings > MAX_ENC_CAPS) {
		vnclog.Print(LL_INTERR,
					 VNCLOG("assertion failed, nEncodings > MAX_ENC_CAPS\n"));
		return FALSE;
	}

	// Create and send the header structure
	rfbInteractionCapsMsg intr_caps;
	intr_caps.nServerMessageTypes = Swap16IfLE(nServerMsgs);
	intr_caps.nClientMessageTypes = Swap16IfLE(nClientMsgs);
	intr_caps.nEncodingTypes = Swap16IfLE(nEncodings);
	intr_caps.pad = 0;
	if (!m_socket->SendExact((char *)&intr_caps, sz_rfbInteractionCapsMsg))
		return FALSE;

	// Send the capability lists
	if (nServerMsgs &&
		!m_socket->SendExact((char *)&smsg_list[0],
							 sz_rfbCapabilityInfo * nServerMsgs))
		return FALSE;
	if (nClientMsgs &&
		!m_socket->SendExact((char *)&cmsg_list[0],
							 sz_rfbCapabilityInfo * nClientMsgs))
		return FALSE;
	if (nEncodings &&
		!m_socket->SendExact((char *)&enc_list[0],
							 sz_rfbCapabilityInfo * nEncodings))
		return FALSE;

	return TRUE;
}

void
ClearKeyState(BYTE key)
{
	// This routine is used by the VNC client handler to clear the
	// CAPSLOCK, NUMLOCK and SCROLL-LOCK states.

	BYTE keyState[256];
	
	GetKeyboardState((LPBYTE)&keyState);

	if(keyState[key] & 1)
	{
		// Simulate the key being pressed
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY, 0);

		// Simulate it being release
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

void
vncClientThread::run(void *arg)
{
	// All this thread does is go into a socket-recieve loop,
	// waiting for stuff on the given socket

	// IMPORTANT : ALWAYS call RemoveClient on the server before quitting
	// this thread.

	vnclog.Print(LL_CLIENTS, VNCLOG("client connected : %s (id %hd)\n"),
				 m_client->GetClientName(), m_client->GetClientId());

	// Save the handle to the thread's original desktop
	HDESK home_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	// To avoid people connecting and then halting the connection, set a timeout
	if (!m_socket->SetTimeout(30000))
		vnclog.Print(LL_INTERR, VNCLOG("failed to set socket timeout, error=%d\n"), GetLastError());

	// Initially blacklist the client so that excess connections from it get dropped
	m_server->AddAuthHostsBlacklist(m_client->GetClientName());

	// LOCK INITIAL SETUP
	// All clients have the m_protocol_ready flag set to FALSE initially, to prevent
	// updates and suchlike interfering with the initial protocol negotiations.

	// GET PROTOCOL VERSION
	if (!InitVersion())
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("negotiated protocol version\n"));

	// INITIALISE TUNNELING (protocol 3.130)
	if (m_client->m_protocol_minor_version >= 130) {
		if (!InitTunneling()) {
			m_server->RemoveClient(m_client->GetClientId());
			return;
		}
		vnclog.Print(LL_INTINFO, VNCLOG("negotiated tunneling type\n"));
	}

	// AUTHENTICATE LINK
	if (!InitAuthenticate()) {
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}

	// Authenticated OK - remove from blacklist and remove timeout
	m_server->RemAuthHostsBlacklist(m_client->GetClientName());
	m_socket->SetTimeout(m_server->AutoIdleDisconnectTimeout()*1000);
	vnclog.Print(LL_INTINFO, VNCLOG("authenticated connection\n"));

	// INIT PIXEL FORMAT

	// Get the screen format
	m_client->m_fullscreen = m_client->m_buffer->GetSize();


	// Get the name of this desktop
	char desktopname[MAX_COMPUTERNAME_LENGTH+1];
	DWORD desktopnamelen = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerName(desktopname, &desktopnamelen))
	{
		// Make the name lowercase
		for (size_t x=0; x<strlen(desktopname); x++)
		{
			desktopname[x] = tolower(desktopname[x]);
		}
	}
	else
	{
		strcpy(desktopname, "WinVNC");
	}

	// Send the server format message to the client
	rfbServerInitMsg server_ini;
	server_ini.format = m_client->m_buffer->GetLocalFormat();

	// Endian swaps
	RECT SharedRect;
	SharedRect = m_server->getSharedRect();
	server_ini.framebufferWidth = Swap16IfLE(SharedRect.right- SharedRect.left);
	server_ini.framebufferHeight = Swap16IfLE(SharedRect.bottom - SharedRect.top);
	server_ini.format.redMax = Swap16IfLE(server_ini.format.redMax);
	server_ini.format.greenMax = Swap16IfLE(server_ini.format.greenMax);
	server_ini.format.blueMax = Swap16IfLE(server_ini.format.blueMax);
	server_ini.nameLength = Swap32IfLE(strlen(desktopname));

	if (!m_socket->SendExact((char *)&server_ini, sizeof(server_ini)))
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	if (!m_socket->SendExact(desktopname, strlen(desktopname)))
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("sent pixel format to client\n"));

	// Inform the client about our interaction capabilities (protocol 3.130)
	if (m_client->m_protocol_minor_version >= 130) {
		if (!SendInteractionCaps()) {
			m_server->RemoveClient(m_client->GetClientId());
			return;
		}
		vnclog.Print(LL_INTINFO, VNCLOG("list of interaction capabilities sent\n"));
	}

	// UNLOCK INITIAL SETUP
	// Initial negotiation is complete, so set the protocol ready flag
	{
		omni_mutex_lock l(m_client->m_regionLock);
		m_client->m_protocol_ready = TRUE;
	}

	// Clear the CapsLock and NumLock keys
	if (m_client->m_keyboardenabled)
	{
		ClearKeyState(VK_CAPITAL);
		// *** JNW - removed because people complain it's wrong
		//ClearKeyState(VK_NUMLOCK);
		ClearKeyState(VK_SCROLL);
	}
	
	// MAIN LOOP

	BOOL connected = TRUE;
	while (connected)
	{
		rfbClientToServerMsg msg;

		// Ensure that we're running in the correct desktop
		if (!vncService::InputDesktopSelected())
			if (!vncService::SelectDesktop(NULL))
				break;

		// Try to read a message ID
		if (!m_socket->ReadExact((char *)&msg.type, sizeof(msg.type)))
		{
			connected = FALSE;
			break;
		}

		// What to do is determined by the message id
		switch(msg.type)
		{

		case rfbSetPixelFormat:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbSetPixelFormatMsg-1))
			{
				connected = FALSE;
				break;
			}

			// Swap the relevant bits.
			msg.spf.format.redMax = Swap16IfLE(msg.spf.format.redMax);
			msg.spf.format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
			msg.spf.format.blueMax = Swap16IfLE(msg.spf.format.blueMax);

			{	omni_mutex_lock l(m_client->m_regionLock);
			
				// Tell the buffer object of the change
				if (!m_client->m_buffer->SetClientFormat(msg.spf.format))
				{
					vnclog.Print(LL_CONNERR, VNCLOG("remote pixel format invalid\n"));

					connected = FALSE;
				}

				// Set the palette-changed flag, just in case...
				m_client->m_palettechanged = TRUE;
			}
			
			break;

		case rfbSetEncodings:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbSetEncodingsMsg-1))
			{
				connected = FALSE;
				break;
			}

			m_client->m_buffer->SetQualityLevel(-1);
			m_client->m_buffer->SetCompressLevel(6);
			m_client->m_buffer->EnableXCursor(FALSE);
			m_client->m_buffer->EnableRichCursor(FALSE);
			m_client->m_buffer->EnableLastRect(FALSE);
			m_client->m_use_PointerPos = FALSE;
			m_client->m_use_NewFBSize = FALSE;

			m_client->m_cursor_update_pending = FALSE;
			m_client->m_cursor_update_sent = FALSE;
			m_client->m_cursor_pos_changed = FALSE;

			// Read in the preferred encodings
			msg.se.nEncodings = Swap16IfLE(msg.se.nEncodings);
			{
				int x;
				BOOL encoding_set = FALSE;
				BOOL shapeupdates_requested = FALSE;
				BOOL pointerpos_requested = FALSE;

				{	omni_mutex_lock l(m_client->m_regionLock);
					// By default, don't use copyrect!
					m_client->m_copyrect_use = FALSE;
				}

				for (x=0; x<msg.se.nEncodings; x++)
				{ omni_mutex_lock l(m_client->m_regionLock);
					CARD32 encoding;

					// Read an encoding in
					if (!m_socket->ReadExact((char *)&encoding, sizeof(encoding)))
					{
						connected = FALSE;
						break;
					}

					// Is this the CopyRect encoding (a special case)?
					if (Swap32IfLE(encoding) == rfbEncodingCopyRect)
					{
						// Client wants us to use CopyRect
						m_client->m_copyrect_use = TRUE;
						continue;
					}

					// Is this an XCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingXCursor) {
						m_client->m_buffer->EnableXCursor(TRUE);
						shapeupdates_requested = TRUE;
						vnclog.Print(LL_INTINFO, VNCLOG("X-style cursor shape updates enabled\n"));
						continue;
					}

					// Is this a RichCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingRichCursor) {
						m_client->m_buffer->EnableRichCursor(TRUE);
						shapeupdates_requested = TRUE;
						vnclog.Print(LL_INTINFO, VNCLOG("Full-color cursor shape updates enabled\n"));
						continue;
					}

					// Is this a CompressLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingCompressLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingCompressLevel9))
					{
						// Client specified encoding-specific compression level
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingCompressLevel0);
						m_client->m_buffer->SetCompressLevel(level);
						vnclog.Print(LL_INTINFO, VNCLOG("compression level requested: %d\n"), level);
						continue;
					}

					// Is this a QualityLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingQualityLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingQualityLevel9))
					{
						// Client specified image quality level used for JPEG compression
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingQualityLevel0);
						m_client->m_buffer->SetQualityLevel(level);
						vnclog.Print(LL_INTINFO, VNCLOG("image quality level requested: %d\n"), level);
						continue;
					}

					// Is this a PointerPos encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingPointerPos) {
						pointerpos_requested = TRUE;
						continue;
					}

					// Is this a LastRect encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingLastRect) {
						m_client->m_buffer->EnableLastRect(TRUE);
						vnclog.Print(LL_INTINFO, VNCLOG("LastRect protocol extension enabled\n"));
						continue;
					}

					// Is this a NewFBSize encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingNewFBSize) {
						m_client->m_use_NewFBSize = TRUE;
						vnclog.Print(LL_INTINFO, VNCLOG("NewFBSize protocol extension enabled\n"));
						continue;
					}

					// Have we already found a suitable encoding?
					if (!encoding_set)
					{	// omni_mutex_lock l(m_client->m_regionLock);

						// No, so try the buffer to see if this encoding will work...
						if (m_client->m_buffer->SetEncoding(Swap32IfLE(encoding))) {
 							encoding_set = TRUE;
						}

					}
				}

				// Enable CursorPos encoding only if cursor shape updates were
				// requested by the client.
				if (shapeupdates_requested && pointerpos_requested) {
					m_client->m_use_PointerPos = TRUE;
					m_client->m_cursor_pos_changed = TRUE;
					vnclog.Print(LL_INTINFO, VNCLOG("PointerPos protocol extension enabled\n"));
				}

				// If no encoding worked then default to RAW!
				// FIXME: Protocol extensions won't work in this case.
				if (!encoding_set)
				{
					omni_mutex_lock l(m_client->m_regionLock);

					vnclog.Print(LL_INTINFO, VNCLOG("defaulting to raw encoder\n"));

					if (!m_client->m_buffer->SetEncoding(Swap32IfLE(rfbEncodingRaw)))
					{
						vnclog.Print(LL_INTERR, VNCLOG("failed to select raw encoder!\n"));

						connected = FALSE;
					}
				}
			}

			break;
			
		case rfbFramebufferUpdateRequest:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbFramebufferUpdateRequestMsg-1))
			{
				connected = FALSE;
				break;
			}

			{
				RECT update, SharedRect;

				{	omni_mutex_lock l(m_client->m_regionLock);
				
				SharedRect = m_server->getSharedRect();
				// Get the specified rectangle as the region to send updates for.
				update.left = Swap16IfLE(msg.fur.x)+ SharedRect.left;
				update.top = Swap16IfLE(msg.fur.y)+ SharedRect.top;
				update.right = update.left + Swap16IfLE(msg.fur.w);
				
				if (update.right > m_client->m_fullscreen.right )
					update.right = m_client->m_fullscreen.right;

				update.bottom = update.top + Swap16IfLE(msg.fur.h);
				if ( update.bottom > m_client->m_fullscreen.bottom )
					update.bottom = m_client->m_fullscreen.bottom;
				

					// Set the update-wanted flag to true
					m_client->m_updatewanted = TRUE;

					// Clip the rectangle to the screen
					if (IntersectRect(&update, &update, &SharedRect))
					{
						// Is this request for an incremental region?
						if (msg.fur.incremental)
						{
							// Yes, so add it to the incremental region
							m_client->m_incr_rgn.AddRect(update);
						}
						else
						{
							// No, so add it to the full update region
							m_client->m_full_rgn.AddRect(update);

							// Disable any pending CopyRect
							m_client->m_copyrect_set = FALSE;
						}
					}

					// Trigger an update
					m_server->RequestUpdate();
				}
			}
			break;

		case rfbKeyEvent:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbKeyEventMsg-1))
			{				
				if (m_client->m_keyboardenabled)
				{
					msg.ke.key = Swap32IfLE(msg.ke.key);

					// Get the keymapper to do the work
					vncKeymap::keyEvent(msg.ke.key, msg.ke.down != 0,
										m_client->m_server);
					m_client->m_remoteevent = TRUE;
				}
			}
			break;

		case rfbPointerEvent:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbPointerEventMsg-1))
			{
				if (m_client->m_pointerenabled)
				{
					// Convert the coords to Big Endian
					msg.pe.x = Swap16IfLE(msg.pe.x);
					msg.pe.y = Swap16IfLE(msg.pe.y);

					
					// Remember cursor position for this client
					m_client->m_cursor_pos.x = msg.pe.x;
					m_client->m_cursor_pos.y = msg.pe.y;
					
					// if we share only one window...
			     	
					RECT coord;
					{	omni_mutex_lock l(m_client->m_regionLock);

					coord = m_server->getSharedRect();
					}
					
					
					// to put position relative to screen
					msg.pe.x = msg.pe.x + coord.left;
					msg.pe.y = msg.pe.y + coord.top;

					// Work out the flags for this event
					DWORD flags = MOUSEEVENTF_ABSOLUTE;
					flags |= MOUSEEVENTF_MOVE;
					m_server->SetMouseCounter(1, m_client->m_cursor_pos, false );

					if ( (msg.pe.buttonMask & rfbButton1Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton1Mask) )
					{
					    if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
						    ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
					    else
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
						    ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
							m_server->SetMouseCounter(1, m_client->m_cursor_pos, false);
					}
					if ( (msg.pe.buttonMask & rfbButton2Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton2Mask) )
					{
						flags |= (msg.pe.buttonMask & rfbButton2Mask) 
						    ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
							m_server->SetMouseCounter(1, m_client->m_cursor_pos, false);
					}
					if ( (msg.pe.buttonMask & rfbButton3Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton3Mask) )
					{
					    if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
						    ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
					    else
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
						    ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
							m_server->SetMouseCounter(1, m_client->m_cursor_pos, false);
					}

					// Treat buttons 4 and 5 presses as mouse wheel events
					DWORD wheel_movement = 0;
					if ((msg.pe.buttonMask & rfbButton4Mask) != 0 &&
						(m_client->m_ptrevent.buttonMask & rfbButton4Mask) == 0)
					{
						flags |= MOUSEEVENTF_WHEEL;
						wheel_movement = (DWORD)+120;
					}
					else if ((msg.pe.buttonMask & rfbButton5Mask) != 0 &&
							 (m_client->m_ptrevent.buttonMask & rfbButton5Mask) == 0)
					{
						flags |= MOUSEEVENTF_WHEEL;
						wheel_movement = (DWORD)-120;
					}

					// Generate coordinate values
					
					HWND temp = GetDesktopWindow();
					GetWindowRect(temp,&coord);

					unsigned long x = (msg.pe.x * 65535) / (coord.right - coord.left);
					unsigned long y = (msg.pe.y * 65535) / (coord.bottom - coord.top);

					// Do the pointer event
					::mouse_event(flags, (DWORD)x, (DWORD)y, wheel_movement, 0);
					// Save the old position
					m_client->m_ptrevent = msg.pe;

					// Flag that a remote event occurred
					m_client->m_remoteevent = TRUE;

					// Flag that the mouse moved
					// FIXME: Is it necessary?
					m_client->UpdateMouse();
					
					// Trigger an update
					m_server->RequestUpdate();
				}
			}
			break;

		case rfbClientCutText:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbClientCutTextMsg-1))
			{
				// Allocate storage for the text
				const UINT length = Swap32IfLE(msg.cct.length);
				char *text = new char [length+1];
				if (text == NULL)
					break;
				text[length] = 0;

				// Read in the text
				if (!m_socket->ReadExact(text, length)) {
					delete [] text;
					break;
				}

				// Get the server to update the local clipboard
				m_server->UpdateLocalClipText(text);

				// Free the clip text we read
				delete [] text;
			}
			break;

#ifndef HORIZONLIVE
		case rfbFileListRequest:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbFileListRequestMsg-1))
			{
				const UINT size = msg.flr.dnamesize;
				char path[255 + 1];
				char drive[4];
				char * pAllMessage = new char[sz_rfbFileListDataMsg + 255];
				rfbFileListDataMsg *pFLD = (rfbFileListDataMsg *) pAllMessage;
				char *pFilename = &pAllMessage[sz_rfbFileListDataMsg];
				pFLD->type = rfbFileListData;
				pFLD->fnamesize = 0;
				pFLD->amount = Swap16IfLE(0);
				pFLD->num = Swap16IfLE(0);
				pFLD->attr = Swap16IfLE(0);
				pFLD->size = Swap32IfLE(0);
				strcpy(pFilename, "");
				if(size > 255) {
					m_socket->SendExact(pAllMessage, sz_rfbFileListDataMsg);
					break;
				}
				m_socket->ReadExact(path, size);
				path[size] = '\0';
				ConvertPath(path);
				if(strlen(path) == 0) {
					TCHAR szDrivesList[256];
					int szDrivesNum = 0;
					char fixeddrive [28] [4];
					if (GetLogicalDriveStrings(256, szDrivesList) == 0) {
						m_socket->SendExact(pAllMessage, sz_rfbFileListDataMsg);
						break;
					}
					for (int i=0, p=0; i<=255; i++, p++) {
						drive[p] = szDrivesList[i];
						if (szDrivesList[i] == '\0') {
							p = -1;
							switch (GetDriveType((LPCTSTR) drive))
								case DRIVE_REMOVABLE:
								case DRIVE_FIXED:
								case DRIVE_REMOTE:
								case DRIVE_CDROM:
								{
									drive[strlen(drive) - 1] = '\0';
									strcpy(fixeddrive[szDrivesNum], drive);
									szDrivesNum += 1;
								}
							if (szDrivesList[i+1] == '\0') break;
						}
					}
					pFLD->amount = Swap16IfLE(szDrivesNum);
					for(int ii=0; ii<szDrivesNum; ii++) {
						pFLD->attr = Swap16IfLE(0x0001);
						pFLD->size = Swap32IfLE(0);
						pFLD->fnamesize = strlen(fixeddrive[ii]);
						pFLD->num = Swap16IfLE(ii);
						strcpy(pFilename, fixeddrive[ii]);
						m_socket->SendExact(pAllMessage, sz_rfbFileListDataMsg + strlen(fixeddrive[ii]));
					}
				} else {
					strcat(path, "\\*");
					HANDLE FLRhandle;
 					int NumFiles = 0;
 					WIN32_FIND_DATA FindFileData;
					SetErrorMode(SEM_FAILCRITICALERRORS);
 					FLRhandle = FindFirstFile(path, &FindFileData);
					DWORD LastError = GetLastError();
					SetErrorMode(0);
					if (FLRhandle != INVALID_HANDLE_VALUE) {
						do {
							if (strcmp(FindFileData.cFileName, ".") != 0 &&
								strcmp(FindFileData.cFileName, "..") != 0) {
								NumFiles++;
							}
						} while (FindNextFile(FLRhandle, &FindFileData));
					} else {
						NumFiles = 0;
						if (LastError != ERROR_SUCCESS && LastError != ERROR_FILE_NOT_FOUND) 
							pFLD->attr = Swap16IfLE(0x0002);
 					}
 					FindClose(FLRhandle);	
					if(NumFiles == 0) {
						m_socket->SendExact(pAllMessage, sz_rfbFileListDataMsg);
						break;
					}
					pFLD->amount = Swap16IfLE(NumFiles);
					FLRhandle = FindFirstFile(path, &FindFileData);
					int i=0;
					while(1) {
						if((strcmp(FindFileData.cFileName, ".") != 0) &&
							(strcmp(FindFileData.cFileName, "..") != 0)) {
							if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {	
								pFLD->attr = Swap16IfLE(0x0001);
								pFLD->size = Swap32IfLE(0);
							} else {
								pFLD->attr = Swap16IfLE(0x0000);
								pFLD->size = Swap32IfLE(FindFileData.nFileSizeLow);
							}
							pFLD->fnamesize = strlen(FindFileData.cFileName);
							pFLD->num = Swap16IfLE(i);
							strcpy(pFilename, FindFileData.cFileName);
							m_socket->SendExact(pAllMessage, sz_rfbFileListDataMsg + strlen(FindFileData.cFileName));
							i++;
						}
						if (!FindNextFile(FLRhandle, &FindFileData)) break;
					}
					FindClose(FLRhandle);
				}
				delete [] pAllMessage;
			}
			break;

		case rfbFileDownloadRequest:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbFileDownloadRequestMsg-1))
			{
				const UINT size = msg.fdr.fnamesize;
				WIN32_FIND_DATA FindFileData;
				HANDLE hFile;
				BOOL bResult;
				DWORD filesize = NULL;
				DWORD dwNumberOfBytesRead = 0;
				DWORD dwNumberOfAllBytesRead = 0;
				DWORD sz_rfbFileSize;
				DWORD sz_rfbBlockSize = 8192;
				char path_file[255];
				char *pBuff = new char [sz_rfbFileDownloadDataMsg + sz_rfbBlockSize];
				rfbFileDownloadDataMsg *pfdd = (rfbFileDownloadDataMsg *)pBuff;
				char *lpBuff = &pBuff[sz_rfbFileDownloadDataMsg];
				m_socket->ReadExact(path_file, size);
				path_file[size] = '\0';
				ConvertPath(path_file);
				vnclog.Print(LL_CLIENTS, VNCLOG("file download requested: %s\n"),
							 path_file);
				pfdd->type = rfbFileDownloadData;
				hFile = FindFirstFile(path_file, &FindFileData);
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || 
					(hFile == INVALID_HANDLE_VALUE) || (path_file[0] == '\0')) {
					pfdd->amount = Swap16IfLE(0);
					pfdd->size = Swap16IfLE(0);
					m_socket->SendExact(pBuff, sz_rfbFileDownloadDataMsg);
					FindClose(hFile);
					break;
				}
				sz_rfbFileSize = FindFileData.nFileSizeLow;
				FindClose(hFile);
				int amount = (int) ((sz_rfbFileSize + sz_rfbBlockSize - 1) / sz_rfbBlockSize);
				if (sz_rfbFileSize <= sz_rfbBlockSize) sz_rfbBlockSize = sz_rfbFileSize;
				pfdd->amount = Swap16IfLE(amount);
				pfdd->size = Swap16IfLE(sz_rfbBlockSize);
				HANDLE hFiletoRead = CreateFile(path_file, GENERIC_READ, FILE_SHARE_READ, NULL,	OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);				// handle to file with attributes to copy  
				if (hFiletoRead != INVALID_HANDLE_VALUE) {
					for (int i=0; i<amount; i++) {
						pfdd->num = Swap16IfLE(i);
						bResult = ReadFile(hFiletoRead, lpBuff, sz_rfbBlockSize, &dwNumberOfBytesRead, NULL);
						dwNumberOfAllBytesRead += dwNumberOfBytesRead;
						m_socket->SendExact(pBuff, sz_rfbFileDownloadDataMsg + dwNumberOfBytesRead);
						if ((sz_rfbFileSize - dwNumberOfAllBytesRead) < sz_rfbBlockSize) {
							sz_rfbBlockSize = sz_rfbFileSize - dwNumberOfAllBytesRead;
							pfdd->size = Swap16IfLE(sz_rfbBlockSize);
						}
					}
				}
				CloseHandle(hFiletoRead);
				delete [] pBuff;
				vnclog.Print(LL_CLIENTS, VNCLOG("file download complete: %s\n"),
							 path_file);
			}
			break;

		case rfbFileUploadRequest:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbFileUploadRequestMsg-1))
			{
				m_socket->ReadExact(m_FullFilename, msg.fupr.fnamesize);
				m_FullFilename[msg.fupr.fnamesize] = '\0';
				ConvertPath(m_FullFilename);
				vnclog.Print(LL_CLIENTS, VNCLOG("file upload requested: %s\n"),
							 m_FullFilename);
				m_hFiletoWrite = CreateFile(m_FullFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			}				
			break;

		case rfbFileUploadData:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbFileUploadDataMsg-1))
			{
				msg.fud.amount = Swap16IfLE(msg.fud.amount);
				msg.fud.num = Swap16IfLE(msg.fud.num);
				msg.fud.size = Swap16IfLE(msg.fud.size);
				
				DWORD dwNumberOfBytesWritten;
				char *pBuff = new char [8192];
				m_socket->ReadExact(pBuff, msg.fud.size);
				WriteFile(m_hFiletoWrite, pBuff, msg.fud.size, &dwNumberOfBytesWritten, NULL);
				if (msg.fud.num == msg.fud.amount - 1) {
					CloseHandle(m_hFiletoWrite);
					vnclog.Print(LL_CLIENTS, VNCLOG("file upload complete: %s\n"),
								 m_FullFilename);
				}
				delete [] pBuff;
			}
			break;

		case rfbFileDownloadCancel:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			break;

		case rfbFileUploadFailed:
			if (!m_server->FileTransfersEnabled() ||
				(!m_client->m_keyboardenabled && !m_client->m_pointerenabled)) {
				connected = FALSE;
				break;
			}
			if (strcmp(m_FullFilename, "") != 0) {
				CloseHandle(m_hFiletoWrite);
				DeleteFile(m_FullFilename);
			}
			break;
#endif

		default:
			// Unknown message, so fail!
			vnclog.Print(LL_CLIENTS, VNCLOG("invalid message received : %d\n"),
						 (int)msg.type);
			connected = FALSE;
		}
	}

	// Move into the thread's original desktop
	vncService::SelectHDESK(home_desktop);

	// Quit this thread.  This will automatically delete the thread and the
	// associated client.
	vnclog.Print(LL_CLIENTS, VNCLOG("client disconnected : %s (id %hd)\n"),
				 m_client->GetClientName(), m_client->GetClientId());

	// Remove the client from the server, just in case!
	m_server->RemoveClient(m_client->GetClientId());
}

// The vncClient itself

vncClient::vncClient()
{
	vnclog.Print(LL_INTINFO, VNCLOG("vncClient() executing...\n"));

	m_socket = NULL;
	m_client_name = 0;
	m_buffer = NULL;

	m_copyrect_use = FALSE;

	m_mousemoved = FALSE;
	m_ptrevent.buttonMask = 0;
	m_ptrevent.x = 0;
	m_ptrevent.y = 0;

	m_cursor_update_pending = FALSE;
	m_cursor_update_sent = FALSE;
	m_cursor_pos_changed = FALSE;
	m_cursor_pos.x = -1;
	m_cursor_pos.y = -1;

	m_thread = NULL;
	m_updatewanted = FALSE;

	m_palettechanged = FALSE;

	m_copyrect_set = FALSE;

	m_pollingcycle = 0;
	m_remoteevent = FALSE;

	// IMPORTANT: Initially, client is not protocol-ready.
	m_protocol_ready = FALSE;
	
	m_use_NewFBSize = FALSE;
	
}

vncClient::~vncClient()
{
	vnclog.Print(LL_INTINFO, VNCLOG("~vncClient() executing...\n"));

	// We now know the thread is dead, so we can clean up
	if (m_client_name != 0) {
		free(m_client_name);
		m_client_name = 0;
	}

	// If we have a socket then kill it
	if (m_socket != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("deleting socket\n"));

		delete m_socket;
		m_socket = NULL;
	}

	// Kill the screen buffer
	if (m_buffer != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("deleting buffer\n"));

		delete m_buffer;
		m_buffer = NULL;
	}
}

// Init
BOOL
vncClient::Init(vncServer *server,
				VSocket *socket,
				BOOL auth,
				BOOL shared,
				vncClientId newid)
{
	// Save the server id;
	m_server = server;

	// Save the socket
	m_socket = socket;

	// Save the name of the connecting client
	char *name = m_socket->GetPeerName();
	if (name != 0)
		m_client_name = strdup(name);
	else
		m_client_name = strdup("<unknown>");

	// Save the client id
	m_id = newid;

	// Spawn the child thread here
	m_thread = new vncClientThread;
	if (m_thread == NULL)
		return FALSE;
	return ((vncClientThread *)m_thread)->Init(this, m_server, m_socket, auth, shared);

	return FALSE;
}

void
vncClient::Kill()
{
	// Close the socket
	if (m_socket != NULL)
		m_socket->Close();
}

// Client manipulation functions for use by the server
void
vncClient::SetBuffer(vncBuffer *buffer)
{
	// Until authenticated, the client object has no access
	// to the screen buffer.  This means that there only need
	// be a buffer when there's at least one authenticated client.
	m_buffer = buffer;
}


void
vncClient::TriggerUpdate()
{
	// Lock the updates stored so far
	omni_mutex_lock l(m_regionLock);
	if (!m_protocol_ready)
		return;
		
	if (m_updatewanted)
	{
		// Check if cursor shape update has to be sent
		m_cursor_update_pending = m_buffer->IsCursorUpdatePending();

		// Clear the remote event flag
		m_remoteevent = FALSE;

		// Send an update if one is waiting
		if (!m_changed_rgn.IsEmpty() ||
			!m_full_rgn.IsEmpty() ||
			m_copyrect_set ||
			m_cursor_update_pending ||
			m_cursor_pos_changed)
		{
			// Has the palette changed?
			if (m_palettechanged)
			{
				m_palettechanged = FALSE;
				if (!SendPalette())
					return;
			}
			
			// Now send the update
			m_updatewanted = !SendUpdate();
		}
	}
}

void
vncClient::UpdateMouse()
{
	POINT cursor;
	GetCursorPos(&cursor);
	// If mouse was moved
	if (!PtInRect(&m_oldmousepos, cursor)) {
	
		if (!m_mousemoved && !m_cursor_update_sent)	{
			omni_mutex_lock l(m_regionLock);

			if (IntersectRect(&m_oldmousepos, &m_oldmousepos, &m_server->getSharedRect()))
				m_changed_rgn.AddRect(m_oldmousepos);

			m_mousemoved = TRUE;
			m_updatewanted = TRUE;

		} else if (m_use_PointerPos) {
			m_cursor_pos_changed = TRUE;
		}
	}
}

void
vncClient::UpdateRect(RECT &rect)
{
	// Add the rectangle to the update region
	if (IsRectEmpty(&rect))
		return;
	
	omni_mutex_lock l(m_regionLock);
		
	if (IntersectRect(&rect, &rect, &m_server->getSharedRect()))
		m_changed_rgn.AddRect(rect);
}

void
vncClient::UpdateRegion(vncRegion &region)
{
	// Merge our current update region with the supplied one
	if (region.IsEmpty())
		return;

	{	omni_mutex_lock l(m_regionLock);
		
		// Merge the two
		vncRegion dummy;
		dummy.AddRect(m_server->getSharedRect());
		region.Intersect(dummy);

		m_changed_rgn.Combine(region);
	}
}

void
vncClient::CopyRect(RECT &dest, POINT &source)
{
	// If copyrect is disabled then just redraw the region!
	if (!m_copyrect_use)
	{
		UpdateRect(dest);
		return;
	}

	{	omni_mutex_lock l(m_regionLock);

		// Clip the destination to the screen
		RECT destrect;
		if (!IntersectRect(&destrect, &dest, &m_server->getSharedRect()))
			return;

		// Adjust the source correspondingly
		source.x = source.x + (destrect.left - dest.left);
		source.y = source.y + (destrect.top - dest.top);

		// Work out the source rectangle
		RECT srcrect;

		// Is this a continuation of an earlier window drag?
		if (m_copyrect_set &&
			((source.x == m_copyrect_rect.left) && (source.y == m_copyrect_rect.top)))
		{
			// Yes, so use the old source position
			srcrect.left = m_copyrect_src.x;
			srcrect.top = m_copyrect_src.y;
		}
		else
		{
			// No, so use this source position
			srcrect.left = source.x;
			srcrect.top = source.y;
		}

		// And fill out the right & bottom using the dest rect
		srcrect.right = destrect.right-destrect.left + srcrect.left;
		srcrect.bottom = destrect.bottom-destrect.top + srcrect.top;

		// Clip the source to the screen
		RECT srcrect2;
		if (!IntersectRect(&srcrect2, &srcrect, &m_server->getSharedRect()))
			return;

		// Correct the destination rectangle
		destrect.left += (srcrect2.left - srcrect.left);
		destrect.top += (srcrect2.top - srcrect.top);
		destrect.right = srcrect2.right-srcrect2.left + destrect.left;
		destrect.bottom = srcrect2.bottom-srcrect2.top + destrect.top;

		// Is there an existing CopyRect rectangle?
		if (m_copyrect_set)
		{
			// Yes, so compare their areas!
			if (((destrect.right-destrect.left) * (destrect.bottom-destrect.top))
				< ((m_copyrect_rect.right-m_copyrect_rect.left) * (m_copyrect_rect.bottom-m_copyrect_rect.top)))
				return;
		}

		// Set the copyrect...
		m_copyrect_rect = destrect;
		m_copyrect_src.x = srcrect2.left;
		m_copyrect_src.y = srcrect2.top;

		m_copyrect_set = TRUE;
	}
}

void
vncClient::UpdateClipText(LPSTR text)
{
	// Lock out any update sends and send clip text to the client
	omni_mutex_lock l(m_regionLock);
	if (!m_protocol_ready) return;

	rfbServerCutTextMsg message;
	message.length = Swap32IfLE(strlen(text));
	if (!SendRFBMsg(rfbServerCutText, (BYTE *) &message, sizeof(message)))
	{
		Kill();
		return;
	}
	if (!m_socket->SendQueued(text, strlen(text)))
	{
		Kill();
		return;
	}
}

void
vncClient::UpdatePalette()
{
	omni_mutex_lock l(m_regionLock);

	m_palettechanged = TRUE;
}

// Functions used to set and retrieve the client settings
const char*
vncClient::GetClientName()
{
	return m_client_name;
}

// Internal methods
BOOL
vncClient::SendRFBMsg(CARD8 type, BYTE *buffer, int buflen)
{
	// Set the message type
	((rfbServerToClientMsg *)buffer)->type = type;

	// Send the message
	if (!m_socket->SendQueued((char *) buffer, buflen))
	{
		vnclog.Print(LL_CONNERR, VNCLOG("failed to send RFB message to client\n"));

		Kill();
		return FALSE;
	}
	return TRUE;
}


BOOL
vncClient::SendUpdate()
{
	vncRegion toBeSent;			// Region to actually be sent
	rectlist toBeSentList;		// List of rectangles to actually send
	vncRegion toBeDone;			// Region to check

	// Prepare to send cursor position update if necessary
	if (m_cursor_pos_changed) {
		POINT cursor_pos;
		if (!GetCursorPos(&cursor_pos)) {
			cursor_pos.x = 0;
			cursor_pos.y = 0;
		}
		RECT shared_rect = m_server->getSharedRect();
		cursor_pos.x -= shared_rect.left;
		cursor_pos.y -= shared_rect.top;
		if (cursor_pos.x < 0) {
			cursor_pos.x = 0;
		} else if (cursor_pos.x >= shared_rect.right - shared_rect.left) {
			cursor_pos.x = shared_rect.right - shared_rect.left - 1;
		}
		if (cursor_pos.y < 0) {
			cursor_pos.y = 0;
		} else if (cursor_pos.y >= shared_rect.bottom - shared_rect.top) {
			cursor_pos.y = shared_rect.bottom - shared_rect.top - 1;
		}
		if (cursor_pos.x == m_cursor_pos.x && cursor_pos.y == m_cursor_pos.y) {
			m_cursor_pos_changed = FALSE;
		} else {
			m_cursor_pos.x = cursor_pos.x;
			m_cursor_pos.y = cursor_pos.y;
		}
	}

	toBeSent.Clear();
	if (!m_full_rgn.IsEmpty())	{
		m_incr_rgn.Clear();
		m_copyrect_set = false;
		toBeSent.Combine(m_full_rgn);	
		m_changed_rgn.Clear();
		m_full_rgn.Clear();
	
	} else {
		if (!m_incr_rgn.IsEmpty()) {
			// Get region to send from vncDesktop
			toBeSent.Combine(m_changed_rgn);
		
			// Mouse stuff for the case when cursor shape updates are off
			if (!m_cursor_update_sent && !m_cursor_update_pending) {
				if (!m_mousemoved) {
					vncRegion tmpMouseRgn;
					tmpMouseRgn.AddRect(m_oldmousepos);
					tmpMouseRgn.Intersect(toBeSent);
					if (!tmpMouseRgn.IsEmpty()) 
						m_mousemoved = true;
				}
				if (m_mousemoved) {
					m_oldmousepos = m_buffer->GrabMouse();
					if (IntersectRect(&m_oldmousepos, &m_oldmousepos, &m_server->getSharedRect())) 
						toBeSent.AddRect(m_oldmousepos);
					m_mousemoved = FALSE;
				}
			}
			m_changed_rgn.Clear();
		}
	}

	// Get the list of changed rectangles!
	int numrects = 0;
	if (toBeSent.Rectangles(toBeSentList))
	{
		// Find out how many rectangles this update will contain
		rectlist::iterator i;
		int numsubrects;
		for (i=toBeSentList.begin(); i != toBeSentList.end(); i++)
		{
			numsubrects = m_buffer->GetNumCodedRects(*i);

			// Skip rest rectangles if an encoder will use LastRect extension.
			if (numsubrects == 0) {
				numrects = 0xFFFF;
				break;
			}
			numrects += numsubrects;
		}
	}

	if (numrects != 0xFFFF) {
		// Count cursor shape and cursor position updates.
		if (m_cursor_update_pending)
			numrects++;
		if (m_cursor_pos_changed)
			numrects++;
		// Handle the copyrect region
		if (m_copyrect_set)
			numrects++;
		// If there are no rectangles then return
		if (numrects != 0)
			m_incr_rgn.Clear();
		else
			return FALSE;
	}

	// Otherwise, send <number of rectangles> header
	rfbFramebufferUpdateMsg header;
	header.nRects = Swap16IfLE(numrects);
	if (!SendRFBMsg(rfbFramebufferUpdate, (BYTE *) &header, sz_rfbFramebufferUpdateMsg))
		return TRUE;

	// Send mouse cursor shape update
	if (m_cursor_update_pending) {
		if (!SendCursorShapeUpdate())
			return TRUE;
	}

	// Send cursor position update
	if (m_cursor_pos_changed) {
		if (!SendCursorPosUpdate())
			return TRUE;
	}

	// Encode & send the copyrect
	if (m_copyrect_set) {
		m_copyrect_set = FALSE;
		if(!SendCopyRect(m_copyrect_rect, m_copyrect_src))
			return TRUE;
	}

	// Encode & send the actual rectangles
	if (!SendRectangles(toBeSentList))
		return TRUE;

	// Send LastRect marker if needed.
	if (numrects == 0xFFFF) {
		if (!SendLastRect())
			return TRUE;
	}

	// Both lists should be empty when we exit
	_ASSERT(toBeSentList.empty());

	return TRUE;
	

}

// Send a set of rectangles
BOOL
vncClient::SendRectangles(rectlist &rects)
{
	RECT rect;

	// Work through the list of rectangles, sending each one
	while(!rects.empty())
	{
		rect = rects.front();
		if (!SendRectangle(rect))
			return FALSE;
			
		rects.pop_front();
	}
	rects.clear();

	return TRUE;
}

// Tell the encoder to send a single rectangle
BOOL
vncClient::SendRectangle(RECT &rect)
{
	RECT SharedRect;
	{	omni_mutex_lock l(m_regionLock);
	SharedRect = m_server->getSharedRect();
	}
	IntersectRect(&rect, &rect, &SharedRect);
	// Get the buffer to encode the rectangle
		UINT bytes = m_buffer->TranslateRect(rect, m_socket, SharedRect.left, SharedRect.top);

	// Send the encoded data
	return m_socket->SendQueued((char *)(m_buffer->GetClientBuffer()), bytes);
}

// Send a single CopyRect message
BOOL
vncClient::SendCopyRect(RECT &dest, POINT &source)
{
	// Create the message header
	rfbFramebufferUpdateRectHeader copyrecthdr;
	copyrecthdr.r.x = Swap16IfLE(dest.left);
	copyrecthdr.r.y = Swap16IfLE(dest.top);
	copyrecthdr.r.w = Swap16IfLE(dest.right-dest.left);
	copyrecthdr.r.h = Swap16IfLE(dest.bottom-dest.top);
	copyrecthdr.encoding = Swap32IfLE(rfbEncodingCopyRect);

	// Create the CopyRect-specific section
	rfbCopyRect copyrectbody;
	copyrectbody.srcX = Swap16IfLE(source.x);
	copyrectbody.srcY = Swap16IfLE(source.y);
	
	// Now send the message;
	if (!m_socket->SendQueued((char *)&copyrecthdr, sizeof(copyrecthdr)))
		return FALSE;
	if (!m_socket->SendQueued((char *)&copyrectbody, sizeof(copyrectbody)))
		return FALSE;

	return TRUE;
}

// Send LastRect marker indicating that there are no more rectangles to send
BOOL
vncClient::SendLastRect()
{
	// Create the message header
	rfbFramebufferUpdateRectHeader hdr;
	hdr.r.x = 0;
	hdr.r.y = 0;
	hdr.r.w = 0;
	hdr.r.h = 0;
	hdr.encoding = Swap32IfLE(rfbEncodingLastRect);

	// Now send the message;
	if (!m_socket->SendQueued((char *)&hdr, sizeof(hdr)))
		return FALSE;

	return TRUE;
}

// Send the encoder-generated palette to the client
// This function only returns FALSE if the SendQueued fails - any other
// error is coped with internally...
BOOL
vncClient::SendPalette()
{
	rfbSetColourMapEntriesMsg setcmap;
	RGBQUAD *rgbquad;
	UINT ncolours = 256;

	// Reserve space for the colour data
	rgbquad = new RGBQUAD[ncolours];
	if (rgbquad == NULL)
		return TRUE;
					
	// Get the data
	if (!m_buffer->GetRemotePalette(rgbquad, ncolours))
	{
		delete [] rgbquad;
		return TRUE;
	}

	// Compose the message
	setcmap.type = rfbSetColourMapEntries;
	setcmap.firstColour = Swap16IfLE(0);
	setcmap.nColours = Swap16IfLE(ncolours);

	if (!m_socket->SendQueued((char *) &setcmap, sz_rfbSetColourMapEntriesMsg))
	{
		delete [] rgbquad;
		return FALSE;
	}

	// Now send the actual colour data...
	for (UINT i=0; i<ncolours; i++)
	{
		struct _PIXELDATA {
			CARD16 r, g, b;
		} pixeldata;

		pixeldata.r = Swap16IfLE(((CARD16)rgbquad[i].rgbRed) << 8);
		pixeldata.g = Swap16IfLE(((CARD16)rgbquad[i].rgbGreen) << 8);
		pixeldata.b = Swap16IfLE(((CARD16)rgbquad[i].rgbBlue) << 8);

		if (!m_socket->SendQueued((char *) &pixeldata, sizeof(pixeldata)))
		{
			delete [] rgbquad;
			return FALSE;
		}
	}

	// Delete the rgbquad data
	delete [] rgbquad;

	return TRUE;
}

BOOL
vncClient::SendCursorShapeUpdate()
{
	m_cursor_update_pending = FALSE;

	if (!m_buffer->SendCursorShape(m_socket)) {
		m_cursor_update_sent = FALSE;
		return m_buffer->SendEmptyCursorShape(m_socket);
	}

	m_cursor_update_sent = TRUE;
	return TRUE;
}

BOOL
vncClient::SendCursorPosUpdate()
{
	m_cursor_pos_changed = FALSE;

	rfbFramebufferUpdateRectHeader hdr;
	hdr.encoding = Swap32IfLE(rfbEncodingPointerPos);
	hdr.r.x = Swap16IfLE(m_cursor_pos.x);
	hdr.r.y = Swap16IfLE(m_cursor_pos.y);
	hdr.r.w = Swap16IfLE(0);
	hdr.r.h = Swap16IfLE(0);

	return m_socket->SendQueued((char *)&hdr, sizeof(hdr));
}

// Send NewFBSize pseudo-rectangle to notify the client about
// framebuffer size change
BOOL
vncClient::SetNewFBSize(BOOL sendnewfb)
{
	rfbFramebufferUpdateRectHeader hdr;
	RECT SharedRect;
	
	SharedRect = m_server->getSharedRect();
	
	m_pollingcycle = 0;
	m_full_rgn.Clear();
	m_incr_rgn.Clear();
	m_full_rgn.AddRect(SharedRect);
	
		
	if (m_use_NewFBSize && sendnewfb) {
		hdr.r.x = 0;
		hdr.r.y = 0;
		hdr.r.w = Swap16IfLE(SharedRect.right - SharedRect.left);
		hdr.r.h = Swap16IfLE(SharedRect.bottom - SharedRect.top);
		hdr.encoding = Swap32IfLE(rfbEncodingNewFBSize);

		rfbFramebufferUpdateMsg header;
		header.nRects = Swap16IfLE(1);
		if (!SendRFBMsg(rfbFramebufferUpdate, (BYTE *)&header,
						sz_rfbFramebufferUpdateMsg))
			return FALSE;

		// Now send the message;
		if (!m_socket->SendQueued((char *)&hdr, sizeof(hdr)))
			return FALSE;
	}

	return TRUE;
}

void
vncClient::UpdateLocalFormat()
{
	m_buffer->UpdateLocalFormat();
}

char * 
vncClientThread::ConvertPath(char *path)
{
	int len = strlen(path);
	if(len >= 255) return path;
	if((path[0] == '/') && (len == 1)) {path[0] = '\0'; return path;}
	for(int i = 0; i < (len - 1); i++) {
		if(path[i+1] == '/') path[i+1] = '\\';
		path[i] = path[i+1];
	}
	path[len-1] = '\0';
	return path;
}
