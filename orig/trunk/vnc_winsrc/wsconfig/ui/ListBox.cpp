#include "stdafx.h"
#include "ListBox.h"

ListBox::ListBox()
{
}

ListBox::~ListBox()
{
}

void ListBox::addString(const TCHAR *str)
{
  SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)str);
}

void ListBox::removeString(int index)
{
  int top = getTopIndex();
  SendMessage(m_hwnd, LB_DELETESTRING, index, NULL);
  setTopIndex(top);
}

int ListBox::getTopIndex()
{
  return (int)SendMessage(m_hwnd, LB_GETTOPINDEX, NULL, NULL);
}

void ListBox::setTopIndex(int index)
{
  SendMessage(m_hwnd, LB_SETTOPINDEX, index, NULL);
}

tstring ListBox::getItemText(int index)
{
  TCHAR temp[255];
  SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)&temp[0]);
  tstring result = temp;
  return result;
}

void ListBox::setItemText(int index, TCHAR *str)
{
  int si = getSelectedIndex();
  int top = getTopIndex();
  UINT topIndex = SendMessage(m_hwnd, LB_GETTOPINDEX, NULL, NULL);
  LPARAM data = getItemData(index);
  removeString(index);
  insertString(index, str, data);
  if (si == index) {
    setSelectedIndex(si);
  }
  setTopIndex(top);
}

void ListBox::addString(const TCHAR *str, LPARAM data)
{
  long index = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)str);
  setItemData(index, data);
}

void ListBox::insertString(int index, const TCHAR *str)
{
  SendMessage(m_hwnd, LB_INSERTSTRING, index, (LPARAM)str);
}

void ListBox::insertString(int index, const TCHAR *str, LPARAM data)
{
  long i = SendMessage(m_hwnd, LB_INSERTSTRING, index, (LPARAM)str);
  setItemData(i, data);
}

void ListBox::setItemData(int index, LPARAM data)
{
  SendMessage(m_hwnd, LB_SETITEMDATA, index, data);
}

LPARAM ListBox::getItemData(int index)
{
  return SendMessage(m_hwnd, LB_GETITEMDATA, index, NULL);
}

int ListBox::getCount()
{
  return SendMessage(m_hwnd, LB_GETCOUNT, NULL, NULL);
}

void ListBox::clear()
{
  SendMessage(m_hwnd, LB_RESETCONTENT, NULL, NULL);
}

int ListBox::getSelectedIndex()
{
  int index = SendMessage(m_hwnd, LB_GETCURSEL, NULL, NULL);
  return (index == LB_ERR) ? -1 : index;
}

void ListBox::setSelectedIndex(int index)
{
  SendMessage(m_hwnd, LB_SETCURSEL, index, NULL);
}