/* Copyright (C) 2005 TightVNC Team.  All Rights Reserved.
 *    
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

// -=- ScaledPixelBuffer.cxx

#include <rfb/Exception.h>
#include <rfb/ScaledPixelBuffer.h>

#include <math.h>
#include <memory.h>
#include <stdlib.h>

using namespace rdr;
using namespace rfb;

ScaledPixelBuffer::ScaledPixelBuffer(U8 **src_data_, int src_width_,
                                     int src_height_, int scale, PixelFormat pf_)
  : xWeightTabs(0), yWeightTabs(0), scaled_data(0), scale_ratio(1),
    scaleFilterID(scaleFilterBicubic) {

  setSourceBuffer(src_data_, src_width_, src_height_);
  setPF(pf_);
}

ScaledPixelBuffer::ScaledPixelBuffer() 
  : src_data(0), src_width(0), src_height(0), scale_ratio(1), scaled_width(0), 
    xWeightTabs(0), yWeightTabs(0), scaled_height(0), scaled_data(0),
    scaleFilterID(scaleFilterBicubic) {
  memset(&pf, 0, sizeof(pf));
}

ScaledPixelBuffer::~ScaledPixelBuffer() {
  freeWeightTabs();
}

void ScaledPixelBuffer::freeWeightTabs() {
  if (xWeightTabs) {
    for (int i = 0; i < scaled_width; i++) delete [] xWeightTabs[i].weight;
    delete [] xWeightTabs;
    xWeightTabs = 0;
  }
  if (yWeightTabs) {
    for (int i = 0; i < scaled_height; i++) delete [] yWeightTabs[i].weight;
    delete [] yWeightTabs;
    yWeightTabs = 0;
  }
}

void ScaledPixelBuffer::setSourceBuffer(U8 **src_data_, int w, int h) {
  freeWeightTabs();
  src_data = src_data_;
  src_width  = w;
  src_height = h;
  calculateScaledBufferSize();
  scaleFilters.makeWeightTabs(scaleFilterID, src_width, scaled_width, &xWeightTabs);
  scaleFilters.makeWeightTabs(scaleFilterID, src_height, scaled_height, &yWeightTabs);
}

void ScaledPixelBuffer::setPF(const PixelFormat &pf_) {
  ///if (pf_.depth != 24) throw rfb::UnsupportedPixelFormatException();
  pf = pf_;
}

void ScaledPixelBuffer::setScaleRatio(double scale_ratio_) {
  if (scale_ratio != scale_ratio_) {
    freeWeightTabs();
    scale_ratio = scale_ratio_;
    calculateScaledBufferSize();
    scaleFilters.makeWeightTabs(scaleFilterID, src_width, scaled_width, &xWeightTabs);
    scaleFilters.makeWeightTabs(scaleFilterID, src_height, scaled_height, &yWeightTabs);
  }
}

inline void ScaledPixelBuffer::rgbFromPixel(U32 p, int &r, int &g, int &b) {
  r = (((p >> pf.redShift  ) & pf.redMax  ) * 255 + pf.redMax  /2) / pf.redMax;
  g = (((p >> pf.greenShift) & pf.greenMax) * 255 + pf.greenMax/2) / pf.greenMax;
  b = (((p >> pf.blueShift ) & pf.blueMax ) * 255 + pf.blueMax /2) / pf.blueMax;
}

inline U32 ScaledPixelBuffer::getSourcePixel(int x, int y) {
  int bytes_per_pixel = pf.bpp / 8;
  U8 *ptr = &(*src_data)[(x + y*src_width)*bytes_per_pixel];
  if (bytes_per_pixel == 1) {
    return *ptr;
  } else if (bytes_per_pixel == 2) {
    int b0 = *ptr++; int b1 = *ptr;
    return b1 << 8 | b0;
  } else if (bytes_per_pixel == 4) {
    int b0 = *ptr++; int b1 = *ptr++;
    int b2 = *ptr++; int b3 = *ptr;
    return b3 << 24 | b2 << 16 | b1 << 8 | b0;
  } else {
    return 0;
  }
}

void ScaledPixelBuffer::scaleRect(const Rect& rect) {
  U8 *src_ptr, *ptr;
  static double c1_sub_dx, c1_sub_dy;
  Rect changed_rect;
  float rx, gx, bx;
  float red, green, blue; 
  int r, g, b;
  

  // Calculate the changed pixel rect in the scaled image
  changed_rect = calculateScaleBoundary(rect);

  int bytesPerPixel = pf.bpp/8;
  int bytesPerRow = src_width * bytesPerPixel;

  for (int y = changed_rect.tl.y; y < changed_rect.br.y; y++) {
    ptr = &(*scaled_data)[(changed_rect.tl.x + y*scaled_width) * bytesPerPixel];
    
    for (int x = changed_rect.tl.x; x < changed_rect.br.x; x++) {
            
      int ywi = 0; red = 0; green = 0; blue = 0;
      for (int ys = yWeightTabs[y].i0; ys < yWeightTabs[y].i1; ys++) {
        
        int xwi = 0; rx = 0; gx = 0; bx = 0;
        for (int xs = xWeightTabs[x].i0; xs < xWeightTabs[x].i1; xs++) {
          rgbFromPixel(getSourcePixel(xs, ys), r, g, b);
          rx += r * xWeightTabs[x].weight[xwi];
          gx += g * xWeightTabs[x].weight[xwi];
          bx += b * xWeightTabs[x].weight[xwi];
          xwi++;
        }
        red += rx * yWeightTabs[y].weight[ywi];
        green += gx * yWeightTabs[y].weight[ywi];
        blue += bx * yWeightTabs[y].weight[ywi];
        ywi++;
      }
      *ptr++ = U8(blue);
      *ptr++ = U8(green);
      *ptr++ = U8(red);
      ptr++;
    }
  }
}

Rect ScaledPixelBuffer::calculateScaleBoundary(const Rect& r) {
  int x_start, y_start, x_end, y_end;
  double sup = scaleFilters[scaleFilterID].radius;
  x_start = r.tl.x-sup < 0 ? 0 : int((r.tl.x-sup) * scale_ratio + 1);
  y_start = r.tl.y-sup < 0 ? 0 : int((r.tl.y-sup) * scale_ratio + 1);
  x_end = int((r.br.x+sup-1) * scale_ratio);
  x_end = x_end < scaled_width ? x_end + 1 : scaled_width;
  y_end = int((r.br.y+sup-1) * scale_ratio);
  y_end = y_end < scaled_height ? y_end + 1 : scaled_height;
  return Rect(x_start, y_start, x_end, y_end);
}

void ScaledPixelBuffer::calculateScaledBufferSize() {
  scaled_width  = (int)ceil(src_width  * scale_ratio);
  scaled_height = (int)ceil(src_height * scale_ratio);
}
