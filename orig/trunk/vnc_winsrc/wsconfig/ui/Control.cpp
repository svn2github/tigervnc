#include "stdafx.h"
#include "Control.h"

Control::Control()
: m_hwnd(NULL)
{
}

Control::Control(HWND hwnd)
: m_hwnd(hwnd)
{
}

void Control::enable()
{  
  if (isEnabled()) return ;
  clearStyleFlag(WS_DISABLED);
  SendMessage(m_hwnd, WM_ENABLE, TRUE, NULL);
  clearStyleFlag(WS_DISABLED);
}

void Control::setForeground()
{
  SetForegroundWindow(getWindow());
}

void Control::setText(LPTSTR text)
{
  SetWindowText(m_hwnd, text);
}

tstring Control::getText()
{
  // FIXME: Not correct to allocate 30 000. it can be less
  // in some OS
  DWORD maxTextBoxSize = 30000;
  TCHAR *buf = new TCHAR[maxTextBoxSize];
  GetWindowText(m_hwnd, buf, maxTextBoxSize);
  tstring result = buf;
  delete []buf;
  return result;
}

void Control::invalidate()
{
  InvalidateRect(m_hwnd, NULL, TRUE);
}

void Control::disable()
{
  if (isStyleFlagEnabled(WS_DISABLED)) return ;
  setStyleFlag(WS_DISABLED);
  SendMessage(m_hwnd, WM_ENABLE, FALSE, NULL);
}

bool Control::isEnabled()
{
  return (!isStyleFlagEnabled(WS_DISABLED));
}

void Control::setFocus()
{
  ::SetFocus(m_hwnd);
}

void Control::setStyle(DWORD style)
{
  ::SetWindowLong(m_hwnd, GWL_STYLE, style);
}

DWORD Control::getStyle()
{
  return ::GetWindowLong(m_hwnd, GWL_STYLE);
}

void Control::setStyleFlag(DWORD value)
{
  DWORD flags = getStyle();
  flags |= value;
  setStyle(flags);
}

void Control::clearStyleFlag(DWORD value)
{
  DWORD flags = getStyle();
  flags &= ~value;
  setStyle(flags);
}

bool Control::isStyleFlagEnabled(DWORD value)
{
  DWORD flags = getStyle();
  return (flags & value) == value;
}

void Control::hide()
{
  ShowWindow(m_hwnd, SW_HIDE);
}

void Control::show()
{
  ShowWindow(m_hwnd, SW_SHOW);
}