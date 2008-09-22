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

#ifndef __UPDATEHANDLER_H__
#define __UPDATEHANDLER_H__

#include "UpdateContainer.h"
#include "UpdateFilter.h"
#include "libscreen/WindowsScreenGrabber.h"
#include "libscreen/FrameBuffer.h"
#include "UpdateDetector.h"

class UpdateHandler
{
public:
  UpdateHandler(void);
  ~UpdateHandler(void);

private:
  UpdateContainer *m_updateContainer;
  UpdateFilter *m_updateFilter;
  UpdateDetector *m_updateDetector;
  ScreenGrabber *m_screenGrabber;
  FrameBuffer *m_frameBuffer;
};

#endif // __UPDATEHANDLER_H__
