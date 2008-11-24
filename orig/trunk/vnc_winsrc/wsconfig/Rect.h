#ifndef _RECT_H_
#define _RECT_H_

class Rect
{
public:
  Rect();
  ~Rect();
public:
  static bool tryParse(LPTSTR string);
public:
  int x;
  int y;
  int width;
  int height;
};

#endif