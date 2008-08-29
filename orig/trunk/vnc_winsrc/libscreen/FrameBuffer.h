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

#include "rect.h"
#include "pixelformat.h"

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  virtual bool grab(const Rect *rect) = 0;

  virtual bool setWorkRect(const Rect *rect);

  virtual void getWorkRect(Rect *rect)                  const { *rect = m_workRect; }
  virtual void getPixelFormat(PixelFormat *pixelFormat) const { *pixelFormat = m_pixelFormat; }
  virtual void getScreenRect(Rect *rect)            const { *rect = m_fullScreenRect; }
  virtual void *getBuffer()                             const { return m_buffer; }
  virtual int getBufferSize()
  { 
    return (m_workRect.getWidth() * m_workRect.getHeight() * m_pixelFormat.bitsPerPixel) / 8;
  }

  inline virtual bool getPropertiesChanged() = 0;
  inline virtual bool getPixelFormatChanged() = 0;
  inline virtual bool getScreenSizeChanged() = 0;

  virtual bool applyNewProperties();

protected:
  virtual bool applyNewFullScreenRect() = 0;
  virtual bool applyNewPixelFormat() = 0;
  virtual bool applyNewBuffer();

  virtual bool setWorkRectDefault();

  void *m_buffer;
  PixelFormat m_pixelFormat;
  Rect m_fullScreenRect;
  Rect m_workRect;
};

#endif // __FRAMEBUFFER_H__
