#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "common/tstring.h"

// Simple class to manipulate Window
//
class Control
{
public:
  Control();
  Control(HWND hwnd);
  virtual ~Control() { }

  HWND getWindow() { return m_hwnd; }
  void setWindow(HWND hwnd) { m_hwnd = hwnd; }

  // Window style
  void setStyle(DWORD style);
  DWORD getStyle();

  void setStyleFlag(DWORD value);
  void clearStyleFlag(DWORD value);
  bool isStyleFlagEnabled(DWORD value);

  // Enable/disabe methods
  void enable();
  void disable();
  bool isEnabled();

  void setFocus();
  void setForeground();
  void hide();
  void show();
  void invalidate();

  // FIXME: Think - in all components we have setText/getText
  // methods?
  void setText(LPTSTR text);
  tstring getText();

protected:
  HWND m_hwnd;

  // FIXME: Not using it yet. Maybe need to remove it?
  virtual void setDefaultFlags() { };
};

#endif