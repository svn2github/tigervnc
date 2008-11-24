#ifndef _PORT_MAPPING_DIALOG_H_
#define _PORT_MAPPING_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/TextBox.h"
#include "Rect.h"

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
protected:
  TextBox m_geometryTextBox;
  TextBox m_portTextBox;
  Rect m_rect;
};

#endif