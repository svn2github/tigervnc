//  Copyright (C) 2000 Const Kaplinsky. All Rights Reserved.
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
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.tridiavnc.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncEncodeTight object

class vncEncodeTight;

#if !defined(_WINVNC_ENCODETIGHT)
#define _WINVNC_ENCODETIGHT
#pragma once

#include "vncEncoder.h"

#include "zlib/zlib.h"

// Minimum amount of data to be compressed. This value should not be
// changed, doing so will break compatibility with existing clients.
#define TIGHT_MIN_TO_COMPRESS 12

// Maximum size of one tight rectangle. This value may be changed.
#define TIGHT_MAX_RECT_SIZE  65536

// C-style structures to store palette entries.
// Such code probably should be converted into C++ classes.

typedef struct COLOR_LIST_s {
	struct COLOR_LIST_s *next;
	int idx;
	CARD32 rgb;
} COLOR_LIST;

typedef struct PALETTE_ENTRY_s {
	COLOR_LIST *listNode;
	int numPixels;
} PALETTE_ENTRY;

typedef struct PALETTE_s {
	PALETTE_ENTRY entry[256];
	COLOR_LIST *hash[256];
	COLOR_LIST list[256];
} PALETTE;

typedef struct PALETTE8_s {
	CARD8 pixelValue[2];
	CARD8 colorIdx[256];
} PALETTE8;


// Class definition

class vncEncodeTight : public vncEncoder
{
// Fields
public:

// Methods
public:
	// Create/Destroy methods
	vncEncodeTight();
	~vncEncodeTight();

	virtual void Init();

	virtual UINT RequiredBuffSize(UINT width, UINT height);
	virtual UINT NumCodedRects(RECT &rect);

	virtual UINT EncodeRect(BYTE *source, VSocket *outConn, BYTE *dest, const RECT &rect);

// Implementation
protected:
	int m_paletteMaxColors;
	int m_paletteNumColors;
	PALETTE m_palette;
	PALETTE8 m_palette8;

	z_stream m_zsStruct[4];
	bool m_zsActive[4];

	BYTE *m_hdrBuffer;
	int m_hdrBufferBytes;
	BYTE *m_buffer;
	int m_bufflen;
	int *m_prevRowBuf;

	bool m_usePixelFormat24;

	UINT EncodeSubrect(BYTE *source, VSocket *outConn, BYTE *dest,
					   int x, int y, int w, int h);
	int SendSolidRect(BYTE *dest, int w, int h);
	int SendIndexedRect(BYTE *dest, int w, int h);
	int SendFullColorRect(BYTE *dest, int w, int h);
	int SendGradientRect(BYTE *dest, int w, int h);
	int CompressData(BYTE *dest, int streamId, int dataLen);

	void FillPalette8(int w, int h);
	void FillPalette16(int w, int h);
	void FillPalette32(int w, int h);

	void PaletteReset(void);
	int PaletteFind(CARD32 rgb);
	int PaletteInsert(CARD32 rgb, int numPixels);

	void Pack24(BYTE *buf, int count);

	void EncodeIndexedRect8(BYTE *buf, int w, int h);
	void EncodeIndexedRect16(BYTE *buf, int w, int h);
	void EncodeIndexedRect32(BYTE *buf, int w, int h);

	void FilterGradient24(BYTE *buf, int w, int h);
	void FilterGradient16(CARD16 *buf, int w, int h);
	void FilterGradient32(CARD32 *buf, int w, int h);

	int DetectStillImage (int w, int h);
	int DetectStillImage24 (int w, int h);
	int DetectStillImage16 (int w, int h);
	int DetectStillImage32 (int w, int h);
};

#endif // _WINVNC_ENCODETIGHT

