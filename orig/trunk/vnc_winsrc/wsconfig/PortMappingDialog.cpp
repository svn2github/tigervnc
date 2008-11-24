#include "StdAfx.h"
#include "Resource.h"
#include "PortMappingDialog.h"

PortMappingDialog::PortMappingDialog()
{
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