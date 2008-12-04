#ifndef _PORT_MAPPING_DIALOG_H_
#define _PORT_MAPPING_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/TextBox.h"
#include "PortMapping.h"

typedef enum {
  Add   = 0x0,
  Edit  = 0x1
} PortMappingDialogType;

class PortMappingDialog : public BaseDialog
{
public:
  PortMappingDialog();
  ~PortMappingDialog();
public:
  PortMapping getMapping() { return m_mapping; }
  void setMapping(PortMapping mapping) { m_mapping = mapping; }
  PortMappingDialogType getDialogType() { return m_dialogType; }
  void setDialogType(PortMappingDialogType dialogType) { m_dialogType = dialogType; }
protected:
  void initControls();
  bool isUserDataValid();
  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT cID, UINT nID);
  void onOkButtonClick();
  void onCancelButtonClick();
protected:
  TextBox m_geometryTextBox;
  TextBox m_portTextBox;
  PortMappingDialogType m_dialogType;
  PortMapping m_mapping;
};

#endif