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

void VNCHelp::Popup(LPARAM lParam) {
	
	LPHELPINFO hlp = (LPHELPINFO) lParam;
	HH_POPUP popup;

	if (hlp->iCtrlId != 0) {
		
		popup.cbStruct=sizeof(popup);
		popup.hinst=pApp->m_instance;
		popup.idString=(UINT)hlp->iCtrlId;
		SetRect(&popup.rcMargins, -1, -1, -1, -1);
		popup.pszFont="MS Sans Serif,8,,";
		popup.clrForeground =-1;
		popup.clrBackground=-1;
		popup.pt.x=-1;
		popup.pt.y=-1;

		HtmlHelp((HWND)hlp->hItemHandle,
					NULL,
					HH_DISPLAY_TEXT_POPUP,
					(DWORD)&popup) ;
	}
}
BOOL VNCHelp::TransMess( DWORD mess) {
	if (HtmlHelp (
                 NULL,
                 NULL,
                 HH_PRETRANSLATEMESSAGE,
                 mess)) {
		return TRUE;
	}else{
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
