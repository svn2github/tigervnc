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
#include <stddef.h>
#include <memory.h>

FrameBuffer::FrameBuffer(void)
: m_buffer(NULL)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

FrameBuffer::~FrameBuffer(void)
{
}

bool FrameBuffer::SetWorkRect(const Rect *rect)
{
  if (m_workRect.CmpRect(rect)) { return true; }

  m_workRect = *rect;
  
  return ApplyNewBuffer();
}

bool FrameBuffer::ApplyNewProperties()
{
  if (!ApplyNewPixelFormat() || !ApplyNewFullScreenRect()) return false;
  if (!ApplyNewBuffer()) return false;
  return true;
}

bool FrameBuffer::ApplyNewBuffer()
{
  if (m_buffer != NULL) delete[] m_buffer;

  m_buffer = new char[m_workRect.GetWidth() * m_workRect.GetHeight() * m_pixelFormat.bitsPerPixel / 8];
  return (m_buffer != NULL);
}

bool FrameBuffer::SetWorkRectDefault()
{
  // Set m_workRect to full screen by default
  Rect rect;
  if (!ApplyNewFullScreenRect()) return false;
  GetFullScreenRect(&rect);
  SetWorkRect(&rect);
  return true;
}
