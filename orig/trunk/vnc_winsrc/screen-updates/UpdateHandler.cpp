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

#include "UpdateHandler.h"

UpdateHandler::UpdateHandler(void)
{
  m_screenGrabber = new WindowsScreenGrabber;
  m_frameBuffer = new FrameBuffer;
  m_updateFilter = new UpdateFilter(m_screenGrabber, m_frameBuffer);
  m_updateKeeper = new UpdateKeeper(m_updateFilter);
  m_criticalSection = new CriticalSection;
  m_updateDetector = new Poller(m_updateKeeper, m_screenGrabber,
                                m_frameBuffer, m_criticalSection);
}

UpdateHandler::~UpdateHandler(void)
{
  terminate();
  delete m_updateKeeper;
  delete m_updateFilter;
  delete m_screenGrabber;
  delete m_frameBuffer;
}

void UpdateHandler::extract(UpdateContainer *updateContainer)
{
  m_criticalSection->enter();
  m_updateKeeper->extract(&m_updateContainer);

  // Checking for ScreenGrabber properties have been changed
  if (m_screenGrabber->getPropertiesChanged()) {
    m_screenGrabber->applyNewProperties();
    m_frameBuffer->setPixelFormat(&m_screenGrabber->getScreenBuffer()->getPixelFormat(),
      false);
    m_frameBuffer->setDimension(&m_screenGrabber->getScreenBuffer()->getDimension());
  }

  // FIXME: There should be a filtering of region

  *updateContainer = m_updateContainer;
  m_criticalSection->leave();
}

void UpdateHandler::execute()
{
  m_updateDetector->resume();
}

void UpdateHandler::terminate()
{
  m_updateDetector->terminate();
}
