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

// Default compression level for the tight encoder (1..9). This value
// may be changed.
#define TIGHT_DEFAULT_COMPRESSION  6

// C-style structures to store palette entries and compression paramentes.
// Such code probably should be converted into C++ classes.

struct COLOR_LIST {
	COLOR_LIST *next;
	int idx;
	CARD32 rgb;
};

struct PALETTE_ENTRY {
	COLOR_LIST *listNode;
	int numPixels;
};

struct PALETTE {
	PALETTE_ENTRY entry[256];
	COLOR_LIST *hash[256];
	COLOR_LIST list[256];
};

struct TIGHT_CONF {
	int maxRectSize, maxRectWidth;
	int monoMinRectSize, gradientMinRectSize;
	int idxZlibLevel, monoZlibLevel, rawZlibLevel, gradientZlibLevel;
	int gradientThreshold, gradientThreshold24;
	int idxMaxColorsDivisor;
};


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
	int m_paletteNumColors, m_paletteMaxColors;
	CARD32 m_monoBackground, m_monoForeground;
	PALETTE m_palette;

	z_stream m_zsStruct[4];
	bool m_zsActive[4];
	int m_zsLevel[4];

	BYTE *m_hdrBuffer;
	int m_hdrBufferBytes;
	BYTE *m_buffer;
	int m_bufflen;
	int *m_prevRowBuf;

	bool m_usePixelFormat24;

	// Compression level stuff. The following array contains various
	// encoder parameters for each of 10 compression levels (0..9).
	//
	// NOTE: m_conf[9].maxRectSize should be >= m_conf[i].maxRectSize,
	// where i in [0..8]. RequiredBuffSize() method depends on this.

	static const TIGHT_CONF m_conf[10] = {
		{  1024,   64,   6, 65536, 0, 0, 0, 0,   0,   0,   4 },
		{  2048,  128,   6, 65536, 1, 1, 1, 0,   0,   0,  12 },
		{  6144,  256,   8, 65536, 3, 3, 2, 0,   0,   0,  24 },
		{ 10240, 1024,  12, 65536, 5, 5, 3, 0,   0,   0,  32 },
		{ 16384, 2048,  12, 65536, 6, 6, 4, 0,   0,   0,  32 },
		{ 32768, 2048,  12,  4096, 7, 7, 5, 4, 150, 380,  32 },
		{ 65536, 2048,  16,  4096, 7, 7, 6, 4, 170, 420,  48 },
		{ 65536, 2048,  16,  4096, 8, 8, 7, 5, 180, 450,  64 },
		{ 65536, 2048,  32,  8192, 9, 9, 8, 6, 190, 475,  64 },
		{ 65536, 2048,  32,  8192, 9, 9, 9, 6, 200, 500,  96 }
	};

	int m_compressLevel;

	// Protected member functions.

	UINT EncodeSubrect(BYTE *source, VSocket *outConn, BYTE *dest,
					   int x, int y, int w, int h);
	int SendSolidRect(BYTE *dest, int w, int h);
	int SendMonoRect(BYTE *dest, int w, int h);
	int SendIndexedRect(BYTE *dest, int w, int h);
	int SendFullColorRect(BYTE *dest, int w, int h);
	int SendGradientRect(BYTE *dest, int w, int h);
	int CompressData(BYTE *dest, int streamId, int dataLen,
					 int zlibLevel, int zlibStrategy);

	void FillPalette8(int count);
	void FillPalette16(int count);
	void FillPalette32(int count);

	void PaletteReset(void);
	int PaletteInsert(CARD32 rgb, int numPixels, int bpp);

	void Pack24(BYTE *buf, int count);

	void EncodeIndexedRect16(BYTE *buf, int count);
	void EncodeIndexedRect32(BYTE *buf, int count);

	void EncodeMonoRect8(BYTE *buf, int w, int h);
	void EncodeMonoRect16(BYTE *buf, int w, int h);
	void EncodeMonoRect32(BYTE *buf, int w, int h);

	void FilterGradient24(BYTE *buf, int w, int h);
	void FilterGradient16(CARD16 *buf, int w, int h);
	void FilterGradient32(CARD32 *buf, int w, int h);

	int DetectStillImage (int w, int h);
	int DetectStillImage24 (int w, int h);
	int DetectStillImage16 (int w, int h);
	int DetectStillImage32 (int w, int h);
};

#endif // _WINVNC_ENCODETIGHT

