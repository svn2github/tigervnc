//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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

#include "WinDesktop.h"
#include "libscreen/DesktopSelector.h"
#include "thread/time.h"
#include <TCHAR.h>

WinDesktop::WinDesktop()
: m_updateHandler(0),
  m_inputBlocker(0),
  m_server(0),
  m_hEvent(0)
{
  DesktopSelector::init();

  m_inputBlocker = new InputBlocker;
}

WinDesktop::~WinDesktop()
{
  terminate();
  wait();

  if (m_updateHandler != 0) {
    delete m_updateHandler;
  }

  if (m_inputBlocker != 0) {
    SetLocalInputDisableHook(false);
    delete m_inputBlocker;
  }
}

bool WinDesktop::Init(vncServer *server)
{
  m_server = server;

  setNewScreenSize();

  m_updateHandler = new UpdateHandler(this);
  if (m_updateHandler == 0) {
    return false;
  }

  m_pixelFormat = m_updateHandler->getFrameBuffer()->getPixelFormat();

  SetLocalInputDisableHook(m_server->LocalInputsDisabled() != 0);

  RECT rect;
  if (m_server->WindowShared()) {
    getWindowRect(m_server->GetWindowShared(), &rect);
  } else if (m_server->ScreenAreaShared()) {
    rect = m_server->GetScreenAreaRect();
  } else {
    rect = m_bmrect;
  }

  IntersectRect(&rect, &rect, &m_bmrect);
  m_server->SetSharedRect(rect);

  resume();

  return true;
}

void WinDesktop::onTerminate()
{
  winDesktopNotify();
}

void WinDesktop::execute()
{
  m_hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
  if (m_hEvent == 0) {
    return;
  }

  while(!m_terminated) {
    WaitForSingleObject(m_hEvent, INFINITE);
    if (!m_terminated) {
      sendUpdate();
    }
  }

  CloseHandle(m_hEvent);
  m_hEvent = 0;
}

void WinDesktop::onUpdate()
{
  winDesktopNotify();
}

void WinDesktop::RequestUpdate()
{
  winDesktopNotify();
}

void WinDesktop::SetClipText(LPSTR text)
{
}

void WinDesktop::TryActivateHooks()
{
}

void WinDesktop::FillDisplayInfo(rfbServerInitMsg *scrInfo)
{
  const FrameBuffer *fb = m_updateHandler->getFrameBuffer();
  m_pixelFormat = fb->getPixelFormat();

  scrInfo->format.bigEndian     = m_pixelFormat.bigEndian;
  scrInfo->format.bitsPerPixel  = (CARD8)m_pixelFormat.bitsPerPixel;
  scrInfo->format.redMax        = m_pixelFormat.redMax;
  scrInfo->format.redShift      = (CARD8)m_pixelFormat.redShift;
  scrInfo->format.greenMax      = m_pixelFormat.greenMax;
  scrInfo->format.greenShift    = (CARD8)m_pixelFormat.greenShift;
  scrInfo->format.blueMax       = m_pixelFormat.blueMax;
  scrInfo->format.blueShift     = (CARD8)m_pixelFormat.blueShift;
  scrInfo->format.depth         = (CARD8)m_pixelFormat.colorDepth;
  scrInfo->format.trueColour    = 1;
  scrInfo->format.pad1          = 0;
  scrInfo->format.pad2          = 0;

  scrInfo->framebufferWidth = fb->getDimension().width;
  scrInfo->framebufferHeight = fb->getDimension().height;
}

void WinDesktop::SetLocalInputDisableHook(bool block)
{
  m_inputBlocker->setKeyboardBlocking(block);
  m_inputBlocker->setMouseBlocking(block);
}

void WinDesktop::SetLocalInputPriorityHook(BOOL enable)
{
}

BYTE *WinDesktop::MainBuffer()
{
  return (BYTE *)m_updateHandler->getFrameBuffer()->getBuffer();
}

int WinDesktop::ScreenBuffSize()
{
  return m_updateHandler->getFrameBuffer()->getBufferSize();
}

