//  Copyright (C) 2002 Constantin Kaplinsky. All Rights Reserved.
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


// vncHTTPConnect.cpp

// Implementation of the HTTP server class

#include "stdhdrs.h"
#include "VSocket.h"
#include "vncHTTPConnect.h"
#include "vncServer.h"
#include <omnithread.h>
#include "resource.h"

// HTTP messages/message formats
const char HTTP_MSG_OK []			="HTTP/1.0 200 OK\n\n";
const char HTTP_FMT_INDEX[]			="<HTML><TITLE>TightVNC desktop [%.256s]</TITLE>\n"
	"<APPLET CODE=VncViewer.class ARCHIVE=VncViewer.jar WIDTH=%d HEIGHT=%d>\n"
	"<param name=PORT value=%d><param name=ENCODING value=Tight></APPLET>\n"
	"<BR>\n"
	"<A href=\"http://www.tightvnc.com/\">www.TightVNC.com</A></HTML>\n";
const char HTTP_MSG_NOSOCKCONN []	="<HTML><TITLE>TightVNC desktop</TITLE>\n"
	"<BODY>The requested desktop is not configured to accept incoming connections.</BODY>\n"
	"</HTML>\n";
const char HTTP_MSG_NOSUCHFILE []	="HTTP/1.0 404 Not found\n\n"
    "<HEAD><TITLE>File Not Found</TITLE></HEAD>\n"
    "<BODY><H1>The requested file could not be found</H1></BODY>\n";

// Filename to resource ID mappings for the Java class files:
typedef struct _FileToResourceMap {
	char *filename;
	char *type;
	int resourceID;
} FileMap;

const FileMap filemapping []	={
	{"/VncViewer.jar", "JavaArchive", IDR_VNCVIEWER_JAR},
	{"/AuthPanel.class", "JavaClass", IDR_AUTHPANEL_CLASS},
	{"/ClipboardFrame.class", "JavaClass", IDR_CLIPBOARDFRAME_CLASS},
	{"/DesCipher.class", "JavaClass", IDR_DESCIPHER_CLASS},
	{"/OptionsFrame.class", "JavaClass", IDR_OPTIONSFRAME_CLASS},
	{"/RfbProto.class", "JavaClass", IDR_RFBPROTO_CLASS},
	{"/VncCanvas.class", "JavaClass", IDR_VNCCANVAS_CLASS},
	{"/VncViewer.class", "JavaClass", IDR_VNCVIEWER_CLASS},
	{"/ButtonPanel.class", "JavaClass", IDR_BUTTONPANEL_CLASS}
	};
const int filemappingsize		= 9;

//
// Connection thread -- one per each client connection.
//

class vncHTTPConnectThread : public omni_thread
{
public:
	// Init routine
	virtual BOOL Init(VSocket *socket, vncServer *server);

	// Code to be executed by the thread
	virtual void run(void *arg);

	// Routines to handle HTTP requests
	virtual void DoHTTP(VSocket *socket);
	virtual char *ReadLine(VSocket *socket, char delimiter, int max);

protected:
	// Fields used internally
	vncServer	*m_server;
	VSocket		*m_socket;
};

// Method implementations
BOOL vncHTTPConnectThread::Init(VSocket *socket, vncServer *server)
{
	m_server = server;
	m_socket = socket;

	// Start the thread
	start();

	return TRUE;
}

// Code to be executed by the thread
void vncHTTPConnectThread::run(void *arg)
{
	vnclog.Print(LL_INTINFO, VNCLOG("started HTTP client thread\n"));

	// Perform the transaction
	DoHTTP(m_socket);

	// And close the client
	m_socket->Shutdown();
	m_socket->Close();
	delete m_socket;

	vnclog.Print(LL_INTINFO, VNCLOG("quitting HTTP client thread\n"));
}

