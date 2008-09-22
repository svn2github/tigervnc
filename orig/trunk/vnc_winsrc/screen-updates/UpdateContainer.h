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

#ifndef __UPDATECONTAINER_H__
#define __UPDATECONTAINER_H__

#include "Region.h"
#include "UpdateFilter.h"

typedef struct _tagUpdates
{
  rfb::Region copiedRegion;
  rfb::Region changedRegion;
  bool screenSizeChanged;
  bool cursorPosChanged;
  int copyOffsetX;
  int copyOffsetY;
} Updates;

class UpdateContainer
{
public:
  UpdateContainer(UpdateFilter *updateFilter);
  ~UpdateContainer(void);

  void addChangedRegion(rfb::Region *changedRegion);
  void addCopyRegion();
  void setScreenSizeChanged();
  void setCursorPosChanged();

  void extract(Updates *output);

private:
  UpdateFilter *m_updateFilter;

  Updates m_updates;
};

#endif // __UPDATECONTAINER_H__
