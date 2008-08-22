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
}

WindowsFrameBuffer::~WindowsFrameBuffer(void)
{
}

bool WindowsFrameBuffer::UpdatePixelFormat()
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    m_lastError = E_GET_DC;
    return false;
  }

  HBITMAP hbm;
  struct {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[16];
  } bmi;

  bmi.bmiHeader.biBitCount = 0;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  hbm = (HBITMAP) GetCurrentObject(screenDC, OBJ_BITMAP);
  if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), NULL, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS) == 0) {
    m_lastError = GetLastError();
    return false;
  }
  return true;
}

bool WindowsFrameBuffer::UpdateFullScreenRect()
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    m_lastError = E_GET_DC;
    return false;
  }

  // Check for resolution changing
  int horzres, vertrez;
  horzres = GetDeviceCaps(screenDC, HORZRES);
  vertrez = GetDeviceCaps(screenDC, VERTRES);
  if (horzres != m_fullScreenRect.right || vertrez != m_fullScreenRect.bottom) {
    m_fullScreenRect.SetRect(0, 0, horzres, vertrez);
    m_sizeChanged = true;
  } else {
    m_sizeChanged = false;
  }
  return true;
}


bool WindowsFrameBuffer::CheckPropertiesChanged()
{
  // Check for resolution changing
  if (!UpdateFullScreenRect()) {
    return false;
  }

  // Check for pixel format changing

  return true;
}

bool WindowsFrameBuffer::Grab()
{
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }
  if (!CheckPropertiesChanged()) return false;
  if (m_sizeChanged) {
    if (m_buffer != NULL) delete[] m_buffer;
    if ((m_buffer = new unsigned long[m_fullScreenRect.GetWidth() * m_fullScreenRect.GetHeight()]) == NULL) {
      m_lastError = E_NO_MEMORY_FOUND;
      return false;
    }
  }

  HBITMAP hbm;
  struct {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[16];
  } bmi;

  bmi.bmiHeader.biBitCount = 0;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  hbm = (HBITMAP) GetCurrentObject(screenDC, OBJ_BITMAP);
  if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), NULL, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS) == 0) {
    m_lastError = GetLastError();
    return false;
  }
  GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), m_buffer, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS);
  
  ReleaseDC(NULL, screenDC);
  return true;
}
