#ifndef _SETTINGS_MANAGER_
#define _SETTINGS_MANAGER_

class SettingsManager
{
public:
  SettingsManager(void);
  ~SettingsManager(void);
public:  
  virtual bool isOk() { return true; };

  virtual bool keyExist(LPCTSTR name) { return false; };
  virtual bool deleteKey(LPCTSTR name) { return false; };

  virtual bool getString(LPCTSTR name, LPTSTR value) { return false; };
  virtual bool setString(LPCTSTR name, LPTSTR value) { return false; };

  virtual bool getLong(LPCTSTR name, long *value) { return false; };
  virtual bool setLong(LPCTSTR name, long value) { return false; };

  static void setInstance(SettingsManager *manager) { m_instance = manager; }
  static SettingsManager *getInstance() { return m_instance; }
protected:
  static SettingsManager *m_instance;
};

#endif