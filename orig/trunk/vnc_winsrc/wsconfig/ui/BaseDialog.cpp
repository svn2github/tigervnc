#include "StdAfx.h"
#include "BaseDialog.h"

BaseDialog::BaseDialog()
{
  m_resourceName = _T("");
}

BaseDialog::BaseDialog(tstring resourceName)
{
  m_resourceName = resourceName;
}

BaseDialog::BaseDialog(LPTSTR resourceName)
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

int BaseDialog::show()
{
  return 0;
}

int BaseDialog::showModal()
{
  int dialogResult = -1;
  if ((dialogResult = DialogBoxParam(NULL, (TCHAR *)m_resourceName.c_str(),
                     NULL, modalDialogProc, (LPARAM)this)) == -1) {
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
  BaseDialog *_this = (BaseDialog *)GetWindowLong(hwnd, GWL_USERDATA);

  switch (uMsg) {
  case WM_INITDIALOG:
    _this = (BaseDialog *)lParam;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)_this);
    _this->m_ctrlThis.setWindow(hwnd);
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