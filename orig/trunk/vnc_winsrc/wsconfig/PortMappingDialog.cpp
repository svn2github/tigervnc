#include "StdAfx.h"
#include "Resource.h"
#include "PortMappingDialog.h"

PortMappingDialog::PortMappingDialog()
{
  setResourceName(_T("WSConfig.EditPortMapping"));
}

PortMappingDialog::~PortMappingDialog()
{
}

void PortMappingDialog::onCancelButtonClick()
{
  kill(0);
}

void PortMappingDialog::onOkButtonClick()
{
  kill(0);
}

void PortMappingDialog::initControls()
{
  HWND dialogHwnd = m_ctrlThis.getWindow();
  m_geometryTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_GEOMETRY_EDIT));
}

void PortMappingDialog::onInitDialog()
{
  initControls();
}

void PortMappingDialog::onCommand(UINT cID, UINT nID)
{
  switch (cID) {
  case IDOK:
    onOkButtonClick();
    break;
  case IDCANCEL:
    onCancelButtonClick();
    break;
  }
}