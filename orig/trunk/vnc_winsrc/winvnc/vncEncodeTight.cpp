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
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncEncodeTight

// This file implements the vncEncoder-derived vncEncodeTight class.
// This class overrides some vncEncoder functions to produce a bitmap
// to Tight encoder. Tight is much more efficient than RAW format on
// most screen data and usually 2..4 times as efficient as hextile.
// It's also more efficient than Zlib encoding in most cases.
// But note that tight compression may use more CPU time on the server.
// However, over slower (64kbps or less) connections, the reduction
// in data transmitted usually outweighs the extra latency added
// while the server CPU performs the compression algorithms.

#include "vncEncodeTight.h"

// Compression level stuff. The following array contains various
// encoder parameters for each of 10 compression levels (0..9).
//
// NOTE: m_conf[9].maxRectSize should be >= m_conf[i].maxRectSize,
// where i in [0..8]. RequiredBuffSize() method depends on this.

const TIGHT_CONF vncEncodeTight::m_conf[10] = {
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

vncEncodeTight::vncEncodeTight()
{
	m_buffer = NULL;
	m_bufflen = 0;

	m_hdrBuffer = new BYTE [sz_rfbFramebufferUpdateRectHeader + 8 + 256*4];
	m_prevRowBuf = NULL;

	m_compresslevel = TIGHT_DEFAULT_COMPRESSION;
	for (int i = 0; i < 4; i++)
		m_zsActive[i] = false;
}

vncEncodeTight::~vncEncodeTight()
{
	if (m_buffer != NULL) {
		delete[] m_buffer;
		m_buffer = NULL;
	}

	delete[] m_hdrBuffer;

	for (int i = 0; i < 4; i++) {
		if (m_zsActive[i])
			deflateEnd(&m_zsStruct[i]);
		m_zsActive[i] = false;
	}
}

void
vncEncodeTight::Init()
{
	vncEncoder::Init();
}

UINT
vncEncodeTight::RequiredBuffSize(UINT width, UINT height)
{
	// FIXME: Use actual compression level instead of 9?
	int result = m_conf[9].maxRectSize * (m_remoteformat.bitsPerPixel / 8);
	result += result / 100 + 16;

	return result;
}

UINT
vncEncodeTight::NumCodedRects(RECT &rect)
{
	const int w = rect.right - rect.left;
	const int h = rect.bottom - rect.top;

	const int maxRectSize = m_conf[m_compresslevel].maxRectSize;
	const int maxRectWidth = m_conf[m_compresslevel].maxRectWidth;

	if (w > maxRectWidth || w * h > maxRectSize) {
		const int subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
		const int subrectMaxHeight = maxRectSize / subrectMaxWidth;
		return (((w - 1) / maxRectWidth + 1) *
				((h - 1) / subrectMaxHeight + 1));
	} else {
		return 1;
	}
}


/*****************************************************************************
 *
 * Routines to implement Tight Encoding.
 *
 */

UINT
vncEncodeTight::EncodeRect(BYTE *source, VSocket *outConn, BYTE *dest,
						   const RECT &rect)
{
	const int x = rect.left, y = rect.top;
	const int w = rect.right - x, h = rect.bottom - y;

	if ( m_remoteformat.depth == 24 && m_remoteformat.redMax == 0xFF &&
		 m_remoteformat.greenMax == 0xFF && m_remoteformat.blueMax == 0xFF ) {
		m_usePixelFormat24 = true;
	} else {
		m_usePixelFormat24 = false;
	}

	const int maxRectSize = m_conf[m_compresslevel].maxRectSize;
	const int maxRectWidth = m_conf[m_compresslevel].maxRectWidth;

	const int rawDataSize = maxRectSize * (m_remoteformat.bitsPerPixel / 8);
	if (m_bufflen < rawDataSize) {
		if (m_buffer != NULL)
			delete [] m_buffer;

		m_buffer = new BYTE [rawDataSize+1];
		if (m_buffer == NULL)
			return vncEncoder::EncodeRect(source, dest, rect);

		m_bufflen = rawDataSize;
	}

	int totalSize = 0;
	int partialSize = 0;

	if (w > maxRectWidth || w * h > maxRectSize) {
		const int subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
		const int subrectMaxHeight = maxRectSize / subrectMaxWidth;
		int dx, dy, rw, rh;

		for (dy = 0; dy < h; dy += subrectMaxHeight) {
			for (dx = 0; dx < w; dx += maxRectWidth) {
				rw = (dx + maxRectWidth < w) ? maxRectWidth : w - dx;
				rh = (dy + subrectMaxHeight < h) ? subrectMaxHeight : h - dy;

				partialSize = EncodeSubrect(source, outConn, dest,
											x+dx, y+dy, rw, rh);
				totalSize += partialSize;
				if (dy + subrectMaxHeight < h || dx + maxRectWidth < w)
					outConn->SendExact((char *)dest, partialSize);
			}
		}
	} else {
		partialSize = EncodeSubrect(source, outConn, dest, x, y, w, h);
		totalSize += partialSize;
	}

	return partialSize;
}

UINT
vncEncodeTight::EncodeSubrect(BYTE *source, VSocket *outConn, BYTE *dest,
							  int x, int y, int w, int h)
{
	rfbFramebufferUpdateRectHeader hdr;

	hdr.r.x = Swap16IfLE(x);
	hdr.r.y = Swap16IfLE(y);
	hdr.r.w = Swap16IfLE(w);
	hdr.r.h = Swap16IfLE(h);
	hdr.encoding = Swap32IfLE(rfbEncodingTight);

	memcpy(m_hdrBuffer, (BYTE *)&hdr, sz_rfbFramebufferUpdateRectHeader);
	m_hdrBufferBytes = sz_rfbFramebufferUpdateRectHeader;

	RECT r;
	r.left = x; r.top = y;
	r.right = x + w; r.bottom = y + h;
	Translate(source, m_buffer, r);

	m_paletteMaxColors = w * h / m_conf[m_compresslevel].idxMaxColorsDivisor;
	if ( m_paletteMaxColors < 2 &&
		 w * h >= m_conf[m_compresslevel].monoMinRectSize ) {
		m_paletteMaxColors = 2;
	}
	switch (m_remoteformat.bitsPerPixel) {
	case 8:
		FillPalette8(w * h);
		break;
	case 16:
		FillPalette16(w * h);
		break;
	default:
		FillPalette32(w * h);
	}

	int dataSize;
	switch (m_paletteNumColors) {
	case 0:
		// Truecolor image
		if (DetectStillImage(w, h)) {
			dataSize = SendGradientRect(dest, w, h);
		} else {
			dataSize = SendFullColorRect(dest, w, h);
		}
		break;
	case 1:
		// Solid rectangle
		dataSize = SendSolidRect(dest, w, h);
		break;
	case 2:
		// Two-color rectangle
		dataSize = SendMonoRect(dest, w, h);
		break;
	default:
		// Up to 256 different colors
		dataSize = SendIndexedRect(dest, w, h);
	}

	if (dataSize < 0)
		return vncEncoder::EncodeRect(source, dest, r);

	outConn->SendExact((char *)m_hdrBuffer, m_hdrBufferBytes);
	return dataSize;
}


//
// Subencoding implementations.
//

int
vncEncodeTight::SendSolidRect(BYTE *dest, int w, int h)
{
	int len;

	if (m_usePixelFormat24) {
		Pack24(m_buffer, 1);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFill << 4;
	memcpy (dest, m_buffer, len);

	return len;
}

int
vncEncodeTight::SendMonoRect(BYTE *dest, int w, int h)
{
	const int streamId = 1;
	int paletteLen, dataLen;
	CARD8 paletteBuf[8];

	// Prepare tight encoding header.
	dataLen = (w + 7) / 8;
	dataLen *= h;

	m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = 1;

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		EncodeMonoRect32((CARD8 *)m_buffer, w, h);

		((CARD32 *)paletteBuf)[0] = m_monoBackground;
		((CARD32 *)paletteBuf)[1] = m_monoForeground;

		if (m_usePixelFormat24) {
			Pack24(paletteBuf, 2);
			paletteLen = 6;
		} else
			paletteLen = 8;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf, paletteLen);
		m_hdrBufferBytes += paletteLen;
		break;

	case 16:
		EncodeMonoRect16((CARD8 *)m_buffer, w, h);

		((CARD16 *)paletteBuf)[0] = (CARD16)m_monoBackground;
		((CARD16 *)paletteBuf)[1] = (CARD16)m_monoForeground;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf, 4);
		m_hdrBufferBytes += 4;
		break;

	default:
		EncodeMonoRect8((CARD8 *)m_buffer, w, h);

		m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)m_monoBackground;
		m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)m_monoForeground;
	}

	return CompressData(dest, streamId, dataLen,
						m_conf[m_compresslevel].monoZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::SendIndexedRect(BYTE *dest, int w, int h)
{
	const int streamId = 2;
	int i, entryLen;
	CARD8 paletteBuf[256*4];

	// Prepare tight encoding header.
	m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)(m_paletteNumColors - 1);

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		EncodeIndexedRect32((CARD8 *)m_buffer, w * h);

		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD32 *)paletteBuf)[i] =
				m_palette.entry[i].listNode->rgb;
		}
		if (m_usePixelFormat24) {
			Pack24(paletteBuf, m_paletteNumColors);
			entryLen = 3;
		} else
			entryLen = 4;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf,
			   m_paletteNumColors * entryLen);
		m_hdrBufferBytes += m_paletteNumColors * entryLen;
		break;

	case 16:
		EncodeIndexedRect16((CARD8 *)m_buffer, w * h);

		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD16 *)paletteBuf)[i] =
				(CARD16)m_palette.entry[i].listNode->rgb;
		}

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf,
			   m_paletteNumColors * 2);
		m_hdrBufferBytes += m_paletteNumColors * 2;
		break;

	default:
		return -1;				// Should never happen.
	}

	return CompressData(dest, streamId, w * h,
						m_conf[m_compresslevel].idxZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::SendFullColorRect(BYTE *dest, int w, int h)
{
	const int streamId = 0;
	int len;

	m_hdrBuffer[m_hdrBufferBytes++] = 0x00;

	if (m_usePixelFormat24) {
		Pack24(m_buffer, w * h);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	return CompressData(dest, streamId, w * h * len,
						m_conf[m_compresslevel].rawZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::SendGradientRect(BYTE *dest, int w, int h)
{
	const int streamId = 3;
	int len;

	if (m_remoteformat.bitsPerPixel == 8)
		return SendFullColorRect(dest, w, h);

	if (m_prevRowBuf == NULL)
		m_prevRowBuf = new int [2048*3];

	m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterGradient;

	if (m_usePixelFormat24) {
		FilterGradient24(m_buffer, w, h);
		len = 3;
	} else if (m_remoteformat.bitsPerPixel == 32) {
		FilterGradient32((CARD32 *)m_buffer, w, h);
		len = 4;
	} else {
		FilterGradient16((CARD16 *)m_buffer, w, h);
		len = 2;
	}

	return CompressData(dest, streamId, w * h * len,
						m_conf[m_compresslevel].gradientZlibLevel,
						Z_FILTERED);
}

int
vncEncodeTight::CompressData(BYTE *dest, int streamId, int dataLen,
							 int zlibLevel, int zlibStrategy)
{
	if (dataLen < TIGHT_MIN_TO_COMPRESS) {
		memcpy(dest, m_buffer, dataLen);
		return dataLen;
	}

	z_streamp pz = &m_zsStruct[streamId];

	// Initialize compression stream if needed.
	if (!m_zsActive[streamId]) {
		pz->zalloc = Z_NULL;
		pz->zfree = Z_NULL;
		pz->opaque = Z_NULL;

		log.Print(LL_INTINFO,
				  VNCLOG("calling deflateInit2 with zlib level:%d\n"),
				  zlibLevel);
		int err = deflateInit2 (pz, zlibLevel, Z_DEFLATED, MAX_WBITS,
								MAX_MEM_LEVEL, zlibStrategy);
		if (err != Z_OK) {
			log.Print(LL_INTINFO,
					  VNCLOG("deflateInit2 returned error:%d:%s\n"),
					  err, pz->msg);
			return -1;
		}

		m_zsActive[streamId] = true;
		m_zsLevel[streamId] = zlibLevel;
	}

	int outBufferSize = dataLen + dataLen / 100 + 16;

	// Prepare buffer pointers.
	pz->next_in = (Bytef *)m_buffer;
	pz->avail_in = dataLen;
	pz->next_out = (Bytef *)dest;
	pz->avail_out = outBufferSize;

	// Change compression parameters if needed.
	if (zlibLevel != m_zsLevel[streamId]) {
		log.Print(LL_INTINFO,
				  VNCLOG("calling deflateParams with zlib level:%d\n"),
				  zlibLevel);
		int err = deflateParams (pz, zlibLevel, zlibStrategy);
		if (err != Z_OK) {
			log.Print(LL_INTINFO,
					  VNCLOG("deflateParams returned error:%d:%s\n"),
					  err, pz->msg);
			return -1;
		}
		m_zsLevel[streamId] = zlibLevel;
	}

	// Actual compression.
	if ( deflate (pz, Z_SYNC_FLUSH) != Z_OK ||
		 pz->avail_in != 0 || pz->avail_out == 0 ) {
		log.Print(LL_INTINFO, VNCLOG("deflate() call failed.\n"));
		return -1;
	}

	int compressedLen = outBufferSize - pz->avail_out;

	// Prepare compressed data size for sending.
	m_hdrBuffer[m_hdrBufferBytes++] = compressedLen & 0x7F;
	if (compressedLen > 0x7F) {
		m_hdrBuffer[m_hdrBufferBytes-1] |= 0x80;
		m_hdrBuffer[m_hdrBufferBytes++] = compressedLen >> 7 & 0x7F;
		if (compressedLen > 0x3FFF) {
			m_hdrBuffer[m_hdrBufferBytes-1] |= 0x80;
			m_hdrBuffer[m_hdrBufferBytes++] = compressedLen >> 14 & 0xFF;
		}
	}

	return compressedLen;
}


void
vncEncodeTight::FillPalette8(int count)
{
	CARD8 *data = (CARD8 *)m_buffer;
	CARD8 c0, c1;
	int i, n0, n1;

	m_paletteNumColors = 0;

	c0 = data[0];
	for (i = 1; i < count && data[i] == c0; i++);
	if (i == count) {
		m_paletteNumColors = 1;
		return;                 // Solid rectangle
	}

	if (m_paletteMaxColors < 2)
		return;

	n0 = i;
	c1 = data[i];
	n1 = 0;
	for (i++; i < count; i++) {
		if (data[i] == c0) {
			n0++;
		} else if (data[i] == c1) {
			n1++;
		} else
			break;
	}
	if (i == count) {
		if (n0 > n1) {
			m_monoBackground = (CARD32)c0;
			m_monoForeground = (CARD32)c1;
		} else {
			m_monoBackground = (CARD32)c1;
			m_monoForeground = (CARD32)c0;
		}
		m_paletteNumColors = 2;   // Two colors
	}
}

#define DEFINE_FILL_PALETTE_FUNCTION(bpp)									  \
																			  \
void																		  \
vncEncodeTight::FillPalette##bpp(int count)									  \
{																			  \
	CARD##bpp *data = (CARD##bpp *)m_buffer;								  \
	CARD##bpp c0, c1, ci;													  \
	int i, n0, n1, ni;														  \
																			  \
	PaletteReset();															  \
																			  \
	c0 = data[0];															  \
	for (i = 1; i < count && data[i] == c0; i++);							  \
	if (i == count) {														  \
		m_paletteNumColors = 1;	/* Solid rectangle */						  \
		return;																  \
	}																		  \
																			  \
	if (m_paletteMaxColors < 2)												  \
		return;																  \
																			  \
	n0 = i;																	  \
	c1 = data[i];															  \
	n1 = 0;																	  \
	for (i++; i < count; i++) {												  \
		ci = data[i];														  \
		if (ci == c0) {														  \
			n0++;															  \
		} else if (ci == c1) {												  \
			n1++;															  \
		} else																  \
			break;															  \
	}																		  \
	if (i == count) {														  \
		if (n0 > n1) {														  \
			m_monoBackground = (CARD32)c0;									  \
			m_monoForeground = (CARD32)c1;									  \
		} else {															  \
			m_monoBackground = (CARD32)c1;									  \
			m_monoForeground = (CARD32)c0;									  \
		}																	  \
		m_paletteNumColors = 2;	/* Two colors */							  \
		return;																  \
	}																		  \
																			  \
	PaletteInsert (c0, (CARD32)n0, bpp);									  \
	PaletteInsert (c1, (CARD32)n1, bpp);									  \
																			  \
	ni = 1;																	  \
	for (i++; i < count; i++) {												  \
		if (data[i] == ci) {												  \
			ni++;															  \
		} else {															  \
			if (!PaletteInsert (ci, (CARD32)ni, bpp))						  \
				return;														  \
			ci = data[i];													  \
			ni = 1;															  \
		}																	  \
	}																		  \
	PaletteInsert (ci, (CARD32)ni, bpp);									  \
}

DEFINE_FILL_PALETTE_FUNCTION(16)
DEFINE_FILL_PALETTE_FUNCTION(32)


//
// Functions to operate with palette structures.
//

#define HASH_FUNC16(rgb) ((int)((rgb >> 8) + rgb & 0xFF))
#define HASH_FUNC32(rgb) ((int)((rgb >> 16) + (rgb >> 8) & 0xFF))

void
vncEncodeTight::PaletteReset(void)
{
	m_paletteNumColors = 0;
	memset(m_palette.hash, 0, 256 * sizeof(COLOR_LIST *));
}

int
vncEncodeTight::PaletteInsert(CARD32 rgb, int numPixels, int bpp)
{
	COLOR_LIST *pnode;
	COLOR_LIST *prev_pnode = NULL;
	int hash_key, idx, new_idx, count;

    hash_key = (bpp == 16) ? HASH_FUNC16(rgb) : HASH_FUNC32(rgb);

	pnode = m_palette.hash[hash_key];

	while (pnode != NULL) {
		if (pnode->rgb == rgb) {
			// Such palette entry already exists.
			new_idx = idx = pnode->idx;
			count = m_palette.entry[idx].numPixels + numPixels;
			if (new_idx && m_palette.entry[new_idx-1].numPixels < count) {
				do {
					m_palette.entry[new_idx] = m_palette.entry[new_idx-1];
					m_palette.entry[new_idx].listNode->idx = new_idx;
					new_idx--;
				}
				while (new_idx &&
					   m_palette.entry[new_idx-1].numPixels < count);
				m_palette.entry[new_idx].listNode = pnode;
				pnode->idx = new_idx;
			}
			m_palette.entry[new_idx].numPixels = count;
			return m_paletteNumColors;
		}
		prev_pnode = pnode;
		pnode = pnode->next;
	}

	// Check if palette is full.
	if ( m_paletteNumColors == 256 ||
		 m_paletteNumColors == m_paletteMaxColors ) {
		m_paletteNumColors = 0;
		return 0;
	}

	// Move palette entries with lesser pixel counts.
	for ( idx = m_paletteNumColors;
		  idx > 0 && m_palette.entry[idx-1].numPixels < numPixels;
		  idx-- ) {
		m_palette.entry[idx] = m_palette.entry[idx-1];
		m_palette.entry[idx].listNode->idx = idx;
	}

	// Add new palette entry into the freed slot.
	pnode = &m_palette.list[m_paletteNumColors];
	if (prev_pnode != NULL) {
		prev_pnode->next = pnode;
	} else {
		m_palette.hash[hash_key] = pnode;
	}
	pnode->next = NULL;
	pnode->idx = idx;
	pnode->rgb = rgb;
	m_palette.entry[idx].listNode = pnode;
	m_palette.entry[idx].numPixels = numPixels;

	return (++m_paletteNumColors);
}


//
// Converting 32-bit color samples into 24-bit colors.
// Should be called only when redMax, greenMax and blueMax are 256.
// 8-bit samples assumed to be byte-aligned.
//

void
vncEncodeTight::Pack24(BYTE *buf, int count)
{
	int i;
	CARD32 pix;
	int r_shift, g_shift, b_shift;

	if (!m_localformat.bigEndian == !m_remoteformat.bigEndian) {
		r_shift = m_remoteformat.redShift;
		g_shift = m_remoteformat.greenShift;
		b_shift = m_remoteformat.blueShift;
	} else {
		r_shift = 24 - m_remoteformat.redShift;
		g_shift = 24 - m_remoteformat.greenShift;
		b_shift = 24 - m_remoteformat.blueShift;
	}

	for (i = 0; i < count; i++) {
		pix = ((CARD32 *)buf)[i];
		buf[i*3]   = (BYTE)(pix >> r_shift);
		buf[i*3+1] = (BYTE)(pix >> g_shift);
		buf[i*3+2] = (BYTE)(pix >> b_shift);
	}
}


//
// Converting truecolor samples into palette indices.
//

#define DEFINE_IDX_ENCODE_FUNCTION(bpp)										  \
																			  \
void																		  \
vncEncodeTight::EncodeIndexedRect##bpp(BYTE *buf, int count)				  \
{																			  \
	COLOR_LIST *pnode;														  \
	CARD##bpp *src;															  \
	CARD##bpp rgb;															  \
	int rep = 0;															  \
																			  \
	src = (CARD##bpp *) buf;												  \
																			  \
	while (count--) {														  \
		rgb = *src++;														  \
		while (count && *src == rgb) {										  \
			rep++, src++, count--;											  \
		}																	  \
		pnode = m_palette.hash[HASH_FUNC##bpp(rgb)];						  \
		while (pnode != NULL) {												  \
			if ((CARD##bpp)pnode->rgb == rgb) {								  \
				*buf++ = (CARD8)pnode->idx;									  \
				while (rep) {												  \
					*buf++ = (CARD8)pnode->idx;								  \
					rep--;													  \
				}															  \
				break;														  \
			}																  \
			pnode = pnode->next;											  \
		}																	  \
	}																		  \
}

DEFINE_IDX_ENCODE_FUNCTION(16)
DEFINE_IDX_ENCODE_FUNCTION(32)

#define DEFINE_MONO_ENCODE_FUNCTION(bpp)									  \
																			  \
void																		  \
vncEncodeTight::EncodeMonoRect##bpp(BYTE *buf, int w, int h)				  \
{																			  \
	CARD##bpp *ptr;															  \
	CARD##bpp bg;															  \
	unsigned int value, mask;												  \
	int aligned_width;														  \
	int x, y, bg_bits;														  \
																			  \
	ptr = (CARD##bpp *) buf;												  \
	bg = (CARD##bpp) m_monoBackground;										  \
	aligned_width = w - w % 8;												  \
																			  \
	for (y = 0; y < h; y++) {												  \
		for (x = 0; x < aligned_width; x += 8) {							  \
			for (bg_bits = 0; bg_bits < 8; bg_bits++) {						  \
				if (*ptr++ != bg)											  \
					break;													  \
			}																  \
			if (bg_bits == 8) {												  \
				*buf++ = 0;													  \
				continue;													  \
			}																  \
			mask = 0x80 >> bg_bits;											  \
			value = mask;													  \
			for (bg_bits++; bg_bits < 8; bg_bits++) {						  \
				mask >>= 1;													  \
				if (*ptr++ != bg) {											  \
					value |= mask;											  \
				}															  \
			}																  \
			*buf++ = (CARD8)value;											  \
		}																	  \
																			  \
		mask = 0x80;														  \
		value = 0;															  \
		if (x >= w)															  \
			continue;														  \
																			  \
		for (; x < w; x++) {												  \
			if (*ptr++ != bg) {												  \
				value |= mask;												  \
			}																  \
			mask >>= 1;														  \
		}																	  \
		*buf++ = (CARD8)value;												  \
	}																		  \
}

DEFINE_MONO_ENCODE_FUNCTION(8)
DEFINE_MONO_ENCODE_FUNCTION(16)
DEFINE_MONO_ENCODE_FUNCTION(32)


//
// ``Gradient'' filter for 24-bit color samples.
// Should be called only when redMax, greenMax and blueMax are 256.
// 8-bit samples assumed to be byte-aligned.
//

void
vncEncodeTight::FilterGradient24(BYTE *buf, int w, int h)
{
	CARD32 *buf32;
	CARD32 pix32;
	int *prevRowPtr;
	int shiftBits[3];
	int pixHere[3], pixUpper[3], pixLeft[3], pixUpperLeft[3];
	int prediction;
	int x, y, c;

	buf32 = (CARD32 *)buf;
	memset (m_prevRowBuf, 0, w * 3 * sizeof(int));

	if (!m_localformat.bigEndian == !m_remoteformat.bigEndian) {
		shiftBits[0] = m_remoteformat.redShift;
		shiftBits[1] = m_remoteformat.greenShift;
		shiftBits[2] = m_remoteformat.blueShift;
	} else {
		shiftBits[0] = 24 - m_remoteformat.redShift;
		shiftBits[1] = 24 - m_remoteformat.greenShift;
		shiftBits[2] = 24 - m_remoteformat.blueShift;
	}

	for (y = 0; y < h; y++) {
		for (c = 0; c < 3; c++) {
			pixUpper[c] = 0;
			pixHere[c] = 0;
		}
		prevRowPtr = m_prevRowBuf;
		for (x = 0; x < w; x++) {
			pix32 = *buf32++;
			for (c = 0; c < 3; c++) {
				pixUpperLeft[c] = pixUpper[c];
				pixLeft[c] = pixHere[c];
				pixUpper[c] = *prevRowPtr;
				pixHere[c] = (int)(pix32 >> shiftBits[c] & 0xFF);
				*prevRowPtr++ = pixHere[c];

				prediction = pixLeft[c] + pixUpper[c] - pixUpperLeft[c];
				if (prediction < 0) {
					prediction = 0;
				} else if (prediction > 0xFF) {
					prediction = 0xFF;
				}
				*buf++ = (BYTE)(pixHere[c] - prediction);
			}
		}
	}
}


//
// ``Gradient'' filter for other color depths.
//

#define DEFINE_GRADIENT_FILTER_FUNCTION(bpp)								  \
																			  \
void																		  \
vncEncodeTight::FilterGradient##bpp(CARD##bpp *buf, int w, int h)			  \
{																			  \
	CARD##bpp pix, diff;													  \
	bool endianMismatch;													  \
	int *prevRowPtr;														  \
	int maxColor[3], shiftBits[3];											  \
	int pixHere[3], pixUpper[3], pixLeft[3], pixUpperLeft[3];				  \
	int prediction;															  \
	int x, y, c;															  \
																			  \
	memset (m_prevRowBuf, 0, w * 3 * sizeof(int));							  \
																			  \
	endianMismatch = (!m_localformat.bigEndian != !m_remoteformat.bigEndian); \
																			  \
	maxColor[0] = m_remoteformat.redMax;									  \
	maxColor[1] = m_remoteformat.greenMax;									  \
	maxColor[2] = m_remoteformat.blueMax;									  \
	shiftBits[0] = m_remoteformat.redShift;									  \
	shiftBits[1] = m_remoteformat.greenShift;								  \
	shiftBits[2] = m_remoteformat.blueShift;								  \
																			  \
	for (y = 0; y < h; y++) {												  \
		for (c = 0; c < 3; c++) {											  \
			pixUpper[c] = 0;												  \
			pixHere[c] = 0;													  \
		}																	  \
		prevRowPtr = m_prevRowBuf;											  \
		for (x = 0; x < w; x++) {											  \
			pix = *buf;														  \
			if (endianMismatch) {											  \
				pix = Swap##bpp(pix);										  \
			}																  \
			diff = 0;														  \
			for (c = 0; c < 3; c++) {										  \
				pixUpperLeft[c] = pixUpper[c];								  \
				pixLeft[c] = pixHere[c];									  \
				pixUpper[c] = *prevRowPtr;									  \
				pixHere[c] = (int)(pix >> shiftBits[c] & maxColor[c]);		  \
				*prevRowPtr++ = pixHere[c];									  \
																			  \
				prediction = pixLeft[c] + pixUpper[c] - pixUpperLeft[c];	  \
				if (prediction < 0) {										  \
					prediction = 0;											  \
				} else if (prediction > maxColor[c]) {						  \
					prediction = maxColor[c];								  \
				}															  \
				diff |= ((pixHere[c] - prediction) & maxColor[c])			  \
					<< shiftBits[c];										  \
			}																  \
			if (endianMismatch) {											  \
				diff = Swap##bpp(diff);										  \
			}																  \
			*buf++ = diff;													  \
		}																	  \
	}																		  \
}

DEFINE_GRADIENT_FILTER_FUNCTION(16)
DEFINE_GRADIENT_FILTER_FUNCTION(32)


//
// Code to guess if given rectangle is suitable for still image
// compression.
//

#define DETECT_SUBROW_WIDTH   7
#define DETECT_MIN_WIDTH      8
#define DETECT_MIN_HEIGHT     8

int
vncEncodeTight::DetectStillImage (int w, int h)
{
	if ( m_remoteformat.bitsPerPixel == 8 ||
		 w * h < m_conf[m_compresslevel].gradientMinRectSize ||
		 w < DETECT_MIN_WIDTH || h < DETECT_MIN_HEIGHT ) {
		return 0;
	}

	if (m_remoteformat.bitsPerPixel == 32) {
		if (m_usePixelFormat24) {
			return DetectStillImage24(w, h);
		} else {
			return DetectStillImage32(w, h);
		}
	} else {
		return DetectStillImage16(w, h);
	}
}

int
vncEncodeTight::DetectStillImage24 (int w, int h)
{
	int diffStat[256];
	int pixelCount = 0;
	int pix, left[3];
	unsigned long avgError;

	// If client is big-endian, color samples begin from the second
	// byte (offset 1) of a 32-bit pixel value.
	int off = (m_remoteformat.bigEndian != 0);

	memset(diffStat, 0, 256*sizeof(int));

	int y = 0, x = 0;
	int d, dx, c;
	while (y < h && x < w) {
		for (d = 0; d < h - y && d < w - x - DETECT_SUBROW_WIDTH; d++) {
			for (c = 0; c < 3; c++) {
				left[c] = (int)m_buffer[((y+d)*w+x+d)*4+off+c] & 0xFF;
			}
			for (dx = 1; dx <= DETECT_SUBROW_WIDTH; dx++) {
				for (c = 0; c < 3; c++) {
					pix = (int)m_buffer[((y+d)*w+x+d+dx)*4+off+c] & 0xFF;
					diffStat[abs(pix - left[c])]++;
					left[c] = pix;
				}
				pixelCount++;
			}
		}
		if (w > h) {
			x += h;
			y = 0;
		} else {
			x = 0;
			y += w;
		}
	}

	if (diffStat[0] * 33 / pixelCount >= 95)
		return 0;

	avgError = 0;
	for (c = 1; c < 8; c++) {
		avgError += (unsigned long)diffStat[c] * (unsigned long)(c * c);
		if (diffStat[c] == 0 || diffStat[c] > diffStat[c-1] * 2)
			return 0;
	}
	for (; c < 256; c++) {
		avgError += (unsigned long)diffStat[c] * (unsigned long)(c * c);
	}
	avgError /= (pixelCount * 3 - diffStat[0]);

	return (avgError < m_conf[m_compresslevel].gradientThreshold24);
}

#define DEFINE_DETECT_FUNCTION(bpp)											  \
																			  \
int																			  \
vncEncodeTight::DetectStillImage##bpp (int w, int h)						  \
{																			  \
	bool endianMismatch;													  \
	CARD##bpp pix;															  \
	int maxColor[3], shiftBits[3];											  \
	int x, y, d, dx, c;														  \
	int diffStat[256];														  \
	int pixelCount = 0;														  \
	int sample, sum, left[3];												  \
	unsigned long avgError;													  \
																			  \
	endianMismatch = (!m_localformat.bigEndian != !m_remoteformat.bigEndian); \
																			  \
	maxColor[0] = m_remoteformat.redMax;									  \
	maxColor[1] = m_remoteformat.greenMax;									  \
	maxColor[2] = m_remoteformat.blueMax;									  \
	shiftBits[0] = m_remoteformat.redShift;									  \
	shiftBits[1] = m_remoteformat.greenShift;								  \
	shiftBits[2] = m_remoteformat.blueShift;								  \
																			  \
	memset(diffStat, 0, 256*sizeof(int));									  \
																			  \
	y = 0, x = 0;															  \
	while (y < h && x < w) {												  \
		for (d = 0; d < h - y && d < w - x - DETECT_SUBROW_WIDTH; d++) {	  \
			pix = ((CARD##bpp *)m_buffer)[(y+d)*w+x+d];						  \
			if (endianMismatch) {											  \
				pix = Swap##bpp(pix);										  \
			}																  \
			for (c = 0; c < 3; c++) {										  \
				left[c] = (int)(pix >> shiftBits[c] & maxColor[c]);			  \
			}																  \
			for (dx = 1; dx <= DETECT_SUBROW_WIDTH; dx++) {					  \
				pix = ((CARD##bpp *)m_buffer)[(y+d)*w+x+d+dx];				  \
				if (endianMismatch) {										  \
					pix = Swap##bpp(pix);									  \
				}															  \
				sum = 0;													  \
				for (c = 0; c < 3; c++) {									  \
					sample = (int)(pix >> shiftBits[c] & maxColor[c]);		  \
					sum += abs(sample - left[c]);							  \
					left[c] = sample;										  \
				}															  \
				if (sum > 255)												  \
					sum = 255;												  \
				diffStat[sum]++;											  \
				pixelCount++;												  \
			}																  \
		}																	  \
		if (w > h) {														  \
			x += h;															  \
			y = 0;															  \
		} else {															  \
			x = 0;															  \
			y += w;															  \
		}																	  \
	}																		  \
																			  \
	if ((diffStat[0] + diffStat[1]) * 100 / pixelCount >= 90)				  \
		return 0;															  \
																			  \
	avgError = 0;															  \
	for (c = 1; c < 8; c++) {												  \
		avgError += (unsigned long)diffStat[c] * (unsigned long)(c * c);	  \
		if (diffStat[c] == 0 || diffStat[c] > diffStat[c-1] * 2)			  \
			return 0;														  \
	}																		  \
	for (; c < 256; c++) {													  \
		avgError += (unsigned long)diffStat[c] * (unsigned long)(c * c);	  \
	}																		  \
	avgError /= (pixelCount - diffStat[0]);									  \
																			  \
	return (avgError < m_conf[m_compresslevel].gradientThreshold);			  \
}

DEFINE_DETECT_FUNCTION(16)
DEFINE_DETECT_FUNCTION(32)

