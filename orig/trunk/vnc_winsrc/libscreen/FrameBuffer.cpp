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

#include "FrameBuffer.h"
#include <string.h>

FrameBuffer::FrameBuffer(void)
: m_buffer(0)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

FrameBuffer::~FrameBuffer(void)
{
}

bool FrameBuffer::cmp(FrameBuffer *frameBuffer)
{
  return (m_rect.cmpRect(&(frameBuffer->getRect()))) && 
          memcmp(&m_pixelFormat, &(frameBuffer->getPixelFormat()), sizeof(PixelFormat));
}

bool FrameBuffer::setPixelFormat(const PixelFormat *pixelFormat, bool resizeBuff)
{
  m_pixelFormat = *pixelFormat;

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

bool FrameBuffer::setRect(const Rect *newRect, bool resizeBuff)
{
  UINT32 newArea = newRect->area();
  UINT32 oldArea = m_rect.area();

  m_rect = *newRect;

  if (resizeBuff && (newArea != oldArea))
  {
    return resizeBuffer();
  }
  return true;
}

int FrameBuffer::getBufferSize()
{ 
  return (m_rect.getWidth() * m_rect.getHeight() * m_pixelFormat.bitsPerPixel) / 8;
}

bool FrameBuffer::resizeBuffer()
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
  if ((m_buffer = new UINT8[getBufferSize()]) == 0) {
    return false;
  }
  return true;
}
