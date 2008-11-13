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

#define HOOK_LIBRARY_NAME "ScreenHooks.dll"
#define SET_KEYBOARD_FILTER_FUN_NAME "SetKeyboardFilterHook"
#define SET_MOUSE_FILTER_FUN_NAME "SetMouseFilterHook"

typedef int (*SetKeyboardFilterHook)(BOOL activate);
typedef int (*SetMouseFilterHook)(BOOL activate);

WinDesktop::WinDesktop()
: m_updateHandler(0),
  m_server(0),
  m_hEvent(0)
{
  DesktopSelector::init();

  // Loading dynamic library
  m_dynamicLibrary = new DynamicLibrary(_T(HOOK_LIBRARY_NAME));

  m_setKeyboardFilterHook = m_dynamicLibrary->getProcAddress(_T(SET_KEYBOARD_FILTER_FUN_NAME));
  m_setMouseFilterHook = m_dynamicLibrary->getProcAddress(_T(SET_MOUSE_FILTER_FUN_NAME));
  if (!m_setKeyboardFilterHook || !m_setMouseFilterHook) {
    vnclog.Print(LL_INTERR, VNCLOG("cannot initialize hooks\n"));
  }
}

WinDesktop::~WinDesktop()
{
  terminate();
  wait();

  if (m_updateHandler != 0) {
    delete m_updateHandler;
    m_updateHandler = 0;
  }

  SetLocalInputDisableHook(false);

  if (m_dynamicLibrary != 0) {
    delete m_dynamicLibrary;
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

  SetLocalInputDisableHook(m_server->LocalInputsDisabled());

  RECT rect;
  if (m_server->WindowShared()) {
    GetWindowRect(m_server->GetWindowShared(), &rect);
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

void WinDesktop::SetLocalInputDisableHook(BOOL enable)
{
  SetKeyboardFilterHook setKeyboardFH = (SetKeyboardFilterHook)m_setKeyboardFilterHook;
  SetMouseFilterHook setMouseFH = (SetMouseFilterHook)m_setMouseFilterHook;

  if (setKeyboardFH && setMouseFH) {
    setKeyboardFH(enable);
    setMouseFH(enable);
  }
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

void WinDesktop::CaptureScreen(RECT &UpdateArea, BYTE *scrBuff)
{
}

void WinDesktop::CaptureMouse(BYTE *scrBuff, UINT scrBuffSize)
{
}

RECT WinDesktop::MouseRect()
{
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = 0;
  rect.bottom = 0;
  return rect;
}

HCURSOR WinDesktop::GetCursor() const
{
  CURSORINFO cursorInfo;
  cursorInfo.cbSize = sizeof(CURSORINFO);
  int result = GetCursorInfo(&cursorInfo);

  if (result == 0) {
  }

  HCURSOR hCursor = cursorInfo.hCursor;
  if (hCursor  == NULL) {
    return NULL;
  }

  return hCursor;
}

BOOL WinDesktop::GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height)
{
  PixelFormat pixelFormat = m_updateHandler->getFrameBuffer()->getPixelFormat();

  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  WindowsScreenGrabber::BMI bmi;
  if (!WindowsScreenGrabber::getBMI(&bmi, screenDC)) {
    return false;
  }

  bmi.bmiHeader.biBitCount = pixelFormat.bitsPerPixel;
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = pixelFormat.redMax   << pixelFormat.redShift;
  bmi.green = pixelFormat.greenMax << pixelFormat.greenShift;
  bmi.blue  = pixelFormat.blueMax  << pixelFormat.blueShift;

  HDC destDC = CreateCompatibleDC(NULL);
  if (destDC == NULL) {
    DeleteDC(screenDC);
    return FALSE;
  }

  void *buffer;
  HBITMAP hbmDIB = CreateDIBSection(destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &buffer, NULL, NULL);
  if (hbmDIB == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return FALSE;
  }

  HBITMAP hbmOld = (HBITMAP) SelectObject(destDC, hbmDIB);

  memset(buffer, 0xff, width * height * 4);
  // Draw the cursor
  int result = DrawIconEx(destDC, 0, 0, hcursor, 0, 0, 0, NULL, DI_IMAGE);

  if (result) {
    memcpy(databuf, buffer, width * height * 4);
  }

  SelectObject(destDC, hbmOld);
  DeleteObject(hbmDIB);
  DeleteDC(destDC);
  DeleteDC(screenDC);

  return result;
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

  int numRects = updateContainer.changedRegion.numRects();

  if (numRects > 0) {
    vncRegion changedRegion;
    changedRegion.assignFromNewFormat(&updateContainer.changedRegion);
    m_server->UpdateRegion(changedRegion);
  }

  // FIXME: Change m_server->CopyRect(Rect *) to m_server->copyRegion(Region *)
  std::vector<Rect> rects;
  std::vector<Rect>::iterator iRect;
  updateContainer.copiedRegion.get_rects(&rects);
  numRects = updateContainer.copiedRegion.numRects();
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

  m_server->TriggerUpdate();

  return true;
}



void WinDesktop::setNewScreenSize()
{
  m_bmrect.left   = 0;
  m_bmrect.top    = 0;
  m_bmrect.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  m_bmrect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
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

    *changed = true;
  }

  return true;
}
