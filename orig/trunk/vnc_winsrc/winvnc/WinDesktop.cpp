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

WinDesktop::WinDesktop()
: m_updateHandler(0),
  m_server(0),
  m_hEvent(0)
{
}

WinDesktop::~WinDesktop()
{
  terminate();
  wait();

  if (m_updateHandler != 0) {
    delete m_updateHandler;
    m_updateHandler = 0;
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
  PixelFormat pf = fb->getPixelFormat();

  scrInfo->format.bigEndian = pf.bigEndian;
  scrInfo->format.bitsPerPixel  = (CARD8)pf.bitsPerPixel;
  scrInfo->format.redMax        = pf.redMax;
  scrInfo->format.redShift      = (CARD8)pf.redShift;
  scrInfo->format.greenMax      = pf.greenMax;
  scrInfo->format.greenShift    = (CARD8)pf.greenShift;
  scrInfo->format.blueMax       = pf.blueMax;
  scrInfo->format.blueShift     = (CARD8)pf.blueShift;
  scrInfo->format.depth         = (CARD8)pf.colorDepth;
  scrInfo->format.trueColour    = 1;
  scrInfo->format.pad1          = 0;
  scrInfo->format.pad2          = 0;

  scrInfo->framebufferWidth = fb->getDimension().width;
  scrInfo->framebufferHeight = fb->getDimension().height;
}

void WinDesktop::SetLocalInputDisableHook(BOOL enable)
{
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
  if (!m_server->IncrRgnRequested() && !m_server->FullRgnRequested()) {
    return true;
  }

  UpdateContainer updateContainer;
  m_updateHandler->extract(&updateContainer);

  if (updateContainer.isEmpty()) {
    // Check for video area presence.
    rfb::Region videoRegion;
    m_server->getVideoRegion(&videoRegion);
    if (videoRegion.is_empty()) {
      return true;
    }
  } else {

    if (updateContainer.cursorPosChanged) {
      m_server->UpdateMouse();
    }

    if (updateContainer.screenSizeChanged) {
      setNewScreenSize();
      return true;
    }

    std::vector<Rect> rects;
    std::vector<Rect>::iterator iRect;
    updateContainer.changedRegion.get_rects(&rects);
    int numRects = updateContainer.changedRegion.numRects();

    if (numRects > 0) {
      vncRegion changedRegion;
      changedRegion.assignFromNewFormat(&updateContainer.changedRegion);
      m_server->UpdateRegion(changedRegion);
    }
  }

  shareRect();
  m_server->TriggerUpdate();

  return true;
}

void WinDesktop::shareRect()
{
  // EXAMINE THE SHARED AREA / WINDOW

  RECT rect = m_server->GetSharedRect();
  RECT new_rect;

  if (m_server->WindowShared()) {
    HWND hwnd = m_server->GetWindowShared();
    GetWindowRect(hwnd, &new_rect);
  } else if (m_server->ScreenAreaShared()) {
    new_rect = m_server->GetScreenAreaRect();
  } else {
    new_rect = m_bmrect;
  }

  if ((m_server->WindowShared() || m_server->GetApplication()) &&
      m_server->GetWindowShared() == NULL) {
    // Disconnect clients if the shared window has dissapeared.
    // FIXME: Make this behavior configurable.
    MessageBox(NULL, "You have exited an application that is being\n"
                     "viewed/controlled from a remote PC. Exiting this\n"
                     "application will terminate the session with the remote PC.",
                     "Warning", MB_ICONWARNING | MB_OK);
    vnclog.Print(LL_CONNERR, VNCLOG("shared window not found - disconnecting clients\n"));
    m_server->KillAuthClients();
    return;
  }

  // intersect the shared rect with the desktop rect
  IntersectRect(&new_rect, &new_rect, &m_bmrect);

  // Disconnect clients if the shared window is empty (dissapeared).
  // FIXME: Make this behavior configurable.
  if (new_rect.right - new_rect.left == 0 ||
      new_rect.bottom - new_rect.top == 0) {
    vnclog.Print(LL_CONNERR, VNCLOG("shared window empty - disconnecting clients\n"));
    m_server->KillAuthClients();
    return;
  }

  // Update screen size if required
  if (!EqualRect(&new_rect, &rect)) {
    m_server->SetSharedRect(new_rect);
    bool sendnewfb = false;

    if (rect.right - rect.left != new_rect.right - new_rect.left ||
        rect.bottom - rect.top != new_rect.bottom - new_rect.top ) {
      sendnewfb = true;
    }

    // FIXME: We should not send NewFBSize if a client
    //        did not send framebuffer update request.
    m_server->SetNewFBSize();
    return;
  }		
}

void WinDesktop::setNewScreenSize()
{
  m_bmrect.left   = 0;
  m_bmrect.top    = 0;
  m_bmrect.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  m_bmrect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}
