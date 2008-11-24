#include "StdAfx.h"
#include "Settings.h"

Settings::Settings()
{
}

bool Settings::saveToStorage(SettingsManager *sm)
{
  bool saveResult = true;
  // Save Port Mapping Vector
  tstring strPortMapping = _T("");
  for (int i = 0; i < m_vPortMapping.size(); i++) {
    strPortMapping += m_vPortMapping.at(i).toString();
    if (i != m_vPortMapping.size() - 1)
      strPortMapping += _T(",");
  }
  if (!sm->setString(_T("ExtraPorts"), (TCHAR *)strPortMapping.c_str())) {
    saveResult = false;
  }
  return saveResult;
}