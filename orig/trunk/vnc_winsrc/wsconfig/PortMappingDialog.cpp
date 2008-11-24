#include "StdAfx.h"
#include "Resource.h"
#include "common/StringParser.h"
#include "PortMappingDialog.h"

PortMappingDialog::PortMappingDialog()
: m_dialogType(Add)
{
  setResourceName(_T("WSConfig.EditPortMapping"));
}

PortMappingDialog::~PortMappingDialog()
{
}

void PortMappingDialog::onCancelButtonClick()
{
  kill(IDCANCEL);
}

void PortMappingDialog::onOkButtonClick()
{
  if (!isUserDataValid())
    return ;
  Rect::parse((TCHAR *)m_geometryTextBox.getText().c_str(), &m_mapping.rect);
  StringParser::parseInt(m_portTextBox.getText(), &m_mapping.port);
  kill(IDOK);
}

void PortMappingDialog::initControls()
{
  HWND dialogHwnd = m_ctrlThis.getWindow();
  m_geometryTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_GEOMETRY_EDIT));
  m_portTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_PORT_EDIT));
}

bool PortMappingDialog::isUserDataValid()
{
  if (!Rect::tryParse((TCHAR *)m_geometryTextBox.getText().c_str())) {
    MessageBox(m_ctrlThis.getWindow(), _T("Wrong geometry string format"), _T("Error"), MB_OK | MB_ICONWARNING);
    return false;
  }
  int port;
  StringParser::parseInt((TCHAR *)m_portTextBox.getText().c_str(), &port);
  if ((port < 1) || (port > 65535)) {
    MessageBox(m_ctrlThis.getWindow(), _T("Port must be between 1 and 65535"), _T("Error"), MB_OK | MB_ICONWARNING);
    return false;
  }
  return true;
}

void PortMappingDialog::onInitDialog()
{
  initControls();
  if (m_dialogType == Add) {
    m_portTextBox.setText(_T("5901"));
    m_geometryTextBox.setText(_T("640x480+0+0"));
  }
  if (m_dialogType == Edit) {
    TCHAR str[10];
    _ltot(m_mapping.port, &str[0], 10);
    m_portTextBox.setText(str);
    m_geometryTextBox.setText((TCHAR *)m_mapping.rect.toString().c_str());
  }
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