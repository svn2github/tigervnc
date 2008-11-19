#ifndef _LIST_BOX_H_
#define _LIST_BOX_H_

#include "Control.h"

class ListBox : public Control
{
public:
  ListBox();
  ~ListBox();
public:
  void addString(const TCHAR *str);
  tstring getItemText(int index);
  void setItemText(int index, TCHAR *str);
  void insertString(int index, const TCHAR *str);
  void insertString(int index, const TCHAR *str, LPARAM data);
  void addString(const TCHAR *str, LPARAM data);
  void setItemData(int index, LPARAM data);
  void removeString(int index);
  int getSelectedIndex();
  int getTopIndex();
  void setTopIndex(int index);
  void setSelectedIndex(int index);
  LPARAM getItemData(int index);
  int getCount();
  void clear();
};

#endif