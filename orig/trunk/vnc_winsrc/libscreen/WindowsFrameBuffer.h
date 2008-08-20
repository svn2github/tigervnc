#pragma once
#include "framebuffer.h"

class WindowsFrameBuffer :
  public FrameBuffer
{
public:
  WindowsFrameBuffer(void);
  virtual ~WindowsFrameBuffer(void);
  void CaptureScreenRect(RECT *aRect, HDC dstDC);
  HPALETTE GetSystemPalette();
};
