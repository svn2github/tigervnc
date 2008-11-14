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

void UpdateKeeper::addChangedRegion(const rfb::Region *changedRegion)
{
  AutoLock al(&m_updContCritSec);

  m_updateContainer.copiedRegion.assign_subtract(*changedRegion);
  m_updateContainer.changedRegion.assign_union(*changedRegion);

  rfb::Region borderRegion(m_borderRect);
  m_updateContainer.changedRegion.assign_intersect(borderRegion);
}

void UpdateKeeper::addChangedRect(const Rect *changedRect)
{
  rfb::Region region(changedRect);
  addChangedRegion(&region);
}

void UpdateKeeper::addCopyRect(const Rect *copyRect, const Point *src)
{
  AutoLock al(&m_updContCritSec);

  if ((src->x == 0) && (src->y == 0) || copyRect->isEmpty()) {
    return;
  }

  m_updateContainer.copySrc = *src;

  rfb::Region *changedRegion = &m_updateContainer.changedRegion;
  rfb::Region *copiedRegion = &m_updateContainer.copiedRegion;

  // Old copiedRegion must be added to changedRegion - (?)
  if (!copiedRegion->is_empty()) {
    changedRegion->assign_union(*copiedRegion);
    copiedRegion->clear();
    addChangedRect(copyRect);
    return;
  }

  copiedRegion->clear();
  copiedRegion->addRect(*copyRect);

  // copiedRegion must be substracted from changedRegion
  changedRegion->assign_subtract(*copiedRegion);

  // Create copy of copyRect in the source coordinates.
  Rect srcCopyRect(copyRect);
  srcCopyRect.setLocation(src->x, src->y);

  // Create region that is intersection of changedRegion and srcCopyRect.
  rfb::Region addonChangedRegion;
  addonChangedRegion.addRect(srcCopyRect);
  addonChangedRegion.assign_intersect(*changedRegion);

  // Move addonChangedRegion and add it to changedRegion.
  addonChangedRegion.move(copyRect->left - src->x, copyRect->top - src->y);
  changedRegion->assign_union(addonChangedRegion);

  // Clipping regions
  rfb::Region borderRegion(m_borderRect);
  m_updateContainer.changedRegion.assign_intersect(borderRegion);
  m_updateContainer.copiedRegion.assign_intersect(borderRegion);
}

void UpdateKeeper::setScreenSizeChanged()
{
  AutoLock al(&m_updContCritSec);

  // Reinitializing m_borderRect
  m_borderRect.setRect(&m_frameBuffer->getDimension().getRect());

  m_updateContainer.screenSizeChanged = true;
}

void UpdateKeeper::setCursorPosChanged()
{
  AutoLock al(&m_updContCritSec);
  m_updateContainer.cursorPosChanged = true;
}

void UpdateKeeper::extract(UpdateContainer *updateContainer)
{
  {
    Rect copyRect;
    Point copySrc;
    m_copyRectDetector.detectWindowMovements(&copyRect, &copySrc);
    addCopyRect(&copyRect, &copySrc);

    AutoLock al(&m_updContCritSec);

    *updateContainer = m_updateContainer;
    m_updateContainer.clear();
  }
  {
    AutoLock al(&m_exclRegCritSec);
    updateContainer->changedRegion.assign_subtract(m_excludedRegion);
    updateContainer->copiedRegion.assign_subtract(m_excludedRegion);
  }
  m_updateFilter->filter(updateContainer);
}

void UpdateKeeper::setExcludedRegion(const rfb::Region *excludedRegion)
{
  AutoLock al(&m_exclRegCritSec);

  if(excludedRegion == 0) {
    m_excludedRegion.clear();
  } else {
    m_excludedRegion = *excludedRegion;
  }
}
