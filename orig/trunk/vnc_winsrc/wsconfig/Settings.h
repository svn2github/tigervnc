#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "common/SettingsManager.h"
#include "PortMappingVector.h"

class Settings
{
public:
  Settings();
public:
  bool saveToStorage(SettingsManager *sm);
  bool loadFromStorage(SettingsManager *sm);
public:
  PortMappingVector m_vPortMapping;
};

#endif