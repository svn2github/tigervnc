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

#include "UpdateKeeper.h"
#include "thread/AutoLock.h"

UpdateKeeper::UpdateKeeper(UpdateFilter *updateFilter, const FrameBuffer *frameBuffer)
: m_updateFilter(updateFilter),
m_frameBuffer(frameBuffer)
{
  m_borderRect.setRect(&m_frameBuffer->getDimension().getRect());
}

UpdateKeeper::~UpdateKeeper(void)
{
}

void UpdateKeeper::addChangedRegion(rfb::Region *changedRegion)
{
  rfb::Region borderRegion(m_borderRect);
  changedRegion->assign_intersect(borderRegion);

  m_updateContainer.copiedRegion.assign_subtract(*changedRegion);

  m_updateContainer.changedRegion.assign_union(*changedRegion);
}

void UpdateKeeper::addChangedRect(const Rect *changedRect)
{
  rfb::Region region(changedRect);
  addChangedRegion(&region);
}

void UpdateKeeper::addCopyRegion(rfb::Region *cpyReg, const Point *copyOffset)
{
  rfb::Region *changedRegion = &m_updateContainer.changedRegion;
  rfb::Region *copiedRegion = &m_updateContainer.copiedRegion;

  // Destroy previous copiedRegion
  changedRegion->assign_union(*copiedRegion);

  rfb::Region srcCopyRegion(*cpyReg);
  rfb::Region srcChangedRegion(*changedRegion);
  srcCopyRegion.move(copyOffset);

  // Create new copiedRegion
  srcChangedRegion.assign_intersect(srcCopyRegion);
  srcCopyRegion.assign_subtract(*changedRegion);

  *copiedRegion = srcCopyRegion;
  m_updateContainer.copyOffset = *copyOffset;
  copiedRegion->move(&copyOffset->getReverse());

  srcChangedRegion.move(&copyOffset->getReverse());
  changedRegion->assign_union(srcChangedRegion);
  changedRegion->assign_subtract(*copiedRegion);
}

void UpdateKeeper::setScreenSizeChanged()
{
  // Reinitializing m_borderRect
  m_borderRect.setRect(&m_frameBuffer->getDimension().getRect());

  m_updateContainer.screenSizeChanged = true;
}

void UpdateKeeper::setCursorPosChanged()
{
  m_updateContainer.cursorPosChanged = true;
}

void UpdateKeeper::extract(UpdateContainer *updateContainer)
{
  {
    AutoLock al(&m_exclRegCritSec);
    m_updateContainer.changedRegion.assign_subtract(m_excludedRegion);
    m_updateContainer.copiedRegion.assign_subtract(m_excludedRegion);
  }

  m_updateFilter->filter(&m_updateContainer);

  *updateContainer = m_updateContainer;

  // Clear all changes
  m_updateContainer.clear();
}

void UpdateKeeper::setExcludedRegion(rfb::Region *excludedRegion)
{
  AutoLock al(&m_exclRegCritSec);

  if(excludedRegion == 0) {
    m_excludedRegion.clear();
  } else {
    m_excludedRegion = *excludedRegion;
  }
}
