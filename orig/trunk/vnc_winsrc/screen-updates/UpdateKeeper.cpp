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

UpdateKeeper::UpdateKeeper(UpdateFilter *updateFilter)
: m_updateFilter(updateFilter)
{
}

UpdateKeeper::~UpdateKeeper(void)
{
}

void UpdateKeeper::addChangedRegion(rfb::Region *changedRegion)
{
  m_updateContainer.changedRegion.assign_union(*changedRegion);
}

void UpdateKeeper::addCopyRegion()
{
}

void UpdateKeeper::setScreenSizeChanged()
{
  m_updateContainer.screenSizeChanged = true;
}

void UpdateKeeper::setCursorPosChanged()
{
  m_updateContainer.cursorPosChanged = true;
}

void UpdateKeeper::extract(UpdateContainer *updateContainer)
{
  *updateContainer = m_updateContainer;

  // Clear all changes
  m_updateContainer.changedRegion.clear();
  m_updateContainer.copiedRegion.clear();
  m_updateContainer.cursorPosChanged = false;
  m_updateContainer.screenSizeChanged = false;
  m_updateContainer.copyOffsetX = 0;
  m_updateContainer.copyOffsetY = 0;
}
