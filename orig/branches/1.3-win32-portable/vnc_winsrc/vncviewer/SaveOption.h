#ifndef __SAVEOPTION_H__
#define __SAVEOPTION_H__

#include "TCHAR.h"
#include "stdhdrs.h"
#include "VirtualReg.h"


enum SaveOptTo {
	sFile,
	sReg
};


class SaveOption {
protected:
	SaveOptTo sOptTo;
	TCHAR *sFileName;
	VirtualReg *vReg;
public:
	SaveOption(SaveOptTo sot, TCHAR *fname);
	void ReInit(SaveOptTo sot, TCHAR *fname);
	~SaveOption();


	LSTATUS soRegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
  
};

#endif // __SAVEOPTION_H__