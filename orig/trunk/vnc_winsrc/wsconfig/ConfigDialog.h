#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/Control.h"
#include "ui/ListBox.h"

class ConfigDialog : public BaseDialog
{
public:
  ConfigDialog(void);
  ~ConfigDialog(void);
protected:
  void initControls();
  // Init dialog handler
  virtual void onInitDialog();
  // Button handlers
  void onAddButtonClick();
  void onEditButtonClick();
  void onRemoveButtonClick();
  void onCancelButtonClick();
  void onOKButtonClick();
  void onApplyButtonClick();
protected:
  // Controls
  ListBox m_ctrlMappingListBox;
};

#endif