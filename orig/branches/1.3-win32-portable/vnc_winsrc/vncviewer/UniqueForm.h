#pragma once

#include "stdhdrs.h"

#define _THIS		"_THIS"

class UniqueForm
{
protected:
	ATOM RegClass(HINSTANCE hInst, LPSTR lpzClassName);
public:
	UniqueForm(HINSTANCE hInst, TCHAR *WindowClassName);
	~UniqueForm(void);

	void WndProcForm(UniqueForm *_this, UINT message, WPARAM wParam, LPARAM lParam);

	HWND f_hwnd;
	HINSTANCE f_hinst;
};
