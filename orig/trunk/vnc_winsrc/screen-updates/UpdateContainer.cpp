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

#include "UpdateContainer.h"

UpdateContainer::UpdateContainer(void)
{
}

UpdateContainer::~UpdateContainer(void)
{
}

void UpdateContainer::addChangedRegion(rfb::Region *changedRegion)
{
  m_updates.changedRegion.assign_union(*changedRegion);
}

void UpdateContainer::addCopyRegion()
{
}

void UpdateContainer::setScreenSizeChanged()
{
  m_updates.screenSizeChanged = true;
}

void UpdateContainer::setCursorPosChanged()
{
  m_updates.cursorPosChanged = true;
}

void UpdateContainer::extract(Updates *output)
{
  *output = m_updates;

  // Clear all changes
  m_updates.changedRegion.clear();
  m_updates.copiedRegion.clear();
  m_updates.cursorPosChanged = false;
  m_updates.screenSizeChanged = false;
  m_updates.copyOffsetX = 0;
  m_updates.copyOffsetY = 0;
}
