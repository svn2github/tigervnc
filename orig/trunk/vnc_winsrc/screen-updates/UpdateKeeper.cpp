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

  rfb::Region *copiedRegion = &m_updateContainer.copiedRegion;
  rfb::Region movedRegion(*copiedRegion);

  Point *copyOffset = &m_updateContainer.copyOffset;
  movedRegion.move(copyOffset);
  movedRegion.assign_subtract(*changedRegion);
  copyOffset->x = -copyOffset->x;
  copyOffset->y = -copyOffset->y;
  movedRegion.move(copyOffset);
  *copiedRegion = movedRegion;

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
  rfb::Region movedRegion(*copiedRegion);
  movedRegion.move(&m_updateContainer.copyOffset);
  changedRegion->assign_union(movedRegion);

  // Create new copiedRegion
  cpyReg->assign_subtract(*changedRegion);

  movedRegion = *cpyReg;
  movedRegion.move(copyOffset);

  changedRegion->assign_subtract(movedRegion);

  copiedRegion = cpyReg;
  m_updateContainer.copyOffset = *copyOffset;
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
  m_updateFilter->filter(&m_updateContainer);

  *updateContainer = m_updateContainer;

  // Clear all changes
  m_updateContainer.clear();
}
