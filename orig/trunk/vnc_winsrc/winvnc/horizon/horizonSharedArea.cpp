
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id$

#include "horizonSharedArea.h"

//
// class implementation
//

horizonSharedArea::horizonSharedArea( HWND hwnd, CMatchWindow *matchwindow ) 
	: m_server( vncServerSingleton::GetInstance() )
{
	m_hwnd = hwnd;
	
	m_bCaptured = FALSE;
	m_KeepHandle = NULL;

	// if a matchwindow wasn't passed in, 
	// we're responsible for it
	if ( matchwindow == NULL )
	{
		m_pMatchWindow = NULL ;
		m_deleteMatchWindow = true ;
	}
	else
	{
		m_pMatchWindow = matchwindow ;
		m_deleteMatchWindow = false ;
	}

	// set to false by default
	m_pref_WindowShared = FALSE ;
	m_pref_BlackRgn = FALSE ;
	m_pref_FullScreen = FALSE ;
	m_pref_ScreenAreaShared = FALSE ;

	Init() ;
}

horizonSharedArea::~horizonSharedArea()
{
	if ( 
		m_pMatchWindow != NULL 
		&& m_deleteMatchWindow == true 
	)
	{
		delete m_pMatchWindow ;
		m_pMatchWindow = NULL ;
	}
}

void horizonSharedArea::Init()
{
	//
	// get settings from server
	//

	SetPrefFullScreen( m_server->FullScreen() ) ;
	SetPrefWindowShared( m_server->WindowShared() ) ;
	SetPrefScreenAreaShared( m_server->ScreenAreaShared() ) ;
	SetPrefBlackRgn( m_server->GetBlackRgn() ) ;
	
	//
	//  configure select window picture
	//
	
	HWND bmp_hWnd = GetDlgItem( m_hwnd, IDC_BMPCURSOR ) ;
	m_OldBmpWndProc = GetWindowLong( bmp_hWnd, GWL_WNDPROC ) ;
	SetWindowLong( bmp_hWnd, GWL_WNDPROC, (LONG)(BmpWndProc) ) ;
	SetWindowLong( bmp_hWnd, GWL_USERDATA, (LONG)(this) ) ;

	//
	// setup match window
	//
	
	if ( m_pMatchWindow == NULL ) 
	{
		// get the desktop's bounds
		RECT desktopRect ;
		GetWindowRect( GetDesktopWindow(), &desktopRect ) ;

		//
		// instantiate object with default bounds
		// ( i.e. top-left quadrant of the desktop )
		//
		
		m_pMatchWindow = new CMatchWindow( 
			m_server, 
			desktopRect.left+5, desktopRect.top+5, 
			desktopRect.right/2, desktopRect.bottom/2
		) ;

		//
		// if region sharing is the current setting,
		// the server rect is not whole screen, 
		// and the server rect is not empty, 
		// then use the server's bounds
		//
		
		if ( GetPrefScreenAreaShared() )
		{
			// get the server's current rect
			RECT serverRect ;
			serverRect = m_server->GetSharedRect() ;

			if ( 
				( EqualRect( &desktopRect, &serverRect ) == FALSE )
				&& ( IsRectEmpty( &serverRect ) == FALSE )
			)
			{
				// use it for match window
				m_pMatchWindow->ModifyPosition( 
					serverRect.left, serverRect.top, 
					serverRect.right, serverRect.bottom 
				) ;
			}
		}
	}

	m_pMatchWindow->CanModify( TRUE ) ;
	m_pMatchWindow->Hide() ;

	//
	// clear values
	//

	// full desktop
	HWND hFullScreen = GetDlgItem(m_hwnd, IDC_FULLSCREEN);
    SendMessage( hFullScreen, BM_SETCHECK, GetPrefFullScreen(), 0 ) ;

	// window
	HWND hWindowCaption = GetDlgItem(m_hwnd, IDC_WINDOW);
	SendMessage( hWindowCaption, BM_SETCHECK, GetPrefWindowShared(), 0 ) ;

	// window name
	hNameAppli = GetDlgItem( m_hwnd, IDC_NAME_APPLI ) ;
	EnableWindow( hNameAppli, GetPrefWindowShared() ) ;

	// screen area
	HWND hScreenCaption = GetDlgItem(m_hwnd, IDC_SCREEN);
	SendMessage( hScreenCaption, BM_SETCHECK, GetPrefScreenAreaShared(), 0 ) ;

	// black-out other windows
	HWND hBlackRgn = GetDlgItem(m_hwnd, IDC_CHECK_BLACK_RGN);
	SendMessage( hBlackRgn, BM_SETCHECK, GetPrefBlackRgn(), 0 ) ;

	//
	// toggle selected option
	//

	if ( GetPrefFullScreen() )
	{
		FullScreen() ;
	}
	else if ( GetPrefScreenAreaShared() ) 
	{
		SharedScreen() ;
	}
	else // if ( GetPrefWindowShared() ) 
	{
		// default
		
		// select radio button
		SendMessage( hWindowCaption, BM_SETCHECK, TRUE, 0 ) ;
		
		// run shared window helper
		SharedWindow() ;
	}

	// bring dialog to the front
	SetForegroundWindow( m_hwnd ) ;
	
	return ;
}

