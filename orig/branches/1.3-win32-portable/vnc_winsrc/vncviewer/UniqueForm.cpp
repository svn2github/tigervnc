#include "UniqueForm.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
						 WPARAM wParam, LPARAM lParam)
{
	// выборка и обработка сообщений
	UniqueForm *_this;
	_this = (UniqueForm *) GetProp(hWnd, _THIS);
	if (_this != NULL) {
		_this->WndProcForm(_this, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} // конец функции обработчика сообщений


UniqueForm::UniqueForm(TCHAR *WindowClassName, HINSTANCE hInst)
{
	f_hinst = hInst;
	RegClass(f_hinst, WindowClassName);
	hwnd = CreateWindow(WindowClassName, "", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 300, 150, NULL, NULL, f_hinst, NULL);

	SetProp(hwnd, _THIS, this);
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

	//регистрация класса
	return RegisterClass(&wcWindowClass);
}

void UniqueForm::WndProcForm(UniqueForm *_this, UINT message, 
							 WPARAM wParam, LPARAM lParam)
{
	// выборка и обработка сообщений
	switch (message)
	{
	case WM_LBUTTONUP:
		TCHAR mess[16];
		MessageBox(hwnd, ltoa((long) this, mess, 16) ,"Log",0); 
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return;
	}
	return ;
} 

