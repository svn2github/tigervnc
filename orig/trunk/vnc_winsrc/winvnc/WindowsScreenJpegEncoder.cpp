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

#include "WindowsScreenJpegEncoder.h"

WindowsScreenJpegEncoder::WindowsScreenJpegEncoder()
{
}

WindowsScreenJpegEncoder::~WindowsScreenJpegEncoder()
{
}

UINT WindowsScreenJpegEncoder::getNumCodedRects(const RECT &rect) const
{
  return 1;
}

void WindowsScreenJpegEncoder::encodeRectangle(const RECT &rect)
{
  Rect r(rect.left, rect.top, rect.right, rect.bottom);
  m_grabber.grab(&r);
  const FrameBuffer *fb = m_grabber.getScreenBuffer();

  const CARD32 *ptr = (const CARD32 *)fb->getBuffer();
  int stride = fb->getDimension().width;
  ptr += r.top * stride + r.left;
  PixelFormat fmt = fb->getPixelFormat();
  _ASSERT(fmt.colorDepth == 24);

  m_compressor.compress(ptr, &fmt, r.getWidth(), r.getHeight(), stride);
}
