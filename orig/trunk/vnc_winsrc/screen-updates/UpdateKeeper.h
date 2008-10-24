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

#ifndef __UPDATEKEEPER_H__
#define __UPDATEKEEPER_H__

#include "region/Region.h"
#include "region/Point.h"
#include "UpdateFilter.h"
#include "UpdateContainer.h"

class UpdateKeeper
{
public:
  UpdateKeeper(UpdateFilter *updateFilter, const FrameBuffer *frameBuffer);
  ~UpdateKeeper(void);

  void addChangedRegion(rfb::Region *changedRegion);
  void addChangedRect(const Rect *changedRect);

  void addCopyRegion(rfb::Region *cpyReg, const Point *copyOffset);

  void setScreenSizeChanged();
  void setCursorPosChanged();

  void setExcludedRegion(rfb::Region *excludedRegion);

  const UpdateContainer *getUpdateContainer() const { return &m_updateContainer; }

  void extract(UpdateContainer *updateContainer);

private:
  UpdateFilter *m_updateFilter;

  // For getDimension() only
  const FrameBuffer *m_frameBuffer;
  Rect m_borderRect;

  rfb::Region m_excludedRegion;
  CriticalSection m_exclRegCritSec;

  UpdateContainer m_updateContainer;
};

#endif // __UPDATEKEEPER_H__
