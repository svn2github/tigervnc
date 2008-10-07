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

#ifndef __UPDATELISTENER_H__
#define __UPDATELISTENER_H__

#include "CriticalSection.h"

class UpdateListener
{
public:
  UpdateListener(void);
  virtual ~UpdateListener(void);

  // Interface function
  virtual void synchroOnUpdate(void *pSender)
  {
    m_updateListenerCriticalSection.enter();
    onUpdate(pSender);
    m_updateListenerCriticalSection.leave();
  }

protected:
  virtual void onUpdate(void *pSender) = 0;

  CriticalSection m_updateListenerCriticalSection;
};

#endif // __UPDATELISTENER_H__