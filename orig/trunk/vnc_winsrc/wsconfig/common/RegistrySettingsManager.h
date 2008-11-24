#ifndef _REGISTRY_SETTINGS_MANAGER_
#define _REGISTRY_SETTINGS_MANAGER_

#include "SettingsManager.h"

class RegistrySettingsManager : public SettingsManager
{
public:
  RegistrySettingsManager(void);
  ~RegistrySettingsManager(void);

public:  
  virtual bool isOk();

  virtual bool keyExist(LPCTSTR name);
  virtual bool deleteKey(LPCTSTR name);

  virtual bool getString(LPCTSTR name, LPTSTR value);
  virtual bool setString(LPCTSTR name, LPTSTR value);

  virtual bool getLong(LPCTSTR name, long *value);
  virtual bool setLong(LPCTSTR name, long value);

  void setRootHKEY(HKEY key);
  HKEY getRootHKEY();

  void setRootFolderName(LPCTSTR folderName);
  LPTSTR getRootFolderName() { return m_appFolderName; }

protected:
  HKEY m_systemRootKey;
  HKEY m_appRootKey;
  LPTSTR m_appFolderName;

  LPTSTR getFolderName(LPCTSTR key);
  LPTSTR getKeyName(LPCTSTR key);

};

#endif