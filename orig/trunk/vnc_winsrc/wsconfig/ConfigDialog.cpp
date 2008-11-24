#include "StdAfx.h"
#include "ConfigDialog.h"
#include "PortMappingDialog.h"
#include "Resource.h"

ConfigDialog::ConfigDialog(void)
{
  setResourceName(_T("WSConfig.MainDialog"));
}

ConfigDialog::~ConfigDialog(void)
{
}

void ConfigDialog::initControls()
{
  HWND dialogHwnd = m_ctrlThis.getWindow();
  m_ctrlMappingListBox.setWindow(GetDlgItem(dialogHwnd, IDC_MAPPINGS));
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
  }
}

void ConfigDialog::onInitDialog()
{
  initControls();
}

void ConfigDialog::onAddButtonClick()
{
  PortMappingDialog portMappingDialog;
  portMappingDialog.setParent(&m_ctrlThis);
  portMappingDialog.showModal();
}

void ConfigDialog::onEditButtonClick()
{
  PortMappingDialog portMappingDialog;
  portMappingDialog.setParent(&m_ctrlThis);
  portMappingDialog.showModal();
}

void ConfigDialog::onRemoveButtonClick()
{
}

void ConfigDialog::onCancelButtonClick()
{
  kill(0);
}

void ConfigDialog::onOKButtonClick()
{
  kill(0);
}

void ConfigDialog::onApplyButtonClick()
{
}