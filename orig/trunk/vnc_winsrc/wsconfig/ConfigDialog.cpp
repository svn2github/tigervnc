#include "StdAfx.h"
#include "ConfigDialog.h"
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

void ConfigDialog::onInitDialog()
{
  initControls();
}

void ConfigDialog::onAddButtonClick()
{
}

void ConfigDialog::onEditButtonClick()
{
}

void ConfigDialog::onRemoveButtonClick()
{
}

void ConfigDialog::onCancelButtonClick()
{
}

void ConfigDialog::onOKButtonClick()
{
}

void ConfigDialog::onApplyButtonClick()
{
}