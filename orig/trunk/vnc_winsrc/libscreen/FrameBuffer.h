#ifndef __FRAMEBUFFER_H__

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  virtual void Update();

  bool GetChanged() {return m_changed;}

protected:
  bool m_changed;
  bool SetChanged();
};

#endif // __FRAMEBUFFER_H__
