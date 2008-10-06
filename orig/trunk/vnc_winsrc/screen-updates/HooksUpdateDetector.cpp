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

#include "HooksUpdateDetector.h"
#include "TCHAR.h"

HooksUpdateDetector::HooksUpdateDetector(UpdateKeeper *updateKeeper,
                                         CriticalSection *updateKeeperCriticalSection)
: UpdateDetector(updateKeeper),
m_updateKeeperCriticalSection(updateKeeperCriticalSection)
{
  // Dll initializing
  if ((m_hHooks = ::LoadLibrary(_T(LIBRARY_NAME))) == 0) {
    m_terminated = true;
    return;
  }
}

HooksUpdateDetector::~HooksUpdateDetector(void)
{
  if (m_hHooks != 0) {
    ::FreeLibrary(m_hHooks);
  }
}

void HooksUpdateDetector::execute()
{
  while (!m_terminated) {
  }
}
