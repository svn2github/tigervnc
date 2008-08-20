//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

#include "WindowsFrameBuffer.h"

WindowsFrameBuffer::WindowsFrameBuffer(void)
{
}

WindowsFrameBuffer::~WindowsFrameBuffer(void)
{
}

HPALETTE WindowsFrameBuffer::GetSystemPalette()
{
  int paletteSize, logSize;
  PLOGPALETTE logPalette;
  HDC dc;
  HWND focus;
  HPALETTE result;

  focus = GetFocus();
  dc = GetDC(focus);

  paletteSize = GetDeviceCaps(dc, SIZEPALETTE);
  logSize = sizeof(LOGPALETTE) + (paletteSize - 1) * sizeof(PALETTEENTRY);
  logPalette = (PLOGPALETTE) malloc(logSize);
  if (logPalette == NULL) {
    ReleaseDC(focus, dc);
    return NULL;
  }

  logPalette->palVersion = 0x0300;
  logPalette->palNumEntries = paletteSize;
  GetSystemPaletteEntries(dc, 0, paletteSize, logPalette->palPalEntry);
  result = CreatePalette(logPalette);

  free(logPalette);
  ReleaseDC(focus, dc);

  return result;
}
  
void WindowsFrameBuffer::CaptureScreenRect(Rect *aRect, HDC dstDC)
{
  HDC screenDC;
  screenDC = GetDC(0);

  //BitBlt(dstDC, aRect->left, aRect->right, aRect->right, aRect->bottom, screenDC, 0, 0, SRCCOPY);

  ReleaseDC(0, screenDC);
}

void WindowsFrameBuffer::SetPropertiesChanged()
{
  // Check for changing

  return;
}

void WindowsFrameBuffer::Update()
{
  SetPropertiesChanged();

  return;
}
