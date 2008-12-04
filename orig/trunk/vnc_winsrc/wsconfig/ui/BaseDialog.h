#ifndef _BASE_DIALOG_H_
#define _BASE_DIALOG_H_

#include "common/tstring.h"
#include "Control.h"

class BaseDialog
{
public:
  BaseDialog();
  BaseDialog(tstring resourceName);
  BaseDialog(LPTSTR resourceName);
  ~BaseDialog();
public:
  int show();
  int showModal();
  void setResourceName(LPTSTR resourceName);
  void setParent(Control *ctrlParent);
  virtual void kill(int code);

  static bool InitCommonControlsEx();
protected:
  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();
  static BOOL CALLBACK modalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
  tstring m_resourceName; // Name of dialog resource
  Control m_ctrlThis;     // This dialog control
  Control *m_ctrlParent;
};

#endif