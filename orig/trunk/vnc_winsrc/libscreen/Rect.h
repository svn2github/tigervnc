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
  Rect(void) {};
  ~Rect(void) {};
  
  inline void SetRect(int lt, int tp, int rt, int bm)
  { 
    left = lt;
    top = tp;
    right = rt;
    bottom = bm;
  }

  inline void SetLeft(int value)    { left = value; }
  inline void SetTop(int value)     { top = value; }
  inline void SetRight(int value)   { right = value; }
  inline void SetBottom(int value)  { bottom = value; }
  inline void SetWidth(int value)   { right = left + value; }
  inline void SetHeight(int value)  { bottom = top + value; }

  inline int GetLeft()    { return left; }
  inline int GetTop()     { return top; }
  inline int GetRight()   { return right; }
  inline int GetBottom()  { return bottom; }
  inline int GetWidth()   { return right - left; }
  inline int GetHeight()  { return bottom - top; }

private:
  int left;
  int top;
  int right;
  int bottom;
};

#endif // __RECT_H__
