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

vncEncodeTight::vncEncodeTight()
{
	m_buffer = NULL;
	m_bufflen = 0;

	m_hdrBuffer = new BYTE [sz_rfbFramebufferUpdateRectHeader + 8 + 256*4];
	m_prevRowBuf = NULL;

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
	int result = TIGHT_MAX_RECT_SIZE * (m_remoteformat.bitsPerPixel / 8);
	result += result / 100 + 16;

	return result;
}

UINT
vncEncodeTight::NumCodedRects(RECT &rect)
{
	const int w = rect.right - rect.left;
	const int h = rect.bottom - rect.top;

	if (w > 2048 || w * h > TIGHT_MAX_RECT_SIZE) {
		int subrectMaxWidth = (w > 2048) ? 2048 : w;
		int subrectMaxHeight = TIGHT_MAX_RECT_SIZE / subrectMaxWidth;
		return (((w - 1) / 2048 + 1) * ((h - 1) / subrectMaxHeight + 1));
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

	int rawDataSize = TIGHT_MAX_RECT_SIZE * (m_remoteformat.bitsPerPixel / 8);

	if (m_bufflen < rawDataSize)
	{
		if (m_buffer != NULL)
		{
			delete [] m_buffer;
			m_buffer = NULL;
		}
		m_buffer = new BYTE [rawDataSize+1];
		if (m_buffer == NULL)
			return vncEncoder::EncodeRect(source, dest, rect);
		m_bufflen = rawDataSize;
	}

	int totalSize = 0;
	int partialSize = 0;

	if (w > 2048 || w * h > TIGHT_MAX_RECT_SIZE) {
		int dx, dy, rw, rh;
		int subrectMaxWidth = (w > 2048) ? 2048 : w;
		int subrectMaxHeight = TIGHT_MAX_RECT_SIZE / subrectMaxWidth;

		for (dy = 0; dy < h; dy += subrectMaxHeight) {
			for (dx = 0; dx < w; dx += 2048) {
				rw = (dx + 2048 < w) ? 2048 : w - dx;
				rh = (dy + subrectMaxHeight < h) ? subrectMaxHeight : h - dy;

				partialSize = EncodeSubrect(source, outConn, dest,
											x+dx, y+dy, rw, rh);
				totalSize += partialSize;
				if (dy + subrectMaxHeight < h || dx + 2048 < w) {
					// Send the encoded data
					outConn->SendExact( (char *)dest, partialSize );
				}
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

	m_paletteMaxColors = w * h / 128;
	switch (m_remoteformat.bitsPerPixel) {
	case 8:
		FillPalette8(w, h);
		break;
	case 16:
		FillPalette16(w, h);
		break;
	default:
		FillPalette32(w, h);
	}

	int dataSize;
	switch (m_paletteNumColors) {
	case 1:
		// Solid rectangle
		dataSize = SendSolidRect(dest, w, h);
		break;
	case 0:
		// Truecolor image
		if (DetectStillImage(w, h)) {
			dataSize = SendGradientRect(dest, w, h);
		} else {
			dataSize = SendFullColorRect(dest, w, h);
		}
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
vncEncodeTight::SendIndexedRect(BYTE *dest, int w, int h)
{
	int i, streamId, entryLen, dataLen;
	CARD8 paletteBuf[256*4];

	// Prepare tight encoding header.
	if (m_paletteNumColors == 2) {
		streamId = 1;
		dataLen = (w + 7) / 8;
		dataLen *= h;
	} else {
		streamId = 2;
		dataLen = w * h;
	}
	m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)(m_paletteNumColors - 1);

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
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

		EncodeIndexedRect32((CARD8 *)m_buffer, w, h);
		break;

	case 16:
		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD16 *)paletteBuf)[i] =
				(CARD16)m_palette.entry[i].listNode->rgb;
		}

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf,
			   m_paletteNumColors * 2);
		m_hdrBufferBytes += m_paletteNumColors * 2;

		EncodeIndexedRect16((CARD8 *)m_buffer, w, h);
		break;

	default:
		memcpy (&m_hdrBuffer[m_hdrBufferBytes], m_palette8.pixelValue,
				m_paletteNumColors);
		m_hdrBufferBytes += m_paletteNumColors;

		EncodeIndexedRect8((CARD8 *)m_buffer, w, h);
	}

	return CompressData(dest, streamId, dataLen);
}

