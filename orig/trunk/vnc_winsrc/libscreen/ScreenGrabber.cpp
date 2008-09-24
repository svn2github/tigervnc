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

#include "ScreenGrabber.h"
#include <stddef.h>
#include <memory.h>

ScreenGrabber::ScreenGrabber(void)
: m_buffer(NULL)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

ScreenGrabber::~ScreenGrabber(void)
{
}

bool ScreenGrabber::setWorkRect(const Rect *rect)
{
  m_workRect = *rect;

  bool result = m_frameBuffer.setRect(rect);
  m_buffer = m_frameBuffer.getBuffer();

  return result;
}

bool ScreenGrabber::applyNewProperties()
{
  if (!applyNewPixelFormat() || !applyNewFullScreenRect()) {
    return false;
  }

  return true;
}

bool ScreenGrabber::setWorkRectDefault()
{
  // Set m_workRect to full screen by default
  Rect rect;
  if (!applyNewFullScreenRect()) {
    return false;
  }

  getScreenRect(&rect);
  setWorkRect(&rect);
  return true;
}

bool ScreenGrabber::grab()
{
  Rect fullWork;
  // Set relative co-ordinates
  fullWork.left = 0; 
  fullWork.top = 0; 
  fullWork.setWidth(m_workRect.getWidth());
  fullWork.setHeight(m_workRect.getHeight());

  return grab(&fullWork);
}
