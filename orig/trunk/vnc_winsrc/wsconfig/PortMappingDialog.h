#ifndef _PORT_MAPPING_DIALOG_H_
#define _PORT_MAPPING_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/TextBox.h"

class PortMappingDialog : public BaseDialog
{
public:
  PortMappingDialog();
  ~PortMappingDialog();
protected:
  virtual void onCommand(UINT cID, UINT nID);
  void onOkButtonClick();
  void onCancelButtonClick();
};

#endif