
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id$

#include "horizonNormalSettings.h"

//
// class implemtentation
//

bool 
horizonNormalSettings::Show( void )
{
	if ( m_dlgvisible == true )
		return true ;
	else
		m_dlgvisible = true ;
		
	int result = DialogBoxParam(
		hAppInstance,
		MAKEINTRESOURCE( IDD_NORMAL_CONTROLS ), 
		NULL,
		( DLGPROC )( DialogProc ),
		( LONG )( this )
	);
	
	// increment times through
	++m_times_through ;
	
	m_dlgvisible = false ;

	return ( result == -1 ) ? false : true ;
}

BOOL CALLBACK 
horizonNormalSettings::DialogProc(
	HWND hwnd, 
	UINT uMsg,
	WPARAM wParam, 
	LPARAM lParam
)
{
	// get the dialog's object
	horizonNormalSettings* _this = reinterpret_cast< horizonNormalSettings* >( 
		GetWindowLong( hwnd, GWL_USERDATA ) ) ;
	
	//
	// handle the message
	//
	
	switch ( uMsg )
	{
	
	// init the dialog
	case WM_INITDIALOG:
		{
			SetWindowLong( hwnd, GWL_USERDATA, lParam ) ;
			_this = reinterpret_cast< horizonNormalSettings* >( lParam ) ;
			
			// create the shared area panel
			_this->m_shareddtarea = new horizonSharedArea( 
				hwnd, _this->m_properties->GetMatchWindow() ) ;

			// reset okay flag
			_this->m_ok_clicked = false ;

//			InitCommonControls() ; // ??? deprecated win32 function ???

			return 0 ;
		}
	
	// handle menu commands
	case WM_COMMAND:
		{
		
		switch ( LOWORD( wParam ) )
		{
		
		//
		// shared region radio buttons
		//
		
		case IDC_FULLSCREEN:
			_this->m_shareddtarea->FullScreen() ;
			return TRUE ;
			
		case IDC_WINDOW:
			_this->m_shareddtarea->SharedWindow() ;
			return TRUE ;
			
		case IDC_SCREEN:
			_this->m_shareddtarea->SharedScreen() ;
			return TRUE ;
		
		// apply and make connection
		case IDOK:
			{

			// return if there was an error
			if ( ! _this->m_shareddtarea->ApplySharedControls() )
				return TRUE ;
				
			// set ok click flag
			_this->m_ok_clicked = true ;
			
			// close dialog
			EndDialog( hwnd, IDOK ) ;
			return TRUE ;

			} // IDOK

		case IDCANCEL:
			// reset the shared-area dialog
			_this->m_shareddtarea->CancelChanges() ;
			
			// end the dialog
			EndDialog( hwnd, IDCANCEL ) ;
			
			return TRUE ;

		default:
			break ;
			
		} // switch ( LOWORD( wParam ) )
		
		return FALSE;

		} // WM_COMMAND

	// clean up
	case WM_DESTROY:
		// delete shared window area
		if ( _this->m_shareddtarea != NULL )
		{
			delete _this->m_shareddtarea;
			_this->m_shareddtarea = NULL;
		}
		return FALSE ;

	default:
		break ;

	}

	return FALSE ;
}
