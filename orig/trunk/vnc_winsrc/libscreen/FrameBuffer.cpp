//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
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

#include "FrameBuffer.h"
#include <string.h>

FrameBuffer::FrameBuffer(void)
: m_buffer(0)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

FrameBuffer::~FrameBuffer(void)
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
}

bool FrameBuffer::assignProperties(const FrameBuffer *srcFrameBuffer, const bool resizeBuff)
{
  setPixelFormat(&srcFrameBuffer->getPixelFormat(), false);
  setDimension(&srcFrameBuffer->getDimension(), false);

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

bool FrameBuffer::assignClone(const FrameBuffer *srcFrameBuffer)
{
  if (!assignProperties(srcFrameBuffer)) {
    return false;
  }

  Rect fbRect = &m_dimension.getRect();
  copyFrom(&fbRect, srcFrameBuffer, fbRect.left, fbRect.top);

  return true;
}

bool FrameBuffer::cmp(const FrameBuffer *frameBuffer)
{
  return m_dimension.cmpDim(&frameBuffer->getDimension()) &&
         m_pixelFormat.cmp(&frameBuffer->getPixelFormat());
}

bool FrameBuffer::copyFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY)
{
  if (!m_pixelFormat.cmp(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Dimension srcDimension = srcFrameBuffer->getDimension();
  Rect dstBufferRect = m_dimension.getRect();
  Rect srcBufferRect = srcDimension.getRect();

  // Building srcRect
  Rect srcRect(srcX, srcY, srcX + dstRect->getWidth(), srcY + dstRect->getHeight());

  // Finding common area between the dstRect, srcRect and the FrameBuffers
  Rect dstRectFB = dstBufferRect.intersection(dstRect);
  Rect srcRectFB = srcBufferRect.intersection(&srcRect);

  // Finding common area between the dstRectFB and the srcRectFB
  Rect dstCommonArea(&dstRectFB);
  Rect srcCommonArea(&srcRectFB);
  // Move to common place (left = 0, top = 0)
  dstCommonArea.move(-dstRect->left, -dstRect->top);
  srcCommonArea.move(-srcRect.left, -srcRect.top);

  Rect commonRect(&dstCommonArea.intersection(&srcCommonArea));

  // Moving commonRect to destination coordinates and source
  Rect resultDstRect(&commonRect);
  resultDstRect.move(dstRect->left, dstRect->top);
  int resultSrcX = srcRect.left + commonRect.left;
  int resultSrcY = srcRect.top + commonRect.top;

  // Data copy
  int dstStride = m_dimension.width;
  int srcStride = srcDimension.width;
  int pdstPixel = resultDstRect.top * dstStride + resultDstRect.left;
  int psrcPixel = resultSrcY * srcStride + resultSrcX;
  int resultHeight = resultDstRect.getHeight();
  int resultWidth = resultDstRect.getWidth();
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  UINT8 *srcDataBuffer = (UINT8 *)srcFrameBuffer->getBuffer();

  for (int i = 0; i < resultHeight; i++, pdstPixel += dstStride, psrcPixel += srcStride) {
    memcpy((UINT8 *)m_buffer + pdstPixel * pixelSize,
           srcDataBuffer + psrcPixel * pixelSize,
           resultWidth * pixelSize);
  }

  return true;
}

bool FrameBuffer::cmpFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY)
{
  // FIXME: Remove code duplicate (copyFrom() function)
  if (!m_pixelFormat.cmp(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Dimension srcDimension = srcFrameBuffer->getDimension();
  Rect dstBufferRect = m_dimension.getRect();
  Rect srcBufferRect = srcDimension.getRect();

  // Building srcRect
  Rect srcRect(srcX, srcY, srcX + dstRect->getWidth(), srcY + dstRect->getHeight());

  // Finding common area between the dstRect, srcRect and the FrameBuffers
  Rect dstRectFB = dstBufferRect.intersection(dstRect);
  Rect srcRectFB = srcBufferRect.intersection(&srcRect);

  // Finding common area between the dstRectFB and the srcRectFB
  Rect dstCommonArea(&dstRectFB);
  Rect srcCommonArea(&srcRectFB);
  // Move to common place (left = 0, top = 0)
  dstCommonArea.move(-dstRect->left, -dstRect->top);
  srcCommonArea.move(-srcRect.left, -srcRect.top);

  Rect commonRect(&dstCommonArea.intersection(&srcCommonArea));

  // Moving commonRect to destination coordinates and source
  Rect resultDstRect(&commonRect);
  resultDstRect.move(dstRect->left, dstRect->top);
  int resultSrcX = srcRect.left + commonRect.left;
  int resultSrcY = srcRect.top + commonRect.top;

  // Data copy
  int dstStride = m_dimension.width;
  int srcStride = srcDimension.width;
  int pdstPixel = resultDstRect.top * dstStride + resultDstRect.left;
  int psrcPixel = resultSrcY * srcStride + resultSrcX;
  int resultHeight = resultDstRect.getHeight();
  int resultWidth = resultDstRect.getWidth();
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  UINT8 *srcDataBuffer = (UINT8 *)srcFrameBuffer->getBuffer();

  for (int i = 0; i < resultHeight; i++, pdstPixel += dstStride, psrcPixel += srcStride) {
    if (memcmp((UINT8 *)m_buffer + pdstPixel * pixelSize,
                srcDataBuffer + psrcPixel * pixelSize,
                resultWidth * pixelSize) != 0) {
      return false;
    }
  }

  return true;
}

bool FrameBuffer::setPixelFormat(const PixelFormat *pixelFormat, bool resizeBuff)
{
  m_pixelFormat = *pixelFormat;

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

bool FrameBuffer::setDimension(const Dimension *newDim, bool resizeBuff)
{
  m_dimension = *newDim;

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

int FrameBuffer::getBufferSize() const
{ 
  return (m_dimension.area() * m_pixelFormat.bitsPerPixel) / 8;
}

bool FrameBuffer::resizeBuffer()
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
  if ((m_buffer = new UINT8[getBufferSize()]) == 0) {
    return false;
  }
  return true;
}
