#include "UniqueForm.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
						 WPARAM wParam, LPARAM lParam)
{
	UniqueForm *_this;
	_this = (UniqueForm *) GetWindowLong(hWnd, GWL_USERDATA);
	if (_this != NULL) {
		if (_this->WndProcForm(_this, message, wParam, lParam)) {
			return 0;
		} 
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} 

UniqueForm::UniqueForm(HINSTANCE hInst, TCHAR *WindowClassName)
{
	// Create global window message for TightVNC control
	wm_ExitCode = RegisterWindowMessage(_T(GLOBAL_REG_MESSAGE));

	f_hinst = hInst;
	if (RegClass(f_hinst, WindowClassName) == 0) {
		return;
	}
	f_hwnd = CreateWindow(WindowClassName, "", 
		0, 0, 0, 1, 1, NULL, NULL, f_hinst, NULL);
	SetWindowLong(f_hwnd, GWL_USERDATA, (LONG) this);
}


UniqueForm::~UniqueForm(void)
{
}

ATOM UniqueForm::RegClass(HINSTANCE hInst, LPSTR lpzClassName)
{
	WNDCLASS wcWindowClass = {0};
	wcWindowClass.lpfnWndProc = WndProc;
	wcWindowClass.style = NULL; 
	wcWindowClass.hInstance = hInst; 
	wcWindowClass.lpszClassName = lpzClassName; 
	wcWindowClass.hCursor = NULL; 
	wcWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;  
	//
	return RegisterClass(&wcWindowClass);
}

bool UniqueForm::WndProcForm(UniqueForm *_this, UINT message, 
							 WPARAM wParam, LPARAM lParam)
{
	if (message == wm_ExitCode) {
		PostQuitMessage(0);
		return true;
	}
	return false;
} 

