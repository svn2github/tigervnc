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

bool FrameBuffer::cmp(FrameBuffer *frameBuffer)
{
  return m_dimension.cmpDim(&(frameBuffer->getDimension())) &&
         (memcmp(&m_pixelFormat, &(frameBuffer->getPixelFormat()),
         sizeof(PixelFormat)) == 0);
}
bool FrameBuffer::copyFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY)
{
  if (memcmp(&m_pixelFormat, &srcFrameBuffer->getPixelFormat(),
              sizeof(PixelFormat)) != 0) {
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
  Dimension dstDim(&dstRectFB);
  Dimension srcDim(&srcRectFB);
  Rect commonRect(&dstDim.getRect().intersection(&srcDim.getRect()));

  // Moving commonRect to destination coordinates and source
  Rect resultDstRect(&commonRect);
  resultDstRect.move(dstRect->left, dstRect->top);
  int resultSrcX = srcX + resultDstRect.left;
  int resultSrcY = srcY + resultDstRect.top;

  // Data copy
  int dstStrike = m_dimension.width;
  int srcStrike = srcDimension.width;
  int pdstPixel = resultDstRect.top * dstStrike + resultDstRect.left;
  int psrcPixel = resultSrcY * srcStrike + resultSrcX;
  int resultHeight = resultDstRect.getHeight();
  int resultWidth = resultDstRect.getWidth();
  UINT8 *srcDataBuffer = (UINT8 *)srcFrameBuffer->getBuffer();

  for (int i = 0; i < resultHeight; i++, pdstPixel += dstStrike, psrcPixel += srcStrike) {
    memcpy((UINT8 *)m_buffer + pdstPixel, srcDataBuffer + psrcPixel,
           (resultWidth * m_pixelFormat.bitsPerPixel) / 8);
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
  UINT32 newArea = newDim->area();
  UINT32 oldArea = m_dimension.area();

  m_dimension = *newDim;

  if (resizeBuff && (newArea != oldArea))
  {
    return resizeBuffer();
  }
  return true;
}

int FrameBuffer::getBufferSize()
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