int
vncEncodeTight::SendFullColorRect(BYTE *dest, int w, int h)
{
	int len;

	m_hdrBuffer[m_hdrBufferBytes++] = 0x00;

	if (m_usePixelFormat24) {
		Pack24(m_buffer, w * h);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	return CompressData(dest, 0, w * h * len);
}

int
vncEncodeTight::SendGradientRect(BYTE *dest, int w, int h)
{
	int len;

	if (m_remoteformat.bitsPerPixel == 8)
		return SendFullColorRect(dest, w, h);

	if (m_prevRowBuf == NULL)
		m_prevRowBuf = new int [2048*3];

	m_hdrBuffer[m_hdrBufferBytes++] = (3 | rfbTightExplicitFilter) << 4;
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

	return CompressData(dest, 3, w * h * len);
}

int
vncEncodeTight::CompressData(BYTE *dest, int streamId, int dataLen)
{
	if (dataLen < TIGHT_MIN_TO_COMPRESS) {
		memcpy(dest, m_buffer, dataLen);
		return dataLen;
	}

	z_streamp pz = &m_zsStruct[streamId];

	// Initialize compression stream.
	if (!m_zsActive[streamId]) {
		pz->zalloc = Z_NULL;
		pz->zfree = Z_NULL;
		pz->opaque = Z_NULL;

		int err;
		if (streamId == 3) {
			err = deflateInit2 (pz, 6, Z_DEFLATED, MAX_WBITS,
								MAX_MEM_LEVEL, Z_FILTERED);
		} else {
			err = deflateInit2 (pz, 9, Z_DEFLATED, MAX_WBITS,
								MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
		}
		if (err != Z_OK)
			return -1;

		m_zsActive[streamId] = true;
	}

	int outBufferSize = dataLen + dataLen / 100 + 16;

	// Actual compression.
	pz->next_in = (Bytef *)m_buffer;
	pz->avail_in = dataLen;
	pz->next_out = (Bytef *)dest;
	pz->avail_out = outBufferSize;

	if ( deflate (pz, Z_SYNC_FLUSH) != Z_OK ||
		 pz->avail_in != 0 || pz->avail_out == 0 ) {
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
vncEncodeTight::FillPalette8(int w, int h)
{
	CARD8 *data = (CARD8 *)m_buffer;
	CARD8 c0, c1;
	int i, n0, n1;

	m_paletteNumColors = 0;

	c0 = data[0];
	for (i = 1; i < w * h && data[i] == c0; i++);
	if (i == w * h) {
		m_paletteNumColors = 1;
		return;                 // Solid rectangle
	}

	if (m_paletteMaxColors < 2)
		return;

	n0 = i;
	c1 = data[i];
	n1 = 0;
	for (i++; i < w * h; i++) {
		if (data[i] == c0) {
			n0++;
		} else if (data[i] == c1) {
			n1++;
		} else
			break;
	}
	if (i == w * h) {
		if (n1 > n0) {
			m_palette8.pixelValue[0] = c0;
			m_palette8.pixelValue[1] = c1;
			m_palette8.colorIdx[c0] = 0;
			m_palette8.colorIdx[c1] = 1;
		} else {
			m_palette8.pixelValue[0] = c1;
			m_palette8.pixelValue[1] = c0;
			m_palette8.colorIdx[c0] = 1;
			m_palette8.colorIdx[c1] = 0;
		}
		m_paletteNumColors = 2;   // Two colors
	}
}

#define DEFINE_FILL_PALETTE_FUNCTION(bpp)									  \
																			  \
void																		  \
vncEncodeTight::FillPalette##bpp(int w, int h)								  \
{																			  \
	CARD##bpp *data = (CARD##bpp *)m_buffer;								  \
	CARD##bpp c0, c1, ci;													  \
	int i, n0, n1, ni;														  \
																			  \
	PaletteReset();															  \
																			  \
	c0 = data[0];															  \
	for (i = 1; i < w * h && data[i] == c0; i++);							  \
	if (i == w * h) {														  \
		m_paletteNumColors = 1;												  \
		return;                 /* Solid rectangle */						  \
	}																		  \
																			  \
	if (m_paletteMaxColors < 2)												  \
		return;																  \
																			  \
	n0 = i;																	  \
	c1 = data[i];															  \
	n1 = 0;																	  \
	for (i++; i < w * h; i++) {												  \
		ci = data[i];														  \
		if (ci == c0) {														  \
			n0++;															  \
		} else if (ci == c1) {												  \
			n1++;															  \
		} else																  \
			break;															  \
	}																		  \
	PaletteInsert (c0, (CARD32)n0);											  \
	PaletteInsert (c1, (CARD32)n1);											  \
	if (i == w * h)															  \
		return;                 /* Two colors */							  \
																			  \
	ni = 1;																	  \
	for (i++; i < w * h; i++) {												  \
		if (data[i] == ci) {												  \
			ni++;															  \
		} else {															  \
			if (!PaletteInsert (ci, (CARD32)ni))							  \
				return;														  \
			ci = data[i];													  \
			ni = 1;															  \
		}																	  \
	}																		  \
	PaletteInsert (ci, (CARD32)ni);											  \
}

DEFINE_FILL_PALETTE_FUNCTION(16)
DEFINE_FILL_PALETTE_FUNCTION(32)


//
// Functions to operate with palette structures.
//

void
vncEncodeTight::PaletteReset(void)
{
	int i;

	m_paletteNumColors = 0;
	for (i = 0; i < 256; i++)
		m_palette.hash[i] = NULL;
}

int
vncEncodeTight::PaletteFind(CARD32 rgb)
{
	COLOR_LIST *pnode;

	if (rgb & 0xFF000000) {
		pnode = m_palette.hash[(int)((rgb >> 24) + (rgb >> 16) & 0xFF)];
	} else {
		pnode = m_palette.hash[(int)((rgb >> 8) + rgb & 0xFF)];
	}

	while (pnode != NULL) {
		if (pnode->rgb == rgb)
			return pnode->idx;
		pnode = pnode->next;
	}
	return -1;
}

int
vncEncodeTight::PaletteInsert(CARD32 rgb, int numPixels)
{
	COLOR_LIST *pnode;
	COLOR_LIST *prev_pnode = NULL;
	int hash_key, idx, new_idx, count;

	if (rgb & 0xFF000000) {
		hash_key = (int)((rgb >> 24) + (rgb >> 16) & 0xFF);
	} else {
		hash_key = (int)((rgb >> 8) + rgb & 0xFF);
	}
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

#define PaletteFind8(c)  m_palette8.colorIdx[(c)]
#define PaletteFind16    PaletteFind
#define PaletteFind32    PaletteFind

#define DEFINE_IDX_ENCODE_FUNCTION(bpp)										  \
																			  \
void																		  \
vncEncodeTight::EncodeIndexedRect##bpp(BYTE *buf, int w, int h)				  \
{																			  \
	int x, y, i, width_bytes;												  \
																			  \
	if (m_paletteNumColors != 2) {											  \
		for (i = 0; i < w * h; i++)											  \
			buf[i] = (BYTE)PaletteFind(((CARD##bpp *)buf)[i]);				  \
		return;																  \
	}																		  \
																			  \
	width_bytes = (w + 7) / 8;												  \
	for (y = 0; y < h; y++) {												  \
		for (x = 0; x < w / 8; x++) {										  \
			for (i = 0; i < 8; i++)											  \
				buf[y*width_bytes+x] = (buf[y*width_bytes+x] << 1) |		  \
					(PaletteFind##bpp (((CARD##bpp *)buf)[y*w+x*8+i]) & 1);	  \
		}																	  \
		buf[y*width_bytes+x] = 0;											  \
		for (i = 0; i < w % 8; i++) {										  \
			buf[y*width_bytes+x] |=											  \
				(PaletteFind##bpp (((CARD##bpp *)buf)[y*w+x*8+i]) & 1) <<	  \
					(7 - i);												  \
		}																	  \
	}																		  \
}

DEFINE_IDX_ENCODE_FUNCTION(8)
DEFINE_IDX_ENCODE_FUNCTION(16)
DEFINE_IDX_ENCODE_FUNCTION(32)


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
#define DETECT_MIN_SIZE    8192

int
vncEncodeTight::DetectStillImage (int w, int h)
{
	if ( m_remoteformat.bitsPerPixel == 8 || w * h < DETECT_MIN_SIZE ||
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

	return (avgError < 500);
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
	return (avgError < 200);												  \
}

DEFINE_DETECT_FUNCTION(16)
DEFINE_DETECT_FUNCTION(32)