void vncHTTPConnectThread::DoHTTP(VSocket *socket)
{
	char filename[1024];
	char *line;

	// Read in the HTTP header
	if ((line = ReadLine(socket, '\n', 1024)) == NULL)
		return;

	// Scan the header for the filename and free the storage
	int result = sscanf(line, "GET %s HTTP/", filename);
	delete [] line;
	if (result != 1)
		return;

	vnclog.Print(LL_CLIENTS, VNCLOG("file %s requested\n"), filename);

	// Read in the rest of the browser's request data and discard...
	BOOL newline = TRUE;

	char c;
	for (;;) {
		if (!socket->ReadExact(&c, 1))
			return;
		if (c == '\n') {
			if (newline)
				break;
			newline = TRUE;
		} else {
			if (c >= ' ')
				newline = FALSE;
		}
	}

	vnclog.Print(LL_INTINFO, VNCLOG("HTTP headers skipped\n"));

    if (filename[0] != '/') {
		vnclog.Print(LL_CONNERR, VNCLOG("filename didn't begin with '/'\n"));
		socket->SendExact(HTTP_MSG_NOSUCHFILE, strlen(HTTP_MSG_NOSUCHFILE));
		return;
	}

	// Switch, dependent upon the filename:
	if (strcmp(filename, "/") == 0)
	{
		char indexpage[1024];

		vnclog.Print(LL_CLIENTS, VNCLOG("sending main page\n"));

		// Send the OK notification message to the client
		if (!socket->SendExact(HTTP_MSG_OK, strlen(HTTP_MSG_OK)))
			return;

		// Compose the index page
		if (m_server->SockConnected())
		{
			int width, height, depth;

			// Get the screen's dimensions
			m_server->GetScreenInfo(width, height, depth);

			// Get the name of this desktop
			char desktopname[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD desktopnamelen = MAX_COMPUTERNAME_LENGTH + 1;
			if (GetComputerName(desktopname, &desktopnamelen))
			{
				// Make the name lowercase
				for (int x=0; x<strlen(desktopname); x++)
				{
					desktopname[x] = tolower(desktopname[x]);
				}
			}
			else
			{
				strcpy(desktopname, "WinVNC");
			}

			// Send the java applet page
			sprintf(indexpage, HTTP_FMT_INDEX,
				desktopname, width, height+32,
				m_server->GetPort()
				);
		}
		else
		{
			// Send a "sorry, not allowed" page
			sprintf(indexpage, HTTP_MSG_NOSOCKCONN);
		}

		// Send the page
		if (socket->SendExact(indexpage, strlen(indexpage)))
			vnclog.Print(LL_INTINFO, VNCLOG("sent page\n"));

		return;
	}

	// File requested was not the index so check the mappings
	// list for a different file.

	// Now search the mappings for the desired file
	for (int x=0; x < filemappingsize; x++)
	{
		if (strcmp(filename, filemapping[x].filename) == 0)
		{
			HRSRC resource;
			HGLOBAL resourcehan;
			char *resourceptr;
			int resourcesize;

			vnclog.Print(LL_INTINFO, VNCLOG("requested file recognised\n"));

			// Find the resource here
			resource = FindResource(NULL,
					MAKEINTRESOURCE(filemapping[x].resourceID),
					filemapping[x].type
					);
			if (resource == NULL)
				return;

			// Get its size
			resourcesize = SizeofResource(NULL, resource);

			// Load the resource
			resourcehan = LoadResource(NULL, resource);
			if (resourcehan == NULL)
				return;

			// Lock the resource
			resourceptr = (char *)LockResource(resourcehan);
			if (resourceptr == NULL)
				return;

			vnclog.Print(LL_INTINFO, VNCLOG("sending file...\n"));

			// Send the OK message
			if (!socket->SendExact(HTTP_MSG_OK, strlen(HTTP_MSG_OK)))
				return;

			// Now send the entirety of the data to the client
			if (!socket->SendExact(resourceptr, resourcesize))
				return;

			vnclog.Print(LL_INTINFO, VNCLOG("file successfully sent\n"));

			return;
		}
	}

	// Send the NoSuchFile notification message to the client
	if (!socket->SendExact(HTTP_MSG_NOSUCHFILE, strlen(HTTP_MSG_NOSUCHFILE)))
		return;
}

char *vncHTTPConnectThread::ReadLine(VSocket *socket, char delimiter, int max)
{
	// Allocate the maximum required buffer
	char *buffer = new char[max+1];
	int buffpos = 0;

	// Read in data until a delimiter is read
	for (;;)
	{
		char c;

		if (!socket->ReadExact(&c, 1))
		{
			delete [] buffer;
			return NULL;
		}

		if (c == delimiter)
		{
			buffer[buffpos] = 0;
			return buffer;
		}

		buffer[buffpos] = c;
		buffpos++;

		if (buffpos == (max-1))
		{
			buffer[buffpos] = 0;
			return buffer;
		}
	}
}

//
// Listening thread.
//

class vncHTTPListenThread : public omni_thread
{
public:
	// Init routine
	virtual BOOL Init(VSocket *socket, vncServer *server);

	// Code to be executed by the thread
	virtual void *run_undetached(void * arg);

	// Fields used internally
	BOOL		m_shutdown;

protected:
	vncServer	*m_server;
	VSocket		*m_listen_socket;
};

BOOL vncHTTPListenThread::Init(VSocket *listen_socket, vncServer *server)
{
	m_server = server;
	m_listen_socket = listen_socket;

	// Start the thread
	m_shutdown = FALSE;
	start_undetached();

	return TRUE;
}

// Code to be executed by the thread
void *vncHTTPListenThread::run_undetached(void * arg)
{
	vnclog.Print(LL_INTINFO, VNCLOG("started HTTP server thread\n"));

	// Go into a loop, listening for connections on the given socket
	while (!m_shutdown)
	{
		// Accept an incoming connection
		VSocket *new_socket = m_listen_socket->Accept();
		if (new_socket == NULL)
			break;

		// Start a client thread for this connection
		vnclog.Print(LL_CLIENTS, VNCLOG("HTTP client connected\n"));
		omni_thread *m_thread = new vncHTTPConnectThread;
		if (m_thread == NULL)
			break;
		((vncHTTPConnectThread *)m_thread)->Init(new_socket, m_server);
	}

	vnclog.Print(LL_INTINFO, VNCLOG("quitting HTTP server thread\n"));

	return NULL;
}

//
// The vncSockConnect class implementation
//

vncHTTPConnect::vncHTTPConnect()
{
	m_listen_thread = NULL;
}

BOOL vncHTTPConnect::Init(vncServer *server, UINT listen_port)
{
	// Save the port number
	m_listen_port = listen_port;

	// Create the listening socket
	if (!m_listen_socket.Create())
		return FALSE;

	// Bind it
	if (!m_listen_socket.Bind(m_listen_port, server->LoopbackOnly()))
		return FALSE;

	// Set it to listen
	if (!m_listen_socket.Listen())
		return FALSE;

	// Create the new thread
	m_listen_thread = new vncHTTPListenThread;
	if (m_listen_thread == NULL)
		return FALSE;

	// And start it running
	return ((vncHTTPListenThread *)m_listen_thread)->Init(&m_listen_socket, server);
}

vncHTTPConnect::~vncHTTPConnect()
{
	m_listen_socket.Shutdown();

	// Join with our lovely thread
	if (m_listen_thread != NULL) {
		// *** This is a hack to force the listen thread out of the accept call,
		// because Winsock accept semantics are broken.
		((vncHTTPListenThread *)m_listen_thread)->m_shutdown = TRUE;

		VSocket tmp_socket;
		tmp_socket.Create();
		tmp_socket.Connect("localhost", m_listen_port);
		tmp_socket.Close();

		void *returnval;
		m_listen_thread->join(&returnval);
		m_listen_thread = NULL;

		m_listen_socket.Close();
    }
}

