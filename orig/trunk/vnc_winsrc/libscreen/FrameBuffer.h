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

//
// An abstract interface for screen grabbing.
//

/*
  //
  // Usage example:
  //

  FrameBuffer *frameBuffer;

  // Initialisation
  frameBuffer = new WindowsFrameBuffer;

  Rect grabRect, workRect;
  workRect.setRect(100, 100, 500, 500);
  grabRect.setRect(20, 20, 120, 120); // Relative to the workRect
  frameBuffer->setWorkRect(&workRect);

  // One-time grabbing
  while (!frameBuffer->grab(&grabRect)) {
    if (frameBuffer->getPropertiesChanged()) { // Check desktop properties
      if (!frameBuffer->applyNewProperties()) {
        MessageBox(NULL, _T("Cannot apply new screen properties"), _T("Error"), MB_ICONHAND);
        return 1;
      }
    } else {
      MessageBox(NULL, _T("Cannot grab screen"), _T("Error"), MB_ICONHAND);
      return 1;
    }
  }
*/

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  /* Provides grabbing.
  Parameters:     *rect - Pointer to a Rect object with relative workRect coordinates.
  Return value:   true if success.
  */
  virtual bool grab(const Rect *rect) = 0;
  virtual bool grab();

  /* Esteblish rectangular work area.
  Parameters:     *rect - Pointer to a Rect object.
  Return value:   true if success.
  */
  virtual bool setWorkRect(const Rect *rect);

  /* Provides read access to the workRect.*/
  virtual void getWorkRect(Rect *rect)                  const { *rect = m_workRect; }
  virtual Rect getWorkRect()                            const { return m_workRect; }
  /* Provides read access to the pixelFormat.*/
  virtual void getPixelFormat(PixelFormat *pixelFormat) const { *pixelFormat = m_pixelFormat; }
  /* Provides read access to rectangular coordinates of the screen (desktop).*/
  virtual void getScreenRect(Rect *rect)                const { *rect = m_fullScreenRect; }
  /* Returns pointer to the m_buffer*/
  virtual void *getBuffer()                             const { return m_buffer; }
  virtual int getBufferSize()
  { 
    return (m_workRect.getWidth() * m_workRect.getHeight() * m_pixelFormat.bitsPerPixel) / 8;
  }

  // Checks screen(desktop) properties on changes
  inline virtual bool getPropertiesChanged() = 0;
  inline virtual bool getPixelFormatChanged() = 0;
  inline virtual bool getScreenSizeChanged() = 0;

  /* Set new values of m_buffer, m_pixelFormat and m_fullScreenRect 
  if properties of the screen is changed*/
  virtual bool applyNewProperties();

protected:
  virtual bool applyNewFullScreenRect() = 0;
  virtual bool applyNewPixelFormat() = 0;
  virtual bool applyNewBuffer();

  virtual bool setWorkRectDefault();

  // Pointer to the workRect bitmap data
  void *m_buffer;
  PixelFormat m_pixelFormat;
  Rect m_fullScreenRect;
  // Coordinates of rectangular work area
  Rect m_workRect;
};

#endif // __FRAMEBUFFER_H__
