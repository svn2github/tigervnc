
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id$

#ifndef __APPSHAREMAIN_H
#define __APPSHAREMAIN_H

// stl first
#include <algorithm>
#include <fstream>
#include <string>

using std::ofstream ;
using std::string ;

#include "stdhdrs.h"
#include "resource.h"

#include "VSocket.h"
#include "Log.h"

#include "vncInstHandler.h"
#include "vncServer.h"
#include "vncService.h"

#include "horizonMenu.h"
#include "horizonDefaults.h"
#include "horizonProperties.h"

// Application specific messages

// Message used for system tray notifications
#define WM_TRAYNOTIFY				WM_USER + 1

// Messages used for the server object to notify windows of things
#define WM_SRV_CLIENT_CONNECT		WM_USER + 2
#define WM_SRV_CLIENT_AUTHENTICATED	WM_USER + 3
#define WM_SRV_CLIENT_DISCONNECT	WM_USER + 4

#define WINVNC_REGISTRY_KEY "Software\\HorizonWimba\\AppShareHost"

//
// external globals
//

extern const char* szAppName ;
extern const char* szProcessName ;

extern HINSTANCE hAppInstance ;
extern DWORD mainthreadId ;

// custom signal for requesting quit
extern const int LS_QUIT ;

//
// needed by vncService
//

const char winvncRunService[]		= "-service" ;
const char winvncRunServiceHelper[]	= "-servicehelper" ;


//
// functions declarations
//

int AppShareMain( const string& args ) ;
void AppShareUsage( const string& args ) ;

int WinVNCAppMain( void ) ;


#endif // __APPSHAREMAIN_H
