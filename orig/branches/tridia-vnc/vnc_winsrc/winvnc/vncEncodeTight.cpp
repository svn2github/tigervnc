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
// most screen data and usually three times as efficient as hextile.
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
	int result = TIGHT_MAX_RECT_WIDTH * TIGHT_MAX_RECT_HEIGHT *
		(m_remoteformat.bitsPerPixel / 8);
	result += result / 100 + 16;

	return result;
}

UINT
vncEncodeTight::NumCodedRects(RECT &rect)
{
	const int w = rect.right - rect.left;
	const int h = rect.bottom - rect.top;

	if ((w > 8 && h > 8 && w * h > 16384) || w > 2048 || h > 2048) {
		return (((w-1) / TIGHT_MAX_RECT_WIDTH + 1) *
				((h-1) / TIGHT_MAX_RECT_HEIGHT + 1));
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

	int rawDataSize = TIGHT_MAX_RECT_WIDTH * TIGHT_MAX_RECT_HEIGHT *
		(m_remoteformat.bitsPerPixel / 8);

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

	if ((w > 8 && h > 8 && w * h > 16384) || w > 2048 || h > 2048) {
		int dx, dy, rw, rh;

		for (dy = 0; dy < h; dy += TIGHT_MAX_RECT_HEIGHT) {
			for (dx = 0; dx < w; dx += TIGHT_MAX_RECT_WIDTH) {
				rw = (dx + TIGHT_MAX_RECT_WIDTH < w) ?
					TIGHT_MAX_RECT_WIDTH : w - dx;
				rh = (dy + TIGHT_MAX_RECT_HEIGHT < h) ?
					TIGHT_MAX_RECT_HEIGHT : h - dy;
				partialSize = EncodeSubrect(source, outConn, dest,
											x+dx, y+dy, rw, rh);
				totalSize += partialSize;
				if ( dy + TIGHT_MAX_RECT_HEIGHT < h ||
					 dx + TIGHT_MAX_RECT_WIDTH < w ) {
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

	FillPalette(w, h);

	int dataSize;
	switch (m_paletteNumColors) {

	case 1:
		// Solid rectangle
		dataSize = SendSolidRect(dest, w, h);
		break;
	case 0:
		// Truecolor image
		dataSize = SendFullColorRect(dest, w, h);
		break;
	default:
		// Up to 256 different colors
		if (w * h >= m_paletteNumColors * 128)
			dataSize = SendIndexedRect(dest, w, h);
		else
			dataSize = SendFullColorRect(dest, w, h);
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

	if ( m_remoteformat.depth == 24 && m_remoteformat.redMax == 0xFF &&
		 m_remoteformat.greenMax == 0xFF && m_remoteformat.blueMax == 0xFF ) {
		Pack24(m_buffer, &m_remoteformat, 1);
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
	} else if (m_paletteNumColors <= 32) {
		streamId = 2;
		dataLen = w * h;
	} else {
		streamId = 3;
		dataLen = w * h;
	}
	m_hdrBuffer[m_hdrBufferBytes++] = streamId << 4 | 0x40;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)(m_paletteNumColors - 1);

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD32 *)paletteBuf)[i] =
				m_palette.entry[i].listNode->rgb;
		}

		if ( m_remoteformat.depth == 24 && m_remoteformat.redMax == 0xFF &&
			 m_remoteformat.greenMax == 0xFF && m_remoteformat.blueMax == 0xFF ) {
			Pack24(paletteBuf, &m_remoteformat, m_paletteNumColors);
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

	if ( m_remoteformat.depth == 24 && m_remoteformat.redMax == 0xFF &&
		 m_remoteformat.greenMax == 0xFF && m_remoteformat.blueMax == 0xFF ) {
		Pack24(m_buffer, &m_remoteformat, w * h);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	return CompressData(dest, 0, w * h * len);
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

		if (deflateInit2 (pz, 9, Z_DEFLATED, MAX_WBITS,
						  MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
			return -1;
		}
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
vncEncodeTight::FillPalette(int w, int h)
{
	int i;
	CARD8 *data8 = (CARD8 *)m_buffer;
	CARD16 *data16 = (CARD16 *)m_buffer;
	CARD32 *data32 = (CARD32 *)m_buffer;

	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		PaletteReset();
		for (i = 0; i <= w * h; i++) {
			if (!PaletteInsert (data32[i]))
				return;
		}
		break;
	case 16:
		PaletteReset();
		for (i = 0; i <= w * h; i++) {
			if (!PaletteInsert ((CARD32)data16[i]))
				return;
		}
		break;
	default:                    // bpp == 8
		PaletteReset8();
		for (i = 0; i <= w * h; i++) {
			if (!PaletteInsert8 (data8[i]))
				return;
		}
		break;
	}
}


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
    CARD8 *crgb = (CARD8 *)&rgb;

    pnode = m_palette.hash[(int)(crgb[0] + crgb[1] + crgb[2]) & 0xFF];

    while (pnode != NULL) {
        if (pnode->rgb == rgb)
            return pnode->idx;
        pnode = pnode->next;
    }
    return -1;
}

int
vncEncodeTight::PaletteInsert(CARD32 rgb)
{
    COLOR_LIST *pnode, *new_pnode;
    COLOR_LIST *prev_pnode = NULL;
    int hash_key, idx, new_idx, count;
    CARD8 *crgb = (CARD8 *)&rgb;

    hash_key = (int)(crgb[0] + crgb[1] + crgb[2]) & 0xFF;
    pnode = m_palette.hash[hash_key];

    while (pnode != NULL) {
        if (pnode->rgb == rgb) {
            // Such palette entry already exists.
            new_idx = idx = pnode->idx;
            count = m_palette.entry[idx].numPixels;
            if (new_idx && count == m_palette.entry[new_idx-1].numPixels) {
                do {
                    new_idx--;
                }
                while (new_idx &&
                       count == m_palette.entry[new_idx-1].numPixels);
                // Preserve sort order.
                new_pnode = m_palette.entry[new_idx].listNode;
                m_palette.entry[idx].listNode = new_pnode;
                m_palette.entry[new_idx].listNode = pnode;
                pnode->idx = new_idx;
                new_pnode->idx = idx;
            }
            m_palette.entry[new_idx].numPixels++;
            return m_paletteNumColors;
        }
        prev_pnode = pnode;
        pnode = pnode->next;
    }

    if (m_paletteNumColors == 256) {
        m_paletteNumColors = 0;
        return 0;
    }

    // Add new palette entry.

    idx = m_paletteNumColors;
    pnode = &m_palette.list[idx];
    if (prev_pnode != NULL) {
        prev_pnode->next = pnode;
    } else {
        m_palette.hash[hash_key] = pnode;
    }
    pnode->next = NULL;
    pnode->idx = idx;
    pnode->rgb = rgb;
    m_palette.entry[idx].listNode = pnode;
    m_palette.entry[idx].numPixels = 1;

    return (++m_paletteNumColors);
}

void
vncEncodeTight::PaletteReset8(void)
{
    int i;

    m_paletteNumColors = 0;
    for (i = 0; i < 256; i++)
        m_palette8.colorIdx[i] = 0xFF;
}

int
vncEncodeTight::PaletteInsert8(CARD8 value)
{
    int idx;

    idx = m_palette8.colorIdx[value];
    if (idx != 0xFF) {
        // Such palette entry already exists.
        m_palette8.numPixels[idx]++;
        if (idx && m_palette8.numPixels[1] > m_palette8.numPixels[0]) {
            // Preserve sort order.
            m_palette8.numPixels[0]++;
            m_palette8.numPixels[1]--;
            m_palette8.pixelValue[1] = m_palette8.pixelValue[0];
            m_palette8.pixelValue[0] = value;
            m_palette8.colorIdx[value] = 0;
            m_palette8.colorIdx[m_palette8.pixelValue[1]] = 1;
        }
        return m_paletteNumColors;
    }
    if (m_paletteNumColors == 2) {
        m_paletteNumColors = 0;
        return 0;
    }

    // Add new palette entry.

    idx = m_paletteNumColors;
    m_palette8.colorIdx[value] = idx;
    m_palette8.pixelValue[idx] = value;
    m_palette8.numPixels[idx] = 1;

    return (++m_paletteNumColors);
}


//
// Converting from 32-bit color samples to 24-bit colors.
// Should be called only when redMax, greenMax and blueMax are 256.
//

void
vncEncodeTight::Pack24(BYTE *buf, rfbPixelFormat *format, int count)
{
    int i;
    CARD32 pix;

    for (i = 0; i < count; i++) {
        pix = ((CARD32 *)buf)[i];
        buf[i*3]   = (BYTE)(pix >> format->redShift);
        buf[i*3+1] = (BYTE)(pix >> format->greenShift);
        buf[i*3+2] = (BYTE)(pix >> format->blueShift);
    }
}


//
// Most ugly part.
//

#define PaletteFind8(c)  m_palette8.colorIdx[(c)]
#define PaletteFind16    PaletteFind
#define PaletteFind32    PaletteFind

#define DEFINE_IDX_ENCODE_FUNCTION(bpp)										  \
																			  \
void																		  \
vncEncodeTight::EncodeIndexedRect##bpp(CARD8 *buf, int w, int h)			  \
{																			  \
    int x, y, i, width_bytes;												  \
																			  \
    if (m_paletteNumColors != 2) {											  \
      for (i = 0; i < w * h; i++)											  \
        buf[i] = (CARD8)PaletteFind(((CARD##bpp *)buf)[i]);					  \
      return;																  \
    }																		  \
																			  \
    width_bytes = (w + 7) / 8;												  \
    for (y = 0; y < h; y++) {												  \
      for (x = 0; x < w / 8; x++) {											  \
        for (i = 0; i < 8; i++)												  \
          buf[y*width_bytes+x] = (buf[y*width_bytes+x] << 1) |				  \
            (PaletteFind##bpp (((CARD##bpp *)buf)[y*w+x*8+i]) & 1);			  \
      }																		  \
      buf[y*width_bytes+x] = 0;												  \
      for (i = 0; i < w % 8; i++) {											  \
        buf[y*width_bytes+x] |=												  \
          (PaletteFind##bpp (((CARD##bpp *)buf)[y*w+x*8+i]) & 1) << (7 - i);  \
      }																		  \
   }																		  \
}

DEFINE_IDX_ENCODE_FUNCTION(8)
DEFINE_IDX_ENCODE_FUNCTION(16)
DEFINE_IDX_ENCODE_FUNCTION(32)

