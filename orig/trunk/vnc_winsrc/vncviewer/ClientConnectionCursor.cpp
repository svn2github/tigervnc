//  Copyright (C) 2000 Const Kaplinsky. All Rights Reserved.
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
// For the latest source code, please check:
//
// http://www.DevelopVNC.org/
//
// or send email to: feedback@developvnc.org.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

// XCursor and RichCursor encodings
//
// Support for cursor shape updates for ClientConnection class.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"

void ClientConnection::ReadXCursorShape(rfbFramebufferUpdateRectHeader *pfburh) {
	if (pfburh->r.w * pfburh->r.h) {
		int bytesPerRow = (pfburh->r.w + 7) / 8;
		int cursorWidth = GetSystemMetrics(SM_CXCURSOR);
		int cursorHeight = GetSystemMetrics(SM_CYCURSOR);

		// Read background and foreground colors for cursor
		CheckBufferSize(sz_rfbXCursorColors);
		ReadExact(m_netbuf, sz_rfbXCursorColors);

		// Read cursor shape, aligning every row at 4-byte boundary
		int bytesPerRowAligned = (bytesPerRow + 3) & ~0x03;
		CheckBufferSize(bytesPerRowAligned * pfburh->r.h * 2);
		CheckBufferSize(bytesPerRowAligned * pfburh->r.h * 2);
		for (int i = 0; i < pfburh->r.h * 2; i++) {
			ReadExact(&m_netbuf[i*bytesPerRowAligned], bytesPerRow);
		}

		// Draw bitmap

		SelectPalette(hMemDC, hOldPal, FALSE);
		DeleteObject((HGDIOBJ)hPal);
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hScreenDC);
	} else {
		SetDefaultCursor();		// FIXME: Why does it never execute?
		return;
	}
	// FIXME: Set custom cursor
}

void ClientConnection::ReadRichCursorShape(rfbFramebufferUpdateRectHeader *pfburh) {

}

void ClientConnection::SetDefaultCursor() {
	LONG hCursor;

	switch (m_opts.m_localCursor) {
	case NOCURSOR:
		hCursor = (LONG) LoadCursor(m_pApp->m_instance,
									MAKEINTRESOURCE(IDC_NOCURSOR));
		break;
	case NORMALCURSOR:
		hCursor = (LONG) LoadCursor(NULL, IDC_ARROW);
		break;
	case DOTCURSOR:
	default:
		hCursor = (LONG) LoadCursor(m_pApp->m_instance,
									MAKEINTRESOURCE(IDC_DOTCURSOR));
	}
	SetClassLong(m_hwnd, GCL_HCURSOR, hCursor);
}

/*
		// Prepare palette
		BYTE paletteBuf[sizeof(LOGPALETTE) + 2 * sizeof(PALETTEENTRY)];
		LOGPALETTE *paletteBits = (LOGPALETTE *) paletteBuf;
		paletteBits->palVersion = 0x300;
		paletteBits->palNumEntries = 2;
		paletteBits->palPalEntry[1].peRed = m_netbuf[0];
		paletteBits->palPalEntry[1].peGreen = m_netbuf[1];
		paletteBits->palPalEntry[1].peBlue = m_netbuf[2];
		paletteBits->palPalEntry[1].peFlags = NULL;
		paletteBits->palPalEntry[0].peRed = m_netbuf[3];
		paletteBits->palPalEntry[0].peGreen = m_netbuf[4];
		paletteBits->palPalEntry[0].peBlue = m_netbuf[5];
		paletteBits->palPalEntry[0].peFlags = NULL;
		HPALETTE hPal = CreatePalette(paletteBits);

		HDC hScreenDC = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hScreenDC);
		HPALETTE hOldPal = SelectPalette(hMemDC, hPal, FALSE);
		RealizePalette(hMemDC);
*/
