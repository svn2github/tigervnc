// SharedDesktopArea.cpp: implementation of the SharedDesktopArea class.
//
//////////////////////////////////////////////////////////////////////
#include "WinVNC.h"
#include "vncProperties.h"
#include "SharedDesktopArea.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SharedDesktopArea::SharedDesktopArea(HWND hwnd, CMatchWindow *Matchwindow,
						vncProperties *vncprop, vncServer *server)
{
	m_pMatchWindow = Matchwindow;
	m_vncprop = vncprop;
	m_server = server;
	m_bCaptured= FALSE;
	m_KeepHandle = NULL;
	
	Init(hwnd);

}
void SharedDesktopArea::Init(HWND hwnd)
{
	HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
	m_OldBmpWndProc=GetWindowLong(bmp_hWnd,GWL_WNDPROC);
	SetWindowLong(bmp_hWnd,GWL_WNDPROC,(LONG)BmpWndProc);
	SetWindowLong(bmp_hWnd, GWL_USERDATA, (LONG)this);
	HBITMAP hNewImage,hOldImage;
	hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);
	if (m_vncprop->GetPrefFullScreen()) {
		// Hide shared area window
		if (m_pMatchWindow!=NULL) 
			m_pMatchWindow->Hide();

		// Disable window select stuff
		EnableWindow(bmp_hWnd,FALSE);
		hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
		hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		::SetWindowText(hNameAppli,"Full Desktop Selected");                                                                                                  
	}

	if(m_vncprop->GetPrefWindowShared()) {

		EnableWindow(bmp_hWnd,TRUE);
		hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);	
		SetWindowCaption(m_server->GetWindowShared());

		if (m_pMatchWindow!=NULL) 
			m_pMatchWindow->Hide();
	
	}
			
	if (m_vncprop->GetPrefScreenAreaShared()) { 
		EnableWindow(bmp_hWnd,FALSE);
		hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
		hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		::SetWindowText(hNameAppli,"Screen Area Selected");

		if (m_pMatchWindow == NULL) {
			RECT temp;
			temp = m_server->getSharedRect();
			m_pMatchWindow=new CMatchWindow(m_server,temp.left+5,temp.top+5,temp.right,temp.bottom);
			m_pMatchWindow->CanModify(TRUE);
		}
		m_pMatchWindow->Show();
	}
				

	HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
    SendMessage(hFullScreen,    
			    BM_SETCHECK,
                m_vncprop->GetPrefFullScreen(),   0);

	EnableWindow(hNameAppli,(m_vncprop->GetPrefWindowShared()));

	HWND hWindowCaption = GetDlgItem(hwnd, IDC_WINDOW);
	SendMessage(hWindowCaption,
				BM_SETCHECK, 
				m_vncprop->GetPrefWindowShared(),0);

	HWND hScreenCaption = GetDlgItem(hwnd, IDC_SCREEN);
	SendMessage(hScreenCaption,
				BM_SETCHECK, 
				(m_vncprop->GetPrefScreenAreaShared()),0);

	SetForegroundWindow(hwnd);
}
bool SharedDesktopArea::ApplySharedControls(HWND hwnd)
{
	if ( m_vncprop->GetPrefWindowShared() && (m_server->GetWindowShared() == NULL) ) {	
		MessageBox(NULL,
				"You have not yet selected a window to share.\n"
				"Please first select a window with the 'Window Target'\n"
				"icon, and try again.", "No Window Selected",
				 MB_OK | MB_ICONEXCLAMATION);
		return true;
	}

	// Handle the share one window stuff
	HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
	m_server->FullScreen(SendMessage(hFullScreen,
								BM_GETCHECK, 0, 0) == BST_CHECKED);
				  				
	HWND hWindowCapture = GetDlgItem(hwnd, IDC_WINDOW);
	m_server->WindowShared(SendMessage(hWindowCapture,
								BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hScreenArea = GetDlgItem(hwnd, IDC_SCREEN);
	m_server->ScreenAreaShared(SendMessage(hScreenArea,
								BM_GETCHECK, 0, 0) == BST_CHECKED);
				
	if ( m_vncprop->GetPrefScreenAreaShared()) {
		int left,right,top,bottom;
		if (m_pMatchWindow!=NULL) {
			m_pMatchWindow->GetPosition(left,top,right,bottom);
			m_server->SetMatchSizeFields(left,top,right,bottom);
		}
	}

	if (m_vncprop->GetPrefFullScreen()) {
		RECT temp;
		GetWindowRect(GetDesktopWindow(), &temp);
		m_server->SetMatchSizeFields(temp.left, temp.top, temp.right, temp.bottom);
	}
	return false;
}
void SharedDesktopArea::FullScreen(HWND hwnd)
{
	HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
	HBITMAP hNewImage,hOldImage;

	m_vncprop->SetPrefFullScreen(TRUE);
	m_vncprop->SetPrefWindowShared(FALSE);
	m_vncprop->SetPrefScreenAreaShared(FALSE);
	hNameAppli = GetDlgItem(hwnd, IDC_NAME_APPLI);
	EnableWindow(hNameAppli, FALSE);
	::SetWindowText(hNameAppli,"Full Desktop Selected");
			
	EnableWindow(bmp_hWnd,FALSE);
	hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
	hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);
				
	if (m_pMatchWindow!=NULL) 
		m_pMatchWindow->Hide();
}
void SharedDesktopArea::SharedWindow(HWND hwnd)
{
	HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
	HBITMAP hNewImage,hOldImage;
	EnableWindow(bmp_hWnd,TRUE);
	hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
	hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);	
	if (m_pMatchWindow!=NULL) 
		m_pMatchWindow->Hide();
			   
	SetWindowCaption(m_server->GetWindowShared());
	EnableWindow(GetDlgItem(hwnd,IDC_NAME_APPLI),TRUE);
	m_vncprop->SetPrefFullScreen(FALSE);
	m_vncprop->SetPrefWindowShared(TRUE);
	m_vncprop->SetPrefScreenAreaShared(FALSE);
}
void SharedDesktopArea::SharedScreen(HWND hwnd)
{
	if (m_pMatchWindow == NULL) {
		RECT temp;
		temp = m_server->getSharedRect();
		m_pMatchWindow=new CMatchWindow(m_server,temp.left+5,temp.top+5,temp.right,temp.bottom);
		m_pMatchWindow->CanModify(TRUE);
	}

	HWND bmp_hWnd=GetDlgItem(hwnd,IDC_BMPCURSOR);
	HBITMAP hNewImage,hOldImage;
	::SetWindowText(hNameAppli,"Screen Area Selected");
	m_vncprop->SetPrefWindowShared(FALSE);
	EnableWindow(bmp_hWnd,FALSE);
	hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP3));
	hOldImage=(HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);
	if (m_pMatchWindow!=NULL) 
		m_pMatchWindow->Show();
	EnableWindow(GetDlgItem(hwnd,IDC_NAME_APPLI),FALSE);
	m_vncprop->SetPrefFullScreen(FALSE);
	m_vncprop->SetPrefWindowShared(FALSE);
	m_vncprop->SetPrefScreenAreaShared(TRUE);
}
LRESULT CALLBACK SharedDesktopArea::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hNewImage,hOldImage;
	HCURSOR hNewCursor,hOldCursor;
	SharedDesktopArea* pDialog=(SharedDesktopArea*) GetWindowLong(hWnd,GWL_USERDATA);

	switch (message)
	{
	
	case WM_SETCURSOR :
		if (HIWORD(lParam)==WM_LBUTTONDOWN)
		{
			SetCapture(hWnd);
			hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP2));
			hOldImage=(HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
			DeleteObject(hOldImage);
			hNewCursor=(HCURSOR)LoadImage(hAppInstance,MAKEINTRESOURCE(IDC_CURSOR1),IMAGE_CURSOR,32,32,LR_DEFAULTCOLOR);
			hOldCursor=SetCursor(hNewCursor);
			DestroyCursor(hOldCursor);
			pDialog->m_bCaptured=TRUE;
		}
		break;
	
	case WM_LBUTTONUP:
		ReleaseCapture();
		
		if (pDialog->m_KeepHandle!=NULL)
		{
			// We need to remove frame
			DrawFrameAroundWindow(pDialog->m_KeepHandle);
			pDialog->m_server->SetWindowShared(pDialog->m_KeepHandle);
			// No more need
			pDialog->m_KeepHandle=NULL;
		} else {
			pDialog->m_server->SetWindowShared(NULL);
		}
		
		hNewImage=LoadBitmap(hAppInstance,MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage=(HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		hNewCursor=LoadCursor(hAppInstance,MAKEINTRESOURCE(IDC_ARROW));
		hOldCursor=SetCursor(hNewCursor);
		DestroyCursor(hOldCursor);
		pDialog->m_bCaptured=FALSE;
		break;
	
	case WM_MOUSEMOVE:
		if (pDialog->m_bCaptured)
		{
			HWND hParent=::GetParent(hWnd);
			POINTS pnt;
			POINT pnt1;
			pnt=MAKEPOINTS(lParam);
			pnt1.x=pnt.x;
			pnt1.y=pnt.y;
			ClientToScreen(hWnd,&pnt1);
			HWND hMouseWnd=::WindowFromPoint(pnt1);
			if (pDialog->m_KeepHandle!=hMouseWnd)
			{
				// New Windows Handle
				// Was KeepHndle A Real Window ?
				if (pDialog->m_KeepHandle!=NULL)
				{
					// We need to remove frame
					SharedDesktopArea::DrawFrameAroundWindow(pDialog->m_KeepHandle);
				}
				pDialog->m_KeepHandle=hMouseWnd;
				if (!IsChild(hParent,hMouseWnd) && (hMouseWnd!=hParent))
				{
					pDialog->SetWindowCaption(hMouseWnd);
					SharedDesktopArea::DrawFrameAroundWindow(hMouseWnd);
				} else {	// My Window
					pDialog->m_KeepHandle=NULL;
					pDialog->SetWindowCaption(NULL);					}
				}
			}
			break;
	case WM_PAINT:
	case STM_SETIMAGE:
		
		return CallWindowProc( (WNDPROC) pDialog->m_OldBmpWndProc,  hWnd, message, wParam, lParam);
	
	default:
		return DefWindowProc( hWnd, message, wParam, lParam);
	}
	return 0;
}


