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
    return false;
  }

  PixelFormat pixelFormat;
  FillPixelFormat(&pixelFormat, &bmi);

  if (memcmp(&m_pixelFormat, &pixelFormat, sizeof(PixelFormat))) 
    return true;

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
    DeleteObject(hbm);
    DeleteDC(screenDC);
    return false;
  }

  // The color table is filled only if it is used BI_BITFIELDS
  if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
    if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.GetHeight(), NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
      DeleteObject(hbm);
      DeleteDC(screenDC);
      return false;
    }
  }
  DeleteObject(hbm);
  DeleteDC(screenDC);
  return true;
}

bool WindowsFrameBuffer::ApplyNewPixelFormat()
{
  BMI bmi;
  if (!GetBMI(&bmi)) return false;

  FillPixelFormat(&m_pixelFormat, &bmi);
  return true;
}
void WindowsFrameBuffer::FillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi)
{
  memset(&pixelFormat, 0, sizeof(PixelFormat));
  m_pixelFormat.bitsPerPixel = bmi->bmiHeader.biBitCount;

  int cmpr = bmi->bmiHeader.biCompression;
  if (cmpr == BI_RGB || cmpr == BI_BITFIELDS) {

  } else {
    //m_pixelFormat.redMax = 
  }
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
  bool result = true;
  result = GrabByDIBSection();
  return result;
}

bool WindowsFrameBuffer::GrabByGetDIBit()
{
  HDC destDC, screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  if (GetPropertiesChanged()) return false;

  destDC = CreateCompatibleDC(screenDC);

  HBITMAP hbmOld, hbm;

  hbm = CreateBitmap(m_workRect.GetWidth(), m_workRect.GetHeight(), 1, m_pixelFormat.bitsPerPixel, NULL);
  hbmOld = (HBITMAP) SelectObject(destDC, hbm);

  if (BitBlt(destDC, 0, 0, m_workRect.GetWidth(),
             m_workRect.GetHeight(), screenDC, m_workRect.left, m_workRect.top, SRCCOPY) == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return false;
  }

  BMI bmi;
  if (!GetBMI(&bmi)) {
    return false;
  }
  bmi.bmiHeader.biWidth = m_workRect.GetWidth();
  bmi.bmiHeader.biHeight = -m_workRect.GetHeight();

  int lines;
  if ((lines = GetDIBits(destDC, hbm, 0, abs(bmi.bmiHeader.biHeight), m_buffer, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS)) == 0) {
    SelectObject(destDC, hbmOld);
    DeleteObject(hbm);  
    DeleteDC(screenDC);
    return false;
  }

  SelectObject(destDC, hbmOld);
  DeleteObject(hbm);
  DeleteDC(destDC);
  DeleteDC(screenDC);
  return true;
}

bool WindowsFrameBuffer::GrabByDIBSection()
{
  HDC destDC, screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  if (GetPropertiesChanged()) return false;

  BMI bmi;
  if (!GetBMI(&bmi)) {
    return false;
  }
  bmi.bmiHeader.biWidth = m_workRect.GetWidth();
  bmi.bmiHeader.biHeight = -m_workRect.GetHeight();
  //bmi.bmiHeader.biCompression = BI_RGB;

  destDC = CreateCompatibleDC(NULL);

  HBITMAP hbmOld, hbm;
  void *sysBuffer = NULL;

  hbm = CreateDIBSection(destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &sysBuffer, NULL, NULL);
  if (hbm == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return false;
  }
  hbmOld = (HBITMAP) SelectObject(destDC, hbm);

  if (BitBlt(destDC, 0, 0, m_workRect.GetWidth(),
             m_workRect.GetHeight(), screenDC, m_workRect.left, m_workRect.top, SRCCOPY) == 0) {
    SelectObject(destDC, hbmOld);
    DeleteObject(hbm);
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return false;
  }

  memcpy(m_buffer, sysBuffer, GetBufferSize());

  SelectObject(destDC, hbmOld);
  DeleteObject(hbm);
  DeleteDC(destDC);
  DeleteDC(screenDC);
  return true;
}