bool 
horizonSharedArea::ApplySharedControls()
{
	if (
		GetPrefWindowShared() 
		&& m_server->GetWindowShared() == NULL
	) 
	{	
		MessageBox(NULL,
				"You have not yet selected a window to share.\n"
				"Please first select a window with the 'Window Target'\n"
				"icon, and try again.", "No Window Selected",
				 MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//
	// set server's share type
	//

	// full screen
	HWND hFullScreen = GetDlgItem( m_hwnd, IDC_FULLSCREEN ) ;
	m_server->FullScreen( SendMessage( hFullScreen, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;

	// window 
	HWND hWindowCapture = GetDlgItem( m_hwnd, IDC_WINDOW ) ;
	m_server->WindowShared( SendMessage( hWindowCapture, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;

	// screen area
	HWND hScreenArea = GetDlgItem( m_hwnd, IDC_SCREEN ) ;
	m_server->ScreenAreaShared( SendMessage( hScreenArea, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;
	
	// unsupported
	HWND hBlackRgn = GetDlgItem( m_hwnd, IDC_CHECK_BLACK_RGN ) ;
	if ( SendMessage( hBlackRgn, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
	{
		m_server->SetBlackRgn( TRUE ) ;
		m_server->SetNewFBSize( TRUE ) ;
	} 
	else 
	{
		m_server->SetBlackRgn( FALSE ) ;
	}

	//
	// set server's match size
	//

	if ( GetPrefScreenAreaShared() ) 
	{
		int left, right, top, bottom ;
		m_pMatchWindow->GetPosition( left, right, top, bottom ) ;
		m_server->SetMatchSizeFields( left, right, top, bottom ) ;
	}
	else if ( GetPrefFullScreen() ) 
	{
		RECT temp ;
		GetWindowRect( GetDesktopWindow(), &temp ) ;
		m_server->SetMatchSizeFields( temp.left, temp.top, temp.right, temp.bottom ) ;
	}

	return true;
}

void horizonSharedArea::FullScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);
	
	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);
				
	// set window text name
	EnableWindow(hNameAppli, FALSE);
	::SetWindowText(hNameAppli, "Full Desktop Selected");
			
	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	SetPrefFullScreen( TRUE ) ;
	SetPrefWindowShared( FALSE ) ;
	SetPrefScreenAreaShared( FALSE ) ;

	return ;
}

void horizonSharedArea::SharedWindow()
{
	// enable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, TRUE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP1));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(hNameAppli, FALSE);
	SetWindowCaption(m_server->GetWindowShared());
	EnableWindow(hNameAppli, TRUE);

	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	SetPrefFullScreen(FALSE);
	SetPrefWindowShared(TRUE);
	SetPrefScreenAreaShared(FALSE);
	
	return ;
}

void horizonSharedArea::SharedScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(hNameAppli, FALSE);
	::SetWindowText(hNameAppli, "Screen Area Selected");
	
	// show match window
	m_pMatchWindow->Show();
	
	// update properties
	SetPrefFullScreen(FALSE);
	SetPrefWindowShared(FALSE);
	SetPrefScreenAreaShared(TRUE);

	return ;
}

LRESULT CALLBACK horizonSharedArea::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hNewImage, hOldImage;
	HCURSOR hNewCursor, hOldCursor;
	horizonSharedArea* pDialog = (horizonSharedArea*) GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) {

	case WM_SETCURSOR :
		if (HIWORD(lParam) == WM_LBUTTONDOWN) {
			SetCapture(hWnd);
			hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP2));
			hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
			DeleteObject(hOldImage);
			hNewCursor = (HCURSOR)LoadImage(hAppInstance, MAKEINTRESOURCE(IDC_CURSOR1),
											IMAGE_CURSOR, 32, 32, LR_DEFAULTCOLOR);
			hOldCursor = SetCursor(hNewCursor);
			DestroyCursor(hOldCursor);
			pDialog->m_bCaptured = TRUE;
		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		
		if (pDialog->m_KeepHandle != NULL) {
			// We need to remove frame
			DrawFrameAroundWindow(pDialog->m_KeepHandle);
			pDialog->m_server->SetWindowShared(pDialog->m_KeepHandle);
			// No more need
			pDialog->m_KeepHandle = NULL;
		} else {
			pDialog->m_server->SetWindowShared(NULL);
		}

		hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		hNewCursor = LoadCursor(hAppInstance, MAKEINTRESOURCE(IDC_ARROW));
		hOldCursor = SetCursor(hNewCursor);
		DestroyCursor(hOldCursor);
		pDialog->m_bCaptured = FALSE;
		break;
	
	case WM_MOUSEMOVE:
		if (pDialog->m_bCaptured) {
			HWND hParent = ::GetParent(hWnd);
			POINTS pnt;
			POINT pnt1;
			pnt = MAKEPOINTS(lParam);
			pnt1.x = pnt.x;
			pnt1.y = pnt.y;
			ClientToScreen(hWnd, &pnt1);
			HWND hMouseWnd=::WindowFromPoint(pnt1);
			if (pDialog->m_KeepHandle != hMouseWnd) {
				// New Windows Handle
				// Was KeepHndle A Real Window ?
				if (pDialog->m_KeepHandle != NULL) {
					// We need to remove frame
					horizonSharedArea::DrawFrameAroundWindow(pDialog->m_KeepHandle);
				}
				pDialog->m_KeepHandle = hMouseWnd;
				if (!IsChild(hParent, hMouseWnd) && hMouseWnd != hParent) {
					pDialog->SetWindowCaption(hMouseWnd);
					horizonSharedArea::DrawFrameAroundWindow(hMouseWnd);
				} else {	// My Window
					pDialog->m_KeepHandle = NULL;
					pDialog->SetWindowCaption(NULL);
				}
			}
		}
		break;

	case WM_PAINT:
	case STM_SETIMAGE:
		return CallWindowProc((WNDPROC)pDialog->m_OldBmpWndProc,
							  hWnd, message, wParam, lParam);
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


void horizonSharedArea::DrawFrameAroundWindow(HWND hWnd)
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
    
    return ;
}

void horizonSharedArea::SetWindowCaption(HWND hWnd)
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

	if ( hNameAppli )
		::SetWindowText(hNameAppli, strWindowText) ;
		
	return ;
}

