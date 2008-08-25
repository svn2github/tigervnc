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
#include "ErrorDef.h"

WindowsFrameBuffer::WindowsFrameBuffer(void)
{
  SetWorkRectDefault();
}

WindowsFrameBuffer::~WindowsFrameBuffer(void)
{
}

bool WindowsFrameBuffer::GetPropertiesChanged()
{
  // Check for changing
  if (GetSizeChanged() || GetPixelFormatChanged()) return true;
  return false;
}

bool WindowsFrameBuffer::GetPixelFormatChanged()
{
  BMI bmi;
  if (!GetBMI(&bmi)) {
    return true;
  }

  PixelFormat pixelFormat;
  memset(&pixelFormat, 0, sizeof(PixelFormat));
  pixelFormat.bitsPerPixel = bmi.bmiHeader.biBitCount;

  if (memcmp(&m_pixelFormat, &pixelFormat, sizeof(PixelFormat))) return true;

  return false;
}

bool WindowsFrameBuffer::GetSizeChanged()
{
  return false;
}

bool WindowsFrameBuffer::GetBMI(BMI *bmi)
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  memset(bmi, 0, sizeof(BMI));
  bmi->bmiHeader.biBitCount = 0;
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  HBITMAP hbm;
  hbm = (HBITMAP) GetCurrentObject(screenDC, OBJ_BITMAP);
  if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
    return false;
  }
  return true;
}


bool WindowsFrameBuffer::ApplyNewPixelFormat()
{
  BMI bmi;
  if (!GetBMI(&bmi)) return false;

  m_pixelFormat.bitsPerPixel = bmi.bmiHeader.biBitCount;

  return true;
}

bool WindowsFrameBuffer::ApplyNewFullScreenRect()
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  m_fullScreenRect.SetRect(0, 0, GetDeviceCaps(screenDC, HORZRES), GetDeviceCaps(screenDC, VERTRES));

  return true;
}

bool WindowsFrameBuffer::Grab()
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }
  if (GetPropertiesChanged()) return false;

  HBITMAP hbm;
  BMI bmi;

  if (!GetBMI(&bmi)) return false;
  hbm = (HBITMAP) GetCurrentObject(screenDC, OBJ_BITMAP);
  int lines;
  if ((lines = GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), m_buffer, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS)) == 0){
    ReleaseDC(NULL, screenDC);
    return false;
  }

  ReleaseDC(NULL, screenDC);
  return true;
}
