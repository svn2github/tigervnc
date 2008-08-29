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
: m_destDC(NULL), m_screenDC(NULL), m_hbmDIB(NULL), m_hbmOld(NULL)
{
  SetWorkRectDefault();
}

WindowsFrameBuffer::~WindowsFrameBuffer(void)
{
  CloseDIBSection();
}

bool WindowsFrameBuffer::ApplyNewProperties()
{
  if (!ApplyNewPixelFormat() || !ApplyNewFullScreenRect()) 
    return false;

  return OpenDIBSection();
}

bool WindowsFrameBuffer::OpenDIBSection()
{
  DeleteDC(m_screenDC);
  m_screenDC = GetDC(0);
  if (m_screenDC == NULL) {
    return false;
  }

  if (GetPropertiesChanged())
    return false;

  BMI bmi;
  if (!GetBMI(&bmi)) {
    return false;
  }

  bmi.bmiHeader.biWidth = m_workRect.GetWidth();
  bmi.bmiHeader.biHeight = -m_workRect.GetHeight();
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = m_pixelFormat.redMax   << m_pixelFormat.redShift;
  bmi.green = m_pixelFormat.greenMax << m_pixelFormat.greenShift;
  bmi.blue  = m_pixelFormat.blueMax  << m_pixelFormat.blueShift;

  DeleteDC(m_destDC);
  m_destDC = CreateCompatibleDC(NULL);
  if (m_destDC == NULL) {
    DeleteDC(m_screenDC);
    return false;
  }

  m_hbmDIB = CreateDIBSection(m_destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &m_buffer, NULL, NULL);
  if (m_hbmDIB == 0) {
    DeleteDC(m_destDC);
    DeleteDC(m_screenDC);
    return false;
  }
  m_hbmOld = (HBITMAP) SelectObject(m_destDC, m_hbmDIB);

  return true;
}

bool WindowsFrameBuffer::CloseDIBSection()
{
  // Free resources
  SelectObject(m_destDC, m_hbmOld);

  DeleteObject(m_hbmDIB);
  m_hbmDIB = NULL;

  DeleteDC(m_destDC);
  m_destDC = NULL;

  DeleteDC(m_screenDC);
  m_screenDC = NULL;

  m_buffer = NULL;
  return true;
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
  if (!GetBMI(&bmi))
    return false;

  return FillPixelFormat(&m_pixelFormat, &bmi);
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

bool WindowsFrameBuffer::Grab(const Rect *rect)
{
  return GrabByDIBSection(rect);;
}

bool WindowsFrameBuffer::GrabByGetDIBit(const Rect *rect)
{
  HDC m_destDC, screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  if (GetPropertiesChanged()) return false;

  m_destDC = CreateCompatibleDC(screenDC);

  HBITMAP hbmOld, hbm;

  hbm = CreateBitmap(m_workRect.GetWidth(), m_workRect.GetHeight(), 1, m_pixelFormat.bitsPerPixel, NULL);
  hbmOld = (HBITMAP) SelectObject(m_destDC, hbm);

  if (BitBlt(m_destDC, 0, 0, m_workRect.GetWidth(),
             m_workRect.GetHeight(), screenDC, m_workRect.left, m_workRect.top, SRCCOPY) == 0) {
    DeleteDC(m_destDC);
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
  if ((lines = GetDIBits(m_destDC, hbm, 0, abs(bmi.bmiHeader.biHeight), m_buffer, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS)) == 0) {
    SelectObject(m_destDC, hbmOld);
    DeleteObject(hbm);  
    DeleteDC(screenDC);
    return false;
  }

  SelectObject(m_destDC, hbmOld);
  DeleteObject(hbm);
  DeleteDC(m_destDC);
  DeleteDC(screenDC);
  return true;
}

bool WindowsFrameBuffer::GrabByDIBSection(const Rect *rect)
{
  if (GetPropertiesChanged())
    return false;

  if (BitBlt(m_destDC, rect->left - m_workRect.left, rect->top - m_workRect.top, rect->GetWidth(), rect->GetHeight(), 
             m_screenDC, rect->left, rect->top, SRCCOPY) == 0) {
    return false;
  }

  return true;
}

bool WindowsFrameBuffer::FillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi)
{
  memset(pixelFormat, 0, sizeof(PixelFormat));

  pixelFormat->bitsPerPixel = bmi->bmiHeader.biBitCount;

  if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
    pixelFormat->redShift   = findFirstBit(bmi->red);
    pixelFormat->greenShift = findFirstBit(bmi->green);
    pixelFormat->blueShift  = findFirstBit(bmi->blue);

    pixelFormat->redMax   = bmi->red    >> pixelFormat->redShift;
    pixelFormat->greenMax = bmi->green  >> pixelFormat->greenShift;
    pixelFormat->blueMax  = bmi->blue   >> pixelFormat->blueShift;
  } else {
    pixelFormat->bitsPerPixel = 32;
    pixelFormat->redMax = pixelFormat->greenMax = pixelFormat->blueMax = 0xff;
    pixelFormat->redShift   = 16;
    pixelFormat->greenShift = 8;
    pixelFormat->blueShift  = 0;
  }
  return (pixelFormat->redMax > 0) && (pixelFormat->greenMax > 0) && (pixelFormat->blueMax > 0);
}

int WindowsFrameBuffer::findFirstBit(const UINT32 bits)
{
  UINT32 b = bits;
  int shift;
  for (shift = 0; (shift < 32) && ((b & 1) == 0); shift++) {
    b >>= 1;
  }
  return shift;
}
