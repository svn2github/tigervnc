#include "SaveOption.h"

SaveOption::SaveOption(SaveOptTo sot, TCHAR *fname){
	sOptTo = sot;
	sFileName = fname;
	vReg = new VirtualReg;
}

void SaveOption::ReInit(SaveOptTo sot, TCHAR *fname) {
	sOptTo = sot;
	sFileName = fname;
}

SaveOption::~SaveOption() {
	delete vReg;
}

LSTATUS SaveOption::soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData) {
	if (sOptTo == sReg) {
		return RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	} else {
		// Save to file
		return 0;
	}
}
