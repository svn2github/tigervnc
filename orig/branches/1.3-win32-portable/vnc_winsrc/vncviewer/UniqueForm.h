#pragma once

#include "stdhdrs.h"

#define GLOBAL_REG_MESSAGE	"exit.code.tightvnc"

class UniqueForm
{
protected:
	ATOM RegClass(HINSTANCE hInst, LPSTR lpzClassName);
	UINT wm_ExitCode;
public:
	UniqueForm(HINSTANCE hInst, TCHAR *WindowClassName);
	~UniqueForm(void);

	bool WndProcForm(UniqueForm *_this, UINT message, WPARAM wParam, LPARAM lParam);

	HWND f_hwnd;
	HINSTANCE f_hinst;
};
