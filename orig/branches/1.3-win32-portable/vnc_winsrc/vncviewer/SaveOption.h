#ifndef __SAVEOPTION_H__
#define __SAVEOPTION_H__

#include "TCHAR.h"
//#include "windows.h"
#include "resource.h"


enum SaveOptTo {
  sFile,
  sReg
};


class SaveOption {
protected:
  SaveOptTo sOptTo;
  TCHAR *sFileName;
public:
	SaveOption(SaveOptTo sot, TCHAR *fname);
	~SaveOption();

  LSTATUS soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);

};

#endif // __SAVEOPTION_H__