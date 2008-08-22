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

#ifndef __RECT_H__
#define __RECT_H__

class Rect
{
public:
  Rect(void) { left = 0; top = 0; right = 0; bottom = 0; }
  ~Rect(void) {};
  
  int left;
  int top;
  int right;
  int bottom;

  inline void SetRect(int lt, int tp, int rt, int bm)
  { 
    left = lt;
    top = tp;
    right = rt;
    bottom = bm;
  }

  inline bool CmpRect(const Rect *rect) { return  rect->left == left &&
                                                  rect->top == top &&
                                                  rect->right == right &&
                                                  rect->bottom == bottom; }

  inline void SetWidth(int value)   { right = left + value; }
  inline void SetHeight(int value)  { bottom = top + value; }

  inline int GetWidth()  const { return right - left; }
  inline int GetHeight() const { return bottom - top; }

};

#endif // __RECT_H__
