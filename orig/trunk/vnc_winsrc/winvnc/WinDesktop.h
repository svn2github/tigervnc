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

#ifndef __WINDESKTOP_H__
#define __WINDESKTOP_H__

class WinDesktop;

#include "stdhdrs.h"

#include "vncServer.h"
#include "translate.h"

#include "screen-updates/UpdateHandler.h"
#include "screen-updates/UpdateListener.h"
#include "thread/AutoLock.h"

class WinDesktop : public UpdateListener
{
public:
  WinDesktop();
  virtual ~WinDesktop();

  bool Init(vncServer *server);
  void RequestUpdate();
  void SetClipText(LPSTR text);
  void TryActivateHooks();
  void FillDisplayInfo(rfbServerInitMsg *scrInfo);
  void SetLocalInputDisableHook(BOOL enable);
  void SetLocalInputPriorityHook(BOOL enable);
  BYTE *MainBuffer();
  int ScreenBuffSize();
  void CaptureScreen(RECT &UpdateArea, BYTE *scrBuff); // could be protected?
  void CaptureMouse(BYTE *scrBuff, UINT scrBuffSize);
  RECT MouseRect();

  HCURSOR GetCursor() const;
  BOOL GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height);

protected:
  virtual void onUpdate(void *pSender);

  bool sendUpdate();

  UpdateHandler *m_updateHandler;
  vncServer *m_server;
};

#endif // __WINDESKTOP_H__
