/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
//  This program is free software; you can redistribute it and/or modify
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
// If the source code for the program is not available from the place from
// which you received this file, check 
// http://ultravnc.sourceforge.net/
//
/////////////////////////////////////////////////////////////////////////////
#include "stdhdrs.h"

#define MAP1 1030
#define UNMAP1 1031
#define TESTDRIVER 1050
#define TESTMAPPED 1051

#define MAXCHANGES_BUF 2000


#define IGNORE 0
#define BLIT 12
#define TEXTOUT 18
#define MOUSEPTR 48

#ifndef VIDEODRIVER
#define VIDEODRIVER

#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#define CDS_GLOBAL          0x00000008
#define CDS_SET_PRIMARY     0x00000010
#define CDS_RESET           0x40000000
#define CDS_SETRECT         0x20000000
#define CDS_NORESET         0x10000000

typedef BOOL (WINAPI* pEnumDisplayDevices)(PVOID,DWORD,PVOID,DWORD);

//*********************************************************************

typedef struct _CHANGES_RECORD
{
	ULONG type;  //screen_to_screen, blit, newcache,oldcache
	RECT rect;
	RECT origrect;
	POINT point;
	ULONG color; //number used in cache array
	ULONG refcolor; //slot used to pase btimap data
}CHANGES_RECORD;
typedef CHANGES_RECORD *PCHANGES_RECORD;
typedef struct _CHANGES_BUF
	{
	 ULONG counter;
	 CHANGES_RECORD pointrect[MAXCHANGES_BUF];
	}CHANGES_BUF;
typedef CHANGES_BUF *PCHANGES_BUF;

typedef struct _GETCHANGESBUF
	{
	 PCHANGES_BUF buffer;
	 PVOID Userbuffer;
	}GETCHANGESBUF;
typedef GETCHANGESBUF *PGETCHANGESBUF;

class vncVideoDriver
{

// Fields
public:

// Methods
public:
	// Make the desktop thread & window proc friends

	vncVideoDriver();
	~vncVideoDriver();
	BOOL Activate_video_driver();
	void DesActivate_video_driver();
	BOOL MapSharedbuffers();
	void UnMapSharedbuffers();
	BOOL testdriver();
	BOOL testmapped();

	GETCHANGESBUF bufdata;
	ULONG oldaantal;
	HDC gdc;
	BOOL driver;
	
protected:
		
};
#endif