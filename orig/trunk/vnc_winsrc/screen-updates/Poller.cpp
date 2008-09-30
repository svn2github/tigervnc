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

#include <windows.h>
#include "Poller.h"
#include "Region.h"

Poller::Poller(UpdateKeeper *updateKeeper,
               ScreenGrabber *screenGrabber,
               FrameBuffer *backupFrameBuffer,
               CriticalSection *frameBufferCriticalSection)
: UpdateDetector(updateKeeper),
m_screenGrabber(screenGrabber),
m_backupFrameBuffer(backupFrameBuffer),
m_frameBufferCriticalSection(frameBufferCriticalSection)
{
  m_pollingRect.setRect(0, 0, 16, 16);
}

Poller::~Poller(void)
{
}

void Poller::execute()
{
  FrameBuffer *screenFrameBuffer;

  while (!m_terminated) {
    rfb::Region region;

    screenFrameBuffer = m_screenGrabber->getScreenBuffer();
    if (!screenFrameBuffer->cmp(m_backupFrameBuffer)) {
      m_updateKeeper->setScreenSizeChanged();
      continue;
    }

    m_screenGrabber->grab();

    // Polling
    int pollingWidth = m_pollingRect.getWidth();
    int pollingHeight = m_pollingRect.getHeight();
    int screenWidth = screenFrameBuffer->getDimension().width;
    int screenHeight = screenFrameBuffer->getDimension().height;

    Rect scanRect;
    for (int iRow = 0; iRow < screenHeight; iRow += pollingHeight) {
      for (int iCol = 0; iCol < screenWidth; iCol += pollingWidth) {
        scanRect.setRect(iCol, iRow, min(iCol + pollingWidth, screenWidth),
                         min(iRow + pollingHeight, screenHeight));
        if (!cmpFrameBuff(&scanRect, screenFrameBuffer, m_backupFrameBuffer)) {
          region.addRect(scanRect);
        }
      }
    }

    m_updateKeeper->addChangedRegion(&region);
    Sleep(1000);
  }
}

bool Poller::cmpFrameBuff(const Rect *rect, const FrameBuffer *fb1,
                          const FrameBuffer *fb2)
{
  UINT32 pixelSize = m_backupFrameBuffer->getPixelFormat().bitsPerPixel / 8;
  UINT32 strike = m_backupFrameBuffer->getDimension().width;
  UINT32 pLine = (rect->top * strike + rect->left) * pixelSize;
  UINT8 *buf1 = (UINT8 *)fb1->getBuffer();
  UINT8 *buf2 = (UINT8 *)fb2->getBuffer();

  for (int i = 0; i < rect->getHeight(); pLine += strike, i++) {
    if (!memcmp(buf1 + pLine, buf2 + pLine, pixelSize * rect->getWidth())) {
      return false;
    }
  }
  return true;
}
