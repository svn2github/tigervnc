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

#ifndef __WINDOWSFRAMEBUFFER_H__
#define __WINDOWSFRAMEBUFFER_H__

#include <windows.h>
#include "framebuffer.h"
#include "rect.h"
#include "inttypes.h"

/**/

class WindowsFrameBuffer :
  public FrameBuffer
{
public:
  WindowsFrameBuffer(void);
  virtual ~WindowsFrameBuffer(void);

  virtual bool Grab(const Rect *rect);

  inline virtual bool GetPropertiesChanged();
  inline virtual bool GetPixelFormatChanged();
  inline virtual bool GetSizeChanged();

  virtual bool ApplyNewFullScreenRect();
  virtual bool ApplyNewPixelFormat();

protected:
  virtual bool ApplyNewProperties();
  virtual bool ApplyNewBuffer() { return OpenDIBSection(); }; // Overriding

  struct BMI
  {
    BITMAPINFOHEADER bmiHeader;
    UINT32 red;
    UINT32 green;
    UINT32 blue;
  };

  inline bool GetBMI(BMI *bmi);

  virtual bool OpenDIBSection();
  virtual bool CloseDIBSection();
  virtual bool GrabByDIBSection(const Rect *rect);
  virtual bool FillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi);

  // Find position of first bit = 1
  inline int findFirstBit(const UINT32 bits);

  // Windows specific variebles
  HDC m_destDC, m_screenDC;
  HBITMAP m_hbmOld, m_hbmDIB;
};

#endif // __WINDOWSFRAMEBUFFER_H__
