#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include "common/SettingsManager.h"
#include "common/RegistrySettingsManager.h"
#include "ui/BaseDialog.h"
#include "ui/Control.h"
#include "ui/ListBox.h"
#include "Settings.h"

class ConfigDialog : public BaseDialog
{
public:
  ConfigDialog(void);
  ~ConfigDialog(void);
protected:
  void initControls();
  void loadSettings();
  // Init dialog handler
  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  // Button handlers
  void onAddButtonClick();
  void onEditButtonClick();
  void onRemoveButtonClick();
  void onCancelButtonClick();
  void onOKButtonClick();
  void onApplyButtonClick();
  void onMappingListBoxSelChange();
  void onMappingListBoxDoubleClick();
protected:
  // Controls
  ListBox m_ctrlMappingListBox;
  Control m_ctrlEditButton;
  Control m_ctrlRemoveButton;
  Control m_ctrlApplyButton;
  // Settings
  Settings m_config;
  SettingsManager *m_settingsManager;
};

#endif