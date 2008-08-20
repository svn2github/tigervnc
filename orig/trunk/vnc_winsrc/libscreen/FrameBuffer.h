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

  virtual void Update() = 0;

  virtual PixelFormat GetPixelFormat()  { return m_pixelFormat; }
  virtual Rect GetRect()                { return m_rect; }
  virtual void *GetBuffer()             { return m_buffer; }

  inline virtual bool GetPixelFormatChanged()  const { return m_pixelFormatChanged; }
  inline virtual bool GetSizeChanged()         const { return m_sizeChanged; }

protected:
  virtual void SetPropertiesChanged() = 0;

  PixelFormat m_pixelFormat;
  Rect m_rect;
  void *m_buffer;

  bool m_pixelFormatChanged;
  bool m_sizeChanged;
};

#endif // __FRAMEBUFFER_H__