bool WinDesktop::sendUpdate()
{
  bool fullUpdateRequest = m_server->FullRgnRequested() != FALSE;
  bool incrUpdateRequest = m_server->IncrRgnRequested() != FALSE;

  if (!incrUpdateRequest && !fullUpdateRequest) {
    return true;
  }

  bool desktopChanged = false;
  checkCurrentDesktop(&desktopChanged);

  bool sharedRectChanged = m_server->updateSharedRect();

  rfb::Region fullRgnReq;
  m_server->getFullRgnRequested(&fullRgnReq);

  RECT r = m_server->GetSharedRect();
  Rect sharedRect(r.left, r.top, r.right, r.bottom);
  rfb::Region sharedReg(&sharedRect);

  if (fullUpdateRequest) {
    m_updateHandler->setFullUpdateRequested(&fullRgnReq);
  }

  if (sharedRectChanged) {
    // sharedReg always contains fullRgnReq.
    m_updateHandler->setFullUpdateRequested(&sharedReg);
  }

  rfb::Region videoRegion;
  m_server->getVideoRegion(&videoRegion);
  m_updateHandler->setFullUpdateRequested(&videoRegion);

  UpdateContainer updateContainer;
  m_updateHandler->extract(&updateContainer);

  if (sharedRectChanged) {
    updateContainer.changedRegion.assign_union(sharedReg);
  }

  updateContainer.changedRegion.assign_intersect(sharedReg);

  if (desktopChanged) {
    setNewScreenSize();
    updateBufferNotify();
  }

  if (updateContainer.cursorPosChanged) {
    m_server->UpdateMouse();
  }

  if (updateContainer.screenSizeChanged) {
    setNewScreenSize();
    updateBufferNotify();
    return true;
  }

  // Checking for PixelFormat changing
  if (!m_pixelFormat.cmp(&m_updateHandler->getFrameBuffer()->getPixelFormat())) {
    updateBufferNotify();
  }

  if (updateContainer.cursorShapeChanged) {
    m_server->UpdateMouseShape();
  }

  // FIXME: Change m_server->CopyRect(Rect *) to m_server->copyRegion(Region *)
  std::vector<Rect> rects;
  std::vector<Rect>::iterator iRect;
  updateContainer.copiedRegion.get_rects(&rects);
  int numRects = updateContainer.copiedRegion.numRects();
  if (numRects > 0) {
    iRect = rects.begin();
    RECT rect;
    rect.left   = (*iRect).left;
    rect.top    = (*iRect).top;
    rect.right  = (*iRect).right;
    rect.bottom = (*iRect).bottom;
    POINT copySrc;
    copySrc.x = updateContainer.copySrc.x;
    copySrc.y = updateContainer.copySrc.y;
    m_server->CopyRect(rect, copySrc);
  }

  numRects = updateContainer.changedRegion.numRects();

  if (numRects > 0) {
    vncRegion changedRegion;
    changedRegion.assignFromNewFormat(&updateContainer.changedRegion);
    m_server->UpdateRegion(changedRegion);
  }

  m_server->TriggerUpdate(m_updateHandler->getFrameBuffer());

  return true;
}

void WinDesktop::setNewScreenSize()
{
  m_bmrect.left   = 0;
  m_bmrect.top    = 0;
  m_bmrect.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  m_bmrect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

BOOL WinDesktop::getWindowRect(HWND hwnd, RECT *windowRect)
{
  BOOL result = GetWindowRect(hwnd, windowRect);
  if (result) {
    OffsetRect(windowRect,
               -GetSystemMetrics(SM_XVIRTUALSCREEN),
               -GetSystemMetrics(SM_YVIRTUALSCREEN));
  }
  return result;
}

void WinDesktop::updateBufferNotify()
{
  m_server->UpdateLocalFormat();
  m_server->UpdatePalette();
}

bool WinDesktop::checkCurrentDesktop(bool *changed)
{
  *changed = false;

  if (DesktopSelector::getDesktopChanging()) {
    if (!DesktopSelector::selectDesktop()) {
      vnclog.Print(LL_INTERR, VNCLOG("cannot select desktop\n"));
      return false;
    }

    // Restarting UpdateHandler;
    if (m_updateHandler != 0) {
      delete m_updateHandler;
    }
    m_updateHandler = new UpdateHandler(this);

    // Restarting InputBlocker;
    if(m_inputBlocker != 0) {
      delete m_inputBlocker;
    }
    m_inputBlocker = new InputBlocker;
    
    *changed = true;
  }

  return true;
}
