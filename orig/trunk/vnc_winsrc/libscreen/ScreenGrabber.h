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

#ifndef __SCREENGRABBER_H__
#define __SCREENGRABBER_H__

#include "Rect.h"
#include "PixelFormat.h"
#include "FrameBuffer.h"

//
// An abstract interface for screen grabbing.
//

/*
  //
  // Usage example:
  //

  ScreenGrabber *frameBuffer;

  // Initialisation
  frameBuffer = new WindowsScreenGrabber;

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

class ScreenGrabber
{
public:
  ScreenGrabber(void);
  virtual ~ScreenGrabber(void);

  /* Provides grabbing.
  Parameters:     *rect - Pointer to a Rect object with relative workRect coordinates.
  Return value:   true if success.
  */
  virtual bool grab(const Rect *rect = 0) = 0;

  /* Provides read access to rectangular coordinates of the screen (desktop).*/
  virtual Rect getScreenRect() { return m_fullScreenRect; }

  // Checks screen(desktop) properties on changes
  inline virtual bool getPropertiesChanged() = 0;
  inline virtual bool getPixelFormatChanged() = 0;
  inline virtual bool getScreenSizeChanged() = 0;

  /* Set new values of m_workFrameBuffer and m_fullScreenRect 
  if properties of the screen is changed*/
  virtual bool applyNewProperties();

protected:
  virtual bool applyNewFullScreenRect() = 0;
  virtual bool applyNewPixelFormat() = 0;

  virtual bool setWorkRectDefault();

  Rect m_fullScreenRect;

  FrameBuffer m_workFrameBuffer;
};

#endif // __SCREENGRABBER_H__