void SharedDesktopArea::DrawFrameAroundWindow(HWND hWnd)
{
	HDC hWindowDc=::GetWindowDC(hWnd);
	HBRUSH hBrush=CreateSolidBrush(RGB(0,0,0));
	HRGN Rgn=CreateRectRgn(0,0,1,1);
	int iRectResult=GetWindowRgn(hWnd,Rgn);
	if (iRectResult==ERROR || iRectResult==NULLREGION || Rgn==NULL)
	{
		RECT rect;
		GetWindowRect(hWnd,&rect);
		OffsetRect(&rect,-rect.left,-rect.top);
		Rgn=CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	}
	
	SetROP2(hWindowDc,R2_MERGEPENNOT);
	FrameRgn(hWindowDc,Rgn,hBrush,3,3);
	::DeleteObject(Rgn);
	::DeleteObject(hBrush);
    ::ReleaseDC(hWnd,hWindowDc);
}

void SharedDesktopArea::SetWindowCaption(HWND hWnd)
{
	char strWindowText[256];
	if (hWnd == NULL) 
	{
		strcpy(strWindowText, "* No Window Selected *");
	} else {
		GetWindowText(hWnd, strWindowText, sizeof(strWindowText));
		if (!strWindowText[0]) 
		{
			int bytes = sprintf(strWindowText, "0x%x ", hWnd);
			GetClassName(hWnd, strWindowText + bytes,
				sizeof(strWindowText) - bytes);
		}
	}
	::SetWindowText(hNameAppli, strWindowText);
}

SharedDesktopArea::~SharedDesktopArea()
{

}
