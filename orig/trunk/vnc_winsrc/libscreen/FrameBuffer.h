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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "rfb/inttypes.h"
#include "Rect.h"
#include "PixelFormat.h"

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  bool setRect(const Rect *newRect, bool resizeBuff = true);
  inline Rect getRect() { return m_rect; }

  bool setPixelFormat(const PixelFormat *pixelFormat, bool resizeBuff);
  inline PixelFormat getPixelFormat() { return m_pixelFormat; }

  void setBuffer(void *newBuffer) { m_buffer = newBuffer; }
  inline virtual void *getBuffer() const { return m_buffer; }
  inline virtual int getBufferSize();

protected:
  bool resizeBuffer();

  Rect m_rect;

  PixelFormat m_pixelFormat;
  void *m_buffer;
};

#endif // __FRAMEBUFFER_H__
