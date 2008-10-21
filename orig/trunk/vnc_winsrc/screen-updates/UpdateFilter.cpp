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

#include "UpdateFilter.h"

UpdateFilter::UpdateFilter(ScreenGrabber *screenGrabber,
                           FrameBuffer *frameBuffer,
                           CriticalSection *frameBufferCriticalSection)
: m_screenGrabber(screenGrabber),
m_frameBuffer(frameBuffer),
m_frameBufferCriticalSection(frameBufferCriticalSection)
{
}

UpdateFilter::~UpdateFilter(void)
{
}

void UpdateFilter::filter(UpdateContainer *updateContainer)
{
  rfb::Region tmpChangedRegion;

  m_frameBufferCriticalSection->enter();

  FrameBuffer *screenFrameBuffer = m_screenGrabber->getScreenBuffer();

  // Checking for buffers equal
  if (!screenFrameBuffer->cmp(m_frameBuffer)) {
    m_frameBufferCriticalSection->leave();
    return;
  }

  // Getting rectangle vector
  std::vector<Rect> rects;
  std::vector<Rect>::iterator iRect;
  updateContainer->changedRegion.get_rects(&rects);
  int numRects = updateContainer->changedRegion.numRects();

  // Grabbing
  for (iRect = rects.begin(); iRect < rects.end(); iRect++) {
    if (!m_screenGrabber->grab(&(*iRect))) {
      m_frameBufferCriticalSection->leave();
      return;
    }
  }

  Rect *rect;
  for (iRect = rects.begin(); iRect < rects.end(); iRect++) {
    rect = &(*iRect);

    // FIXME: Here should be cutting rect edges
    if (!m_frameBuffer->cmpFrom(rect, screenFrameBuffer, rect->left, rect->top)) {
      tmpChangedRegion.addRect(*rect);
      m_frameBuffer->copyFrom(rect, screenFrameBuffer, rect->left, rect->top);
    }
  }

  updateContainer->changedRegion = tmpChangedRegion;

  m_frameBufferCriticalSection->leave();
}
