#include "UniqueForm.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
						 WPARAM wParam, LPARAM lParam)
{
	UniqueForm *_this;
	_this = (UniqueForm *) GetWindowLong(hWnd, GWL_USERDATA);
	if (_this != NULL) {
		_this->WndProcForm(_this, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} 

UniqueForm::UniqueForm(HINSTANCE hInst, TCHAR *WindowClassName)
{
	f_hinst = hInst;
	if (RegClass(f_hinst, WindowClassName) == 0) {
		return;
	}
	f_hwnd = CreateWindow(WindowClassName, "", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 300, 150, NULL, NULL, f_hinst, NULL);
	SetWindowLong(f_hwnd, GWL_USERDATA, (LONG) this);
}


UniqueForm::~UniqueForm(void)
{
}

ATOM UniqueForm::RegClass(HINSTANCE hInst, LPSTR lpzClassName)
{
	WNDCLASS wcWindowClass = {0};
	wcWindowClass.lpfnWndProc = WndProc;
	wcWindowClass.style = CS_HREDRAW|CS_VREDRAW; 
	wcWindowClass.hInstance = hInst; 
	wcWindowClass.lpszClassName = lpzClassName; 
	wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wcWindowClass.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;  
	//
	return RegisterClass(&wcWindowClass);
}

void UniqueForm::WndProcForm(UniqueForm *_this, UINT message, 
							 WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return;
	}
	return ;
} 

