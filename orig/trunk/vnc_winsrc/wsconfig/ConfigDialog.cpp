#include "StdAfx.h"
#include "ConfigDialog.h"
#include "PortMappingDialog.h"
#include "Resource.h"

ConfigDialog::ConfigDialog(void)
{
  setResourceName(_T("WSConfig.MainDialog"));
  m_settingsManager = new RegistrySettingsManager();
  ((RegistrySettingsManager *)m_settingsManager)->setRootHKEY(HKEY_LOCAL_MACHINE);
  ((RegistrySettingsManager *)m_settingsManager)->setRootFolderName(_T("Software\\ORL\\WinVNC3\\"));
  m_settingsManager->setInstance(m_settingsManager);
}

ConfigDialog::~ConfigDialog(void)
{
  delete m_settingsManager;
}

void ConfigDialog::initControls()
{
  HWND dialogHwnd = m_ctrlThis.getWindow();
  m_ctrlMappingListBox.setWindow(GetDlgItem(dialogHwnd, IDC_MAPPINGS));
  m_ctrlEditButton.setWindow(GetDlgItem(dialogHwnd, IDC_EDIT_PORT));
  m_ctrlRemoveButton.setWindow(GetDlgItem(dialogHwnd, IDC_REMOVE_PORT));
  m_ctrlApplyButton.setWindow(GetDlgItem(dialogHwnd, IDC_APPLY));
}

void ConfigDialog::loadSettings()
{
  m_config.loadFromStorage(m_settingsManager);
  for (int i = 0; i < m_config.m_vPortMapping.size(); i++) {
    m_ctrlMappingListBox.insertString(i, (TCHAR *)m_config.m_vPortMapping.at(i).toString().c_str());
  }
}

void ConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDOK:
    onOKButtonClick();
    break;
  case IDCANCEL:
    onCancelButtonClick();
    break;
  case IDC_APPLY:
    onApplyButtonClick();
    break;
  case IDC_ADD_PORT:
    onAddButtonClick();
    break;
  case IDC_EDIT_PORT:
    onEditButtonClick();
    break;
  case IDC_REMOVE_PORT:
    onRemoveButtonClick();
    break;
  case IDC_MAPPINGS:
    switch (notificationID) {
    case LBN_SELCHANGE:
      onMappingListBoxSelChange();
      break;
    case LBN_DBLCLK:
      onMappingListBoxDoubleClick();
      break;
    }
    break;
  }
}

void ConfigDialog::onInitDialog()
{
  initControls();
  loadSettings();
}

void ConfigDialog::onMappingListBoxSelChange()
{
  int selectedIndex = m_ctrlMappingListBox.getSelectedIndex();
  if (selectedIndex == -1) {
    m_ctrlEditButton.disable();
    m_ctrlRemoveButton.disable();
    return ;
  }
  else {
    m_ctrlEditButton.enable();
    m_ctrlRemoveButton.enable();
  }
}

void ConfigDialog::onAddButtonClick()
{
  PortMappingDialog portMappingDialog;
  portMappingDialog.setParent(&m_ctrlThis);
  portMappingDialog.setDialogType(Add);
  if (portMappingDialog.showModal() == IDOK) {
    PortMapping newMapping = portMappingDialog.getMapping();
    m_config.m_vPortMapping.push_back(newMapping);
    m_ctrlMappingListBox.addString((TCHAR *)newMapping.toString().c_str());
    m_ctrlApplyButton.enable();
  }
}

void ConfigDialog::onEditButtonClick()
{
  PortMappingDialog portMappingDialog;
  portMappingDialog.setParent(&m_ctrlThis);
  portMappingDialog.setDialogType(Edit);
  int selectedIndex = m_ctrlMappingListBox.getSelectedIndex();
  if (selectedIndex == -1) {
    return ;
  }
  portMappingDialog.setMapping(m_config.m_vPortMapping.at(selectedIndex));
  if (portMappingDialog.showModal() == IDOK) {
    PortMapping editedMapping = portMappingDialog.getMapping();
    PortMapping *oldPtrMapping = &m_config.m_vPortMapping.at(selectedIndex);
    *oldPtrMapping = editedMapping;
    m_ctrlMappingListBox.setItemText(selectedIndex,(TCHAR *)editedMapping.toString().c_str());
    m_ctrlApplyButton.enable();
  }
}

void ConfigDialog::onRemoveButtonClick()
{
  int selectedIndex = m_ctrlMappingListBox.getSelectedIndex();
  if (selectedIndex == -1) {
    return ;
  }
  m_ctrlMappingListBox.removeString(selectedIndex);
  PortMappingVector::iterator it = m_config.m_vPortMapping.begin();
  for (int j = 0; it != m_config.m_vPortMapping.end(); it++) {
    if (j == selectedIndex) {
      m_config.m_vPortMapping.erase(it);
      m_ctrlApplyButton.enable();
      break;
    }
    j++;
  }
  if (m_ctrlMappingListBox.getCount() > 0) {
    m_ctrlMappingListBox.setSelectedIndex(selectedIndex);
    if (m_ctrlMappingListBox.getSelectedIndex() == -1) {
      m_ctrlMappingListBox.setSelectedIndex(selectedIndex - 1);
    }
    if (m_ctrlMappingListBox.getSelectedIndex() == -1) {
      m_ctrlMappingListBox.setSelectedIndex(selectedIndex + 1);
    }
  }
  else {
    m_ctrlRemoveButton.disable();
    m_ctrlEditButton.disable();
  }
}

void ConfigDialog::onCancelButtonClick()
{
  kill(0);
}

void ConfigDialog::onOKButtonClick()
{
  onApplyButtonClick();
  kill(0);
}

void ConfigDialog::onApplyButtonClick()
{
  if (!m_config.saveToStorage(m_settingsManager)) {
    MessageBox(m_ctrlThis.getWindow(), _T("Cannot save data to storage"), _T("Error"), MB_OK | MB_ICONERROR);
  }
  else {
    m_ctrlApplyButton.disable();

    // Restart server
    STARTUPINFO sti;
    ZeroMemory(&sti,sizeof(STARTUPINFO));
    sti.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi;
    TCHAR *szCmdline = _tcsdup(TEXT("\"WinVNC.exe\" -reload"));
    BOOL bResult = CreateProcess(NULL, (TCHAR *)szCmdline, NULL, NULL, FALSE, NULL, NULL, NULL, &sti, &pi);
    delete []szCmdline;
    if (bResult == FALSE) {
      MessageBox(m_ctrlThis.getWindow(), _T("Cannot restart WinVNC.exe"), _T("Warning"), MB_OK | MB_ICONWARNING);
    }
  }
}

void ConfigDialog::onMappingListBoxDoubleClick()
{
  if (m_ctrlMappingListBox.getSelectedIndex() != -1)
    onEditButtonClick();
}