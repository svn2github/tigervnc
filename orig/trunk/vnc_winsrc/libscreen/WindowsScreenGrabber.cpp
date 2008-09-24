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

#include "WindowsScreenGrabber.h"

WindowsScreenGrabber::WindowsScreenGrabber(void)
: m_destDC(NULL), m_screenDC(NULL), m_hbmDIB(NULL), m_hbmOld(NULL)
{
  setWorkRectDefault();
  applyNewProperties();
}

WindowsScreenGrabber::~WindowsScreenGrabber(void)
{
  closeDIBSection();
}

bool WindowsScreenGrabber::applyNewProperties()
{
  if (!applyNewPixelFormat() || !applyNewFullScreenRect() || !openDIBSection()) 
  {
    return false;
  }

  return true;
}

bool WindowsScreenGrabber::setWorkRect(const Rect *rect)
{
  INT32 oldArea = abs(m_frameBuffer.getRect().area());

  m_frameBuffer.setRect(rect, false);
  m_workRect = m_frameBuffer.getRect();

  if (abs(m_frameBuffer.getRect().area()) > oldArea)
  {
    bool result = openDIBSection();
    m_frameBuffer.setBuffer(m_buffer);
  }

  return true;
}

bool WindowsScreenGrabber::openDIBSection()
{
  closeDIBSection();

  m_frameBuffer.setBuffer(0);

  m_screenDC = GetDC(0);
  if (m_screenDC == NULL) {
    return false;
  }

  if (getPropertiesChanged()) {
    return false;
  }

  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  bmi.bmiHeader.biBitCount = m_pixelFormat.bitsPerPixel;
  bmi.bmiHeader.biWidth = m_workRect.getWidth();
  bmi.bmiHeader.biHeight = -m_workRect.getHeight();
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = m_pixelFormat.redMax   << m_pixelFormat.redShift;
  bmi.green = m_pixelFormat.greenMax << m_pixelFormat.greenShift;
  bmi.blue  = m_pixelFormat.blueMax  << m_pixelFormat.blueShift;

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

  m_frameBuffer.setBuffer(m_buffer);

  return true;
}

bool WindowsScreenGrabber::closeDIBSection()
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

bool WindowsScreenGrabber::getPropertiesChanged()
{
  // Check for changing
  if (getScreenSizeChanged() || getPixelFormatChanged()) {
    return true;
  }

  return false;
}

bool WindowsScreenGrabber::getPixelFormatChanged()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  PixelFormat pixelFormat;
  fillPixelFormat(&pixelFormat, &bmi);

  if (memcmp(&m_pixelFormat, &pixelFormat, sizeof(PixelFormat))) {
    return true;
  }

  return false;
}

bool WindowsScreenGrabber::getScreenSizeChanged()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  int width = bmi.bmiHeader.biWidth;
  int height = bmi.bmiHeader.biHeight;

  if (width != m_fullScreenRect.getWidth() || height != m_fullScreenRect.getHeight()) {
    return true;
  }

  return false;
}

bool WindowsScreenGrabber::getBMI(BMI *bmi)
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
  if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.getHeight(), NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
    DeleteObject(hbm);
    DeleteDC(screenDC);
    return false;
  }

  // The color table is filled only if it is used BI_BITFIELDS
  if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
    if (GetDIBits(screenDC, hbm, 0, m_fullScreenRect.getHeight(), NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
      DeleteObject(hbm);
      DeleteDC(screenDC);
      return false;
    }
  }
  DeleteObject(hbm);
  DeleteDC(screenDC);
  return true;
}

bool WindowsScreenGrabber::applyNewPixelFormat()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  bool result = fillPixelFormat(&m_pixelFormat, &bmi);
  m_frameBuffer.setPixelFormat(&m_pixelFormat, false);

  return result;
}

bool WindowsScreenGrabber::applyNewFullScreenRect()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  m_fullScreenRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  m_fullScreenRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  m_fullScreenRect.setWidth(GetSystemMetrics(SM_CXVIRTUALSCREEN));
  m_fullScreenRect.setHeight(GetSystemMetrics(SM_CYVIRTUALSCREEN));

  return true;
}

bool WindowsScreenGrabber::grab(const Rect *rect)
{
  return grabByDIBSection(rect);;
}

bool WindowsScreenGrabber::grabByDIBSection(const Rect *rect)
{
  if (getPropertiesChanged()) {
    return false;
  }

  if (BitBlt(m_destDC, rect->left, rect->top, rect->getWidth(), rect->getHeight(), 
             m_screenDC, rect->left + m_workRect.left, rect->top + m_workRect.top, SRCCOPY | CAPTUREBLT) == 0) {
    return false;
  }

  return true;
}

bool WindowsScreenGrabber::fillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi)
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
    pixelFormat->colorDepth = 24;
    pixelFormat->redMax = pixelFormat->greenMax = pixelFormat->blueMax = 0xff;
    pixelFormat->redShift   = 16;
    pixelFormat->greenShift = 8;
    pixelFormat->blueShift  = 0;
  }

  if (pixelFormat->bitsPerPixel == 32) {
    pixelFormat->colorDepth = 24;
  } else {
    pixelFormat->colorDepth = 16;
  }

  return (pixelFormat->redMax > 0)
    && (pixelFormat->greenMax > 0)
    && (pixelFormat->blueMax > 0);
}

int WindowsScreenGrabber::findFirstBit(const UINT32 bits)
{
  UINT32 b = bits;
  int shift;

  for (shift = 0; (shift < 32) && ((b & 1) == 0); shift++) {
    b >>= 1;
  }

  return shift;
}
