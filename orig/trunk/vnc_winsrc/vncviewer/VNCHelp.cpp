// VNCHelp.cpp: implementation of the VNCHelp class.
//
//////////////////////////////////////////////////////////////////////


#include "stdhdrs.h"
#include "Htmlhelp.h"
#include "vncviewer.h"
#include "VNCHelp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VNCHelp::VNCHelp()
{
	dwCookie = NULL;
	HtmlHelp(
         NULL,
         NULL,
         HH_INITIALIZE,
         (DWORD)&dwCookie) ; // Cookie returned by Hhctrl.ocx.
}

void VNCHelp::Popup(LPARAM lParam) 
{
	
	LPHELPINFO hlp = (LPHELPINFO) lParam;
	HH_POPUP popup;

	if (hlp->iCtrlId != 0) {
		
		popup.cbStruct = sizeof(popup);
		popup.hinst = pApp->m_instance;
		popup.idString = (UINT)hlp->iCtrlId;
		SetRect(&popup.rcMargins, -1, -1, -1, -1);
		popup.pszFont = "MS Sans Serif, 8, , ";
		popup.clrForeground = -1;
		popup.clrBackground = -1;
		popup.pt.x = -1;
		popup.pt.y = -1;

		switch  (hlp->iCtrlId) {
		case IDC_STATIC_LEVEL:
		case IDC_STATIC_TEXT_LEVEL:
		case IDC_STATIC_FAST:
		case IDC_STATIC_BEST:
			popup.idString = IDC_COMPRESSLEVEL;
			break;
		case IDC_STATIC_QUALITY:
		case IDC_STATIC_TEXT_QUALITY:
		case IDC_STATIC_POOR:
		case IDC_STATIC_QBEST:
			popup.idString = IDC_QUALITYLEVEL;
			break;
		case IDC_STATIC_ENCODING:
			popup.idString = IDC_ENCODING;
			break;
		case IDC_STATIC_SCALE:
		case IDC_STATIC_P:
			popup.idString = IDC_SCALE_EDIT;
			break;
		case IDC_STATIC_SERVER:
			popup.idString = IDC_HOSTNAME_EDIT;
			break;
		case IDC_STATIC_LIST:
		case IDC_SPIN1:
			popup.idString = IDC_EDIT_AMOUNT_LIST;
			break;
		case IDC_STATIC_LOG_LEVEL:
		case IDC_SPIN2:
			popup.idString = IDC_EDIT_LOG_LEVEL;
			break;
		case IDC_SPIN3:
		case IDC_STATIC_PORT:
			popup.idString = IDC_LISTEN_PORT;
			break;
		}

		HtmlHelp((HWND)hlp->hItemHandle,
					NULL,
					HH_DISPLAY_TEXT_POPUP,
					(DWORD)&popup) ;
	}
}
BOOL VNCHelp::TransMess( DWORD mess) 
{
	if (HtmlHelp (
                 NULL,
                 NULL,
                 HH_PRETRANSLATEMESSAGE,
                 mess)) {
		return TRUE;
	} else {
		return FALSE;
	}
}
VNCHelp::~VNCHelp()
{
	HtmlHelp(
         NULL,
         NULL,
         HH_UNINITIALIZE,
         (DWORD)dwCookie) ; // Pass in cookie.
}
