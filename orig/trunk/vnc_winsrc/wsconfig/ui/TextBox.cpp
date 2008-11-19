#include "stdafx.h"
#include "TextBox.h"

TextBox::~TextBox()
{
}

void TextBox::scroll(int h, int v)
{
  SendMessage(m_hwnd, EM_LINESCROLL, h, v);
}

DWORD TextBox::getTextLimit()
{
  return SendMessage(m_hwnd, EM_GETLIMITTEXT, 0, 0);
}

int TextBox::getLineIndex()
{
  return SendMessage(m_hwnd, EM_LINEINDEX, -1, 0);
}

int TextBox::getLineCount()
{
  return SendMessage(m_hwnd, EM_GETLINECOUNT, 0, 0);
}

// FIXME: Unimplemented
int TextBox::getCaretPos()
{
  return 0;
}