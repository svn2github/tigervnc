#ifndef _PORT_MAPPING_DIALOG_H_
#define _PORT_MAPPING_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/TextBox.h"
#include "Rect.h"

typedef enum {
  Add   = 0x0,
  Edit  = 0x1
} PortMappingDialogType;

class PortMappingDialog : public BaseDialog
{
public:
  PortMappingDialog();
  ~PortMappingDialog();
protected:
  void initControls();
  bool isUserDataValid();
  virtual void onInitDialog();
  virtual void onCommand(UINT cID, UINT nID);
  void onOkButtonClick();
  void onCancelButtonClick();

  Rect getRect() { return m_rect; }
  void setRect(Rect rect) { m_rect = rect; }

  PortMappingDialogType getDialogType() { return m_dialogType; }
  void setDialogType(PortMappingDialogType dialogType) { m_dialogType = dialogType; }
protected:
  TextBox m_geometryTextBox;
  TextBox m_portTextBox;
  PortMappingDialogType m_dialogType;
  Rect m_rect;
};

#endif