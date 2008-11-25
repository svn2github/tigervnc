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

bool Settings::loadFromStorage(SettingsManager *sm)
{
  bool loadResult = true;
  TCHAR strVal[1024*4];
  if (!sm->getString(_T("ExtraPorts"), &strVal[0])) {
    loadResult = false;
  }
  else {
    tstring strMappings = strVal;
    size_t index = 0;
    size_t lastIndex = 0;
    while (true) {
      index = strMappings.find(_T(","), lastIndex);
      if (index == tstring::npos) {
        break;
      }
      tstring strMapping = strMappings.substr(lastIndex, index - lastIndex);
      PortMapping newPortMapping;
      if (PortMapping::parse(strMapping, &newPortMapping)) {
        m_vPortMapping.push_back(newPortMapping);
      }
      lastIndex = index + 1;
    }
    tstring strMapping = strMappings.substr(lastIndex, strMappings.size() - lastIndex);
    PortMapping newPortMapping;
    if (PortMapping::parse(strMapping, &newPortMapping)) {
      m_vPortMapping.push_back(newPortMapping);
    }
  }
  return loadResult;
}