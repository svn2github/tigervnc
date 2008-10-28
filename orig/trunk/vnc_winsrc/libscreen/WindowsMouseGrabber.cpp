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

#include "WindowsMouseGrabber.h"
#include "WindowsScreenGrabber.h"

WindowsMouseGrabber::WindowsMouseGrabber(void)
{
}

WindowsMouseGrabber::~WindowsMouseGrabber(void)
{
}

bool WindowsMouseGrabber::grab(PixelFormat *pixelFormat)
{
  return grabPixels(pixelFormat);
}

bool WindowsMouseGrabber::grabPixels(PixelFormat *pixelFormat)
{
  CURSORINFO cursorInfo;
  cursorInfo.cbSize = sizeof(CURSORINFO);
  GetCursorInfo(&cursorInfo);

  if (GetCursorInfo(&cursorInfo) == 0) {
    return false;
  }

  HCURSOR hCursor = cursorInfo.hCursor;
  if (hCursor == NULL) {
    return false;
  }

  // Get bitmap mask
  ICONINFO iconInfo;
  if (!GetIconInfo(hCursor, &iconInfo)) {
    return false;
  }

  if (iconInfo.hbmMask == NULL) {
    return false;
  }

  BITMAP bmMask;
  if (!GetObject(iconInfo.hbmMask, sizeof(BITMAP), (LPVOID)&bmMask)) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  if (bmMask.bmPlanes != 1 || bmMask.bmBitsPixel != 1) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  int width = bmMask.bmWidth;
  int height = bmMask.bmHeight;
  int widthBytes = bmMask.bmWidthBytes;

  PixelFormat pf;
  pf.bitsPerPixel = 1;
  m_mask.setPixelFormat(&pf, false);
  m_mask.setDimension(&Dimension(widthBytes, height));

  bool result = GetBitmapBits(iconInfo.hbmMask,
                              bmMask.bmWidthBytes * bmMask.bmHeight,
                              m_mask.getBuffer()) != 0;
  DeleteObject(iconInfo.hbmMask);
  if (!result) {
    return false;
  }

  // Get cursor pixels
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  WindowsScreenGrabber::BMI bmi;
  if (!WindowsScreenGrabber::getBMI(&bmi, screenDC)) {
    return false;
  }

  bmi.bmiHeader.biBitCount = pixelFormat->bitsPerPixel;
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = pixelFormat->redMax   << pixelFormat->redShift;
  bmi.green = pixelFormat->greenMax << pixelFormat->greenShift;
  bmi.blue  = pixelFormat->blueMax  << pixelFormat->blueShift;

  HDC destDC = CreateCompatibleDC(NULL);
  if (destDC == NULL) {
    DeleteDC(screenDC);
    return FALSE;
  }

  void *buffer;
  HBITMAP hbmDIB = CreateDIBSection(destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &buffer, NULL, NULL);
  if (hbmDIB == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return FALSE;
  }

  m_mask.setBuffer(buffer);

  HBITMAP hbmOld = (HBITMAP) SelectObject(destDC, hbmDIB);

  memset(buffer, 0xff, width * height * 4);

  result = DrawIconEx(destDC, 0, 0, hCursor, 0, 0, 0, NULL, DI_IMAGE);

  SelectObject(destDC, hbmOld);
  DeleteObject(hbmDIB);
  DeleteDC(destDC);
  DeleteDC(screenDC);

  return result != 0;
}
