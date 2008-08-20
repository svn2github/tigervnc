#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "Rect.h"

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  virtual void Update() = 0;

  virtual bool GetPixelFormatChanged() const { return m_pixelFormatChanged; }
  virtual bool GetSizeChanged() const { return m_sizeChanged; }

protected:
  virtual void SetPropertiesChanged() = 0;

  bool m_pixelFormatChanged;
  bool m_sizeChanged;
};

#endif // __FRAMEBUFFER_H__
