#include "StdAfx.h"
#include "BaseDialog.h"

BaseDialog::BaseDialog()
: m_ctrlParent(NULL)
{
  m_resourceName = _T("");
}

BaseDialog::BaseDialog(tstring resourceName)
: m_ctrlParent(NULL)
{
  m_resourceName = resourceName;
}

BaseDialog::BaseDialog(LPTSTR resourceName)
: m_ctrlParent(NULL)
{
  m_resourceName = resourceName;
}

BaseDialog::~BaseDialog()
{
}

void BaseDialog::setResourceName(LPTSTR resourceName)
{
  m_resourceName = resourceName;
}

void BaseDialog::setParent(Control *ctrlParent)
{
  m_ctrlParent = ctrlParent;
}

int BaseDialog::show()
{
  return 0;
}

void BaseDialog::kill(int code)
{
  EndDialog(m_ctrlThis.getWindow(), code);
}

int BaseDialog::showModal()
{
  int dialogResult = -1;
  HWND parentHwnd = m_ctrlParent == NULL ? NULL : m_ctrlParent->getWindow();
  if ((dialogResult = DialogBoxParam(NULL, (TCHAR *)m_resourceName.c_str(),
                     parentHwnd, modalDialogProc, (LPARAM)this)) == -1) {
    // Error notification
    //
  }
  return dialogResult;
}

void BaseDialog::onInitDialog()
{
}

void BaseDialog::onNotify(UINT controlID, LPARAM data)
{
}

void BaseDialog::onCommand(UINT controlID, UINT notificationID)
{
}

void BaseDialog::onDestroy()
{
}

BOOL CALLBACK BaseDialog::modalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{  
  BaseDialog *_this;
  if (uMsg == WM_INITDIALOG) {
    _this = (BaseDialog *)lParam;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)_this);
    _this->m_ctrlThis.setWindow(hwnd);
  } else {
    _this = (BaseDialog *)GetWindowLong(hwnd, GWL_USERDATA);
    if (_this == 0) {
      return FALSE;
    }
  }

  switch (uMsg) {
  case WM_INITDIALOG:
    _this->onInitDialog();
    break;
  case WM_NOTIFY:
    _this->onNotify(LOWORD(wParam), lParam);
    break;
  case WM_COMMAND:
    _this->onCommand(LOWORD(wParam), HIWORD(wParam));
    break;
  case WM_DESTROY:
    _this->onDestroy();
    break;
  }

  return FALSE;
}