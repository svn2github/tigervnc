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
#include "Poller.h"
#include "HooksUpdateDetector.h"
#include "MouseDetector.h"

UpdateHandler::UpdateHandler(void)
: m_outUpdateListener(0)
{
  m_criticalSection = new CriticalSection;
  m_screenGrabber = new WindowsScreenGrabber;
  m_backupFrameBuffer = new FrameBuffer;
  m_updateFilter = new UpdateFilter(m_screenGrabber, m_backupFrameBuffer,
                                    m_criticalSection);
  m_updateKeeper = new UpdateKeeper(m_updateFilter,
                                    m_screenGrabber->getScreenBuffer());
  m_poller = new Poller(m_updateKeeper, m_screenGrabber,
                                m_backupFrameBuffer, m_criticalSection);
  m_poller->setOutUpdateListener(this);
  m_hooks = new HooksUpdateDetector(m_updateKeeper,
                                    m_screenGrabber,
                                    m_criticalSection);
  m_hooks->setOutUpdateListener(this);
  m_mouseDetector = new MouseDetector(m_updateKeeper,
                                      m_criticalSection);
  m_mouseDetector->setOutUpdateListener(this);
}

UpdateHandler::~UpdateHandler(void)
{
  terminate();
  delete m_mouseDetector;
  delete m_poller;
  delete m_hooks;
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
  m_backupFrameBuffer->assignProperties(m_screenGrabber->getScreenBuffer());
  m_poller->resume();
  m_hooks->resume();
  m_mouseDetector->resume();
}

void UpdateHandler::terminate()
{
  m_poller->terminate();
  m_hooks->terminate();
  m_mouseDetector->terminate();
  m_poller->wait();
  m_hooks->wait();
  m_mouseDetector->wait();
}

void UpdateHandler::onUpdate()
{
  m_criticalSection->enter();

  if (!m_updateKeeper->getUpdateContainer()->isEmpty()) {
    m_criticalSection->leave();
    doOutUpdate();
    return;
  }

  m_criticalSection->leave();
}

bool UpdateHandler::checkForUpdates(rfb::Region *region)
{
  AutoLock aL(m_criticalSection);
  bool result = !m_updateKeeper->getUpdateContainer()->isEmpty();

  return result;
}

void UpdateHandler::setExcludedRegion(const rfb::Region *excludedRegion)
{
  m_updateKeeper->setExcludedRegion(excludedRegion);
}
