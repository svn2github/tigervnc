#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include "ui/Control.h"
#include "ui/ListBox.h"

class ConfigDialog
{
public:
  ConfigDialog(void);
  ~ConfigDialog(void);
protected:
  // Init dialog handler
  void onInitDialog(HWND hWnd);
  // Button handlers
  void onAddButtonClick();
  void onEditButtonClick();
  void onRemoveButtonClick();
  void onCancelButtonClick();
  void onOKButtonClick();
  void onApplyButtonClick();
protected:
  // Controls
  Control m_ctrlThis;
  ListBox m_ctrlMappingListBox;
};

#endif