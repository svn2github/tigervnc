#ifndef __TEXTBOX_H_
#define __TEXTBOX_H_

#include "Control.h"

class TextBox : public Control
{
public:
  ~TextBox();
public:
  void scroll(int h, int v);
  DWORD getTextLimit();
  int getLineIndex();
  int getLineCount();
  int getCaretPos();
};

#endif