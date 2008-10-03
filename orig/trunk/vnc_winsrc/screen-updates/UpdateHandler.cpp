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
: m_outUpdateListener(0)
{
  m_criticalSection = new CriticalSection;
  m_screenGrabber = new WindowsScreenGrabber;
  m_backupFrameBuffer = new FrameBuffer;
  m_updateFilter = new UpdateFilter(m_screenGrabber, m_backupFrameBuffer,
                                    m_criticalSection);
  m_updateKeeper = new UpdateKeeper(m_updateFilter);
  m_poller = new Poller(m_updateKeeper, m_screenGrabber,
                                m_backupFrameBuffer, m_criticalSection);
  m_poller->setOutUpdateListener(this);
}

UpdateHandler::~UpdateHandler(void)
{
  terminate();
  delete m_updateKeeper;
  delete m_updateFilter;
  delete m_screenGrabber;
  delete m_backupFrameBuffer;
}

void UpdateHandler::extract(UpdateContainer *updateContainer)
{
  m_criticalSection->enter();

  m_updateKeeper->extract(&m_updateContainer);

  // Checking for screen properties changing or frame buffers differ
  if (m_screenGrabber->getPropertiesChanged() ||
      !m_backupFrameBuffer->cmp(m_screenGrabber->getScreenBuffer())) {
    if (m_screenGrabber->getScreenSizeChanged()) {
      m_updateContainer.screenSizeChanged = true;
    }
    m_screenGrabber->applyNewProperties();
    m_backupFrameBuffer->assignProperties(m_screenGrabber->getScreenBuffer());
  }

  *updateContainer = m_updateContainer;

  m_criticalSection->leave();
}

void UpdateHandler::execute()
{
  m_poller->resume();
}

void UpdateHandler::terminate()
{
  m_poller->terminate();
  m_poller->wait();
}

void UpdateHandler::onUpdate(void *pSender)
{
  m_criticalSection->enter();

  if (!m_updateKeeper->getUpdateContainer()->isEmpty()) {
    m_criticalSection->leave();
    doOutUpdate();
    return;
  }

  m_criticalSection->leave();
}
