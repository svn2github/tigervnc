#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(void)
: m_changed(true)
{
}

FrameBuffer::~FrameBuffer(void)
{
}

bool FrameBuffer::SetChanged()
{
  // Check for changing

  return m_changed;
}

void FrameBuffer::Update()
{
  if (SetChanged()) {
  }
  return;
}