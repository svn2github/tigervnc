#ifndef __RECT_H__

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
