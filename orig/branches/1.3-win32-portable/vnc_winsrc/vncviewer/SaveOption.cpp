#include "SaveOption.h"

SaveOption::SaveOption(SaveOptTo sot, TCHAR *fname){
  sOptTo = sot;
  sFileName = fname;
}

SaveOption::~SaveOption() {

}

LSTATUS SaveOption::soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData) {
  if (sOptTo == sReg) {
    return RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
  } else {
    // Save to ini file
    char buf[4];
	  sprintf(buf, "%d", value); 
	  WritePrivateProfileString("options", name, buf, fname);
    return 0;
  }
}
