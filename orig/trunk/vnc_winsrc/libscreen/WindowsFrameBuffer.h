#ifndef __WINDOWSFRAMEBUFFER_H__
#define __WINDOWSFRAMEBUFFER_H__

#include <windows.h>
#include "framebuffer.h"
#include "rect.h"

class WindowsFrameBuffer :
  public FrameBuffer
{
public:
  WindowsFrameBuffer(void);
  virtual ~WindowsFrameBuffer(void);

  virtual void Update();

protected:
  virtual void SetPropertiesChanged();

  void CaptureScreenRect(Rect *aRect, HDC dstDC);
  HPALETTE GetSystemPalette();
};

#endif // __WINDOWSFRAMEBUFFER_H__
