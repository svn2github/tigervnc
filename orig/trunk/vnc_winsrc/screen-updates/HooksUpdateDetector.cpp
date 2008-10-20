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
#include "region/Rect.h"

// Constants
const UINT RFB_SCREEN_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.DrawRect"));
const UINT RFB_COPYRECT_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.CopyRect"));
const UINT RFB_MOUSE_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.Mouse"));

HooksUpdateDetector::HooksUpdateDetector(UpdateKeeper *updateKeeper,
                                         ScreenGrabber *screenGrabber,
                                         CriticalSection *updateKeeperCriticalSection)
: UpdateDetector(updateKeeper),
m_screenGrabber(screenGrabber),
m_updateKeeperCriticalSection(updateKeeperCriticalSection),
m_hooksTargetWindow(0)
{
}

HooksUpdateDetector::~HooksUpdateDetector(void)
{
  if (m_hHooks != 0) {
    if (m_hooksTargetWindow->getHWND() != 0) {
      m_pUnSetHook(m_hooksTargetWindow->getHWND());
    }
    ::FreeLibrary(m_hHooks);
  }
}

void HooksUpdateDetector::onTerminate()
{
  if (m_hooksTargetWindow != 0) {
    PostMessage(m_hooksTargetWindow->getHWND(), WM_QUIT, 0, 0);
  }
}

void HooksUpdateDetector::execute()
{
  // Dll initializing
  if ((m_hHooks = ::LoadLibrary(_T(LIBRARY_NAME))) == 0) {
    return;
  }

  m_pSetHook = (PSetHook)::GetProcAddress(m_hHooks, SET_HOOK_FUNCTION_NAME);
  m_pUnSetHook = (PUnSetHook)::GetProcAddress(m_hHooks, UNSET_HOOK_FUNCTION_NAME);
  if (!m_pSetHook || !m_pUnSetHook)
  {
    return;
  }

  HINSTANCE hinst = GetModuleHandle(0);

  m_hooksTargetWindow = new HooksTargetWindow(hinst);
  if (!m_hooksTargetWindow->createWindow()) {
    return;
  }

  // Hooks initializing
  if (m_pSetHook(m_hooksTargetWindow->getHWND(),
             RFB_SCREEN_UPDATE,
             RFB_COPYRECT_UPDATE,
             RFB_MOUSE_UPDATE) == FALSE) {
    return;
  }

  MSG msg;
  Rect screenRect;
  while (!m_terminated) {
    if (!PeekMessage(&msg, m_hooksTargetWindow->getHWND(), NULL, NULL, PM_REMOVE)) {
      //Sleep(1);
      if (!WaitMessage()) {
        break;
      }
    } else if (msg.message == RFB_SCREEN_UPDATE) {
      Rect rect;
      rect.left = (SHORT)LOWORD(msg.wParam);
      rect.top = (SHORT)HIWORD(msg.wParam);
      rect.right = (SHORT)LOWORD(msg.lParam);
      rect.bottom = (SHORT)HIWORD(msg.lParam);

      m_updateKeeperCriticalSection->enter();
      screenRect = m_screenGrabber->getScreenRect();
      rect.move(-screenRect.left, -screenRect.top);
      m_updateKeeper->addChangedRect(&rect);
      m_updateKeeperCriticalSection->leave();

      doOutUpdate();
    } else if (msg.message == WM_QUIT) {
      Sleep(1);
      break;
    } else {
      DispatchMessage(&msg);
    }
  }
}
