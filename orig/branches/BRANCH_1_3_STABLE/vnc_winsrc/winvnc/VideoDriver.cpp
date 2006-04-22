//  Copyright (C) 2005 Lev Kazarkin. All Rights Reserved.
//  Copyright (C) 2004 Vladimir Vologzhanin. All Rights Reserved.
//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

#include "VideoDriver.h"

char	vncVideoDriver::szDriverString[] = "Mirage Driver";
char	vncVideoDriver::szDriverStringAlt[] = "DemoForge Mirage Driver";
char	vncVideoDriver::szMiniportName[] = "dfmirage";

#define MINIPORT_REGISTRY_PATH	"SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services"


vncVideoDriver::vncVideoDriver()
{
	bufdata.buffer = NULL;
	bufdata.Userbuffer = NULL;
	m_fIsActive = false;
	m_fDirectAccessInEffect = false;
	*m_devname= 0;
}

vncVideoDriver::~vncVideoDriver()
{
	UnMapSharedbuffers();
	Deactivate();
	_ASSERTE(!m_fIsActive);
	_ASSERTE(!m_fDirectAccessInEffect);
}

#define	BYTE0(x)	((x) & 0xFF)
#define	BYTE1(x)	(((x) >> 8) & 0xFF)
#define	BYTE2(x)	(((x) >> 16) & 0xFF)
#define	BYTE3(x)	(((x) >> 24) & 0xFF)

BOOL vncVideoDriver::CheckVersion()
{
	HDC	l_gdc= ::CreateDC(m_devname, NULL, NULL, NULL);
	if (!l_gdc)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("vncVideoDriver::CheckVersion: can't create DC on \"%s\"\n"),
			m_devname);
		return FALSE;
	}

	Esc_dmf_Qvi_IN qvi_in;
	qvi_in.cbSize = sizeof(qvi_in);
	vnclog.Print(
		LL_INTINFO,
		"Supported driver version is: min = %u.%u.%u.%u, cur = %u.%u.%u.%u\n",
		BYTE3(DMF_PROTO_VER_MINCOMPAT), BYTE2(DMF_PROTO_VER_MINCOMPAT), BYTE1(DMF_PROTO_VER_MINCOMPAT), BYTE0(DMF_PROTO_VER_MINCOMPAT),
		BYTE3(DMF_PROTO_VER_CURRENT), BYTE2(DMF_PROTO_VER_CURRENT), BYTE1(DMF_PROTO_VER_CURRENT), BYTE0(DMF_PROTO_VER_CURRENT));
	qvi_in.app_actual_version = DMF_PROTO_VER_CURRENT;
	qvi_in.display_minreq_version = DMF_PROTO_VER_MINCOMPAT;
	qvi_in.connect_options = 0;

	Esc_dmf_Qvi_OUT qvi_out;
	qvi_out.cbSize = sizeof(qvi_out);

	int drvCr = ExtEscape(
		l_gdc,
		ESC_QVI,
		sizeof(qvi_in), (LPSTR) &qvi_in,
		sizeof(qvi_out), (LPSTR) &qvi_out);
	DeleteDC(l_gdc);

	if (drvCr == 0)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("vncVideoDriver::CheckVersion: ESC_QVI not supported by this version of Mirage\n"));
		return FALSE;
	}

	vnclog.Print(
		LL_INTINFO,
		"Driver version is: display = %u.%u.%u.%u (build %u),"
		" miniport = %u.%u.%u.%u (build %u),"
		" appMinReq = %u.%u.%u.%u\n",
		BYTE3(qvi_out.display_actual_version), BYTE2(qvi_out.display_actual_version), BYTE1(qvi_out.display_actual_version), BYTE0(qvi_out.display_actual_version),
		qvi_out.display_buildno, 
		BYTE3(qvi_out.miniport_actual_version), BYTE2(qvi_out.miniport_actual_version), BYTE1(qvi_out.miniport_actual_version), BYTE0(qvi_out.miniport_actual_version),
		qvi_out.miniport_buildno,
		BYTE3(qvi_out.app_minreq_version), BYTE2(qvi_out.app_minreq_version), BYTE1(qvi_out.app_minreq_version), BYTE0(qvi_out.app_minreq_version));

	if (drvCr < 0)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("vncVideoDriver::CheckVersion: ESC_QVI call returned 0x%x\n"),
			drvCr);
		return FALSE;
	}
	return TRUE;
}

BOOL vncVideoDriver::MapSharedbuffers(BOOL fForDirectScreenAccess)
{
	_ASSERTE(!m_fIsActive);
	_ASSERTE(!m_fDirectAccessInEffect);

	HDC	l_gdc= ::CreateDC(m_devname, NULL, NULL, NULL);
	if (!l_gdc)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("vncVideoDriver::MapSharedbuffers: can't create DC on \"%s\"\n"),
			m_devname);
		return FALSE;
	}

	oldCounter = 0;
	int drvCr = ExtEscape(
		l_gdc,
		MAP1,
		0, NULL,
		sizeof(GETCHANGESBUF), (LPSTR) &bufdata);
	DeleteDC(l_gdc);

	if (drvCr <= 0)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("vncVideoDriver::MapSharedbuffers: MAP1 call returned 0x%x\n"),
			drvCr);
		return FALSE;
	}
	m_fIsActive = true;
	if (fForDirectScreenAccess)
	{
		if (!bufdata.Userbuffer)
		{
			vnclog.Print(
				LL_INTERR,
				VNCLOG("vncVideoDriver::MapSharedbuffers: mirror screen view is NULL\n"));
			return FALSE;
		}
		m_fDirectAccessInEffect = true;
	}
	else
	{
		if (bufdata.Userbuffer)
		{
			vnclog.Print(
				LL_INTINFO,
				VNCLOG("vncVideoDriver::MapSharedbuffers: mirror screen view is mapped but direct access mode is OFF\n"));
		}
	}
	return TRUE;	
}

BOOL vncVideoDriver::TestMapped()
{
	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
		return FALSE;

	HDC	l_ddc = ::CreateDC((const char *)dd.DeviceName, NULL, NULL, NULL);
	if (l_ddc)
	{
		BOOL b = ExtEscape(l_ddc, TESTMAPPED, 0, NULL, 0, NULL);	
		DeleteDC(l_ddc);
		return b;
	}
	return FALSE;
}

void vncVideoDriver::UnMapSharedbuffers()
{
	int DrvCr = 0;

	if (m_devname[0])
	{
		_ASSERTE(m_fIsActive);
		_ASSERTE(bufdata.buffer);
		_ASSERTE(!m_fDirectAccessInEffect || bufdata.Userbuffer);

		HDC	l_gdc= ::CreateDC(m_devname, NULL, NULL, NULL);
		if (!l_gdc)
		{
			vnclog.Print(
				LL_INTERR,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: can't create DC on \"%s\"\n"),
				m_devname);
		}
		else
		{
			DrvCr = ExtEscape(
				l_gdc,
				UNMAP1,
				sizeof(GETCHANGESBUF), (LPSTR) &bufdata,
				0, NULL);
			DeleteDC(l_gdc);

			_ASSERTE(DrvCr > 0);
		}
	}
// 0 return value is unlikely for Mirage because its DC is independent
// from the reference device;
// this happens with Quasar if its mode was changed externally.
// nothing is particularly bad with it.

	if (DrvCr <= 0)
	{
		vnclog.Print(
			LL_INTINFO,
			VNCLOG("vncVideoDriver::UnMapSharedbuffers failed. unmapping manually\n"));
		if (bufdata.buffer)
		{
			BOOL br = UnmapViewOfFile(bufdata.buffer);
			vnclog.Print(
				LL_INTINFO,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: UnmapViewOfFile(bufdata.buffer) returned %d\n"),
				br);
		}
		if (bufdata.Userbuffer)
		{
			BOOL br = UnmapViewOfFile(bufdata.Userbuffer);
			vnclog.Print(
				LL_INTINFO,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: UnmapViewOfFile(bufdata.Userbuffer) returned %d\n"),
				br);
		}
	}
	m_fIsActive = false;
	m_fDirectAccessInEffect = false;
}

template <class TpFn>
HINSTANCE  LoadNImport(LPCTSTR szDllName, LPCTSTR szFName, TpFn &pfn)
{
	HINSTANCE hDll = LoadLibrary(szDllName);
	if (hDll)
	{
		pfn = (TpFn)GetProcAddress(hDll, szFName);
		if (pfn)
			return hDll;
		FreeLibrary(hDll);
	}
	vnclog.Print(
		LL_INTERR,
		VNCLOG("Can not import '%s' from '%s'.\n"),
		szFName, szDllName);
	return NULL;
}

//BOOL vncVideoDriver::LookupVideoDevice(LPCTSTR szDeviceString, INT &devNum, DISPLAY_DEVICE *pDd)
BOOL vncVideoDriver::LookupVideoDeviceAlt(
		LPCTSTR szDevStr,
		LPCTSTR szDevStrAlt,
		INT &devNum,
		DISPLAY_DEVICE *pDd)
{
	pEnumDisplayDevices pd = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "EnumDisplayDevicesA", pd);
	if (!hInstUser32) return FALSE;

	ZeroMemory(pDd, sizeof(DISPLAY_DEVICE));
	pDd->cb = sizeof(DISPLAY_DEVICE);
	BOOL result;
	while (result = (*pd)(NULL,devNum, pDd, 0))
	{
		if (strcmp((const char *)pDd->DeviceString, szDevStr) == 0 ||
			szDevStrAlt && strcmp((const char *)pDd->DeviceString, szDevStrAlt) == 0)
		{
			vnclog.Print(
				LL_INTINFO,
				VNCLOG("Found display device \"%s\": \"%s\"\n"),
				pDd->DeviceString,
				pDd->DeviceName);
			break;
		}
		devNum++;
	}

	FreeLibrary(hInstUser32);
	return result;
}

HKEY vncVideoDriver::CreateDeviceKey(LPCTSTR szMpName)
{
	HKEY hKeyProfileMirror = (HKEY)0;
	if (RegCreateKey(
			HKEY_LOCAL_MACHINE,
			(MINIPORT_REGISTRY_PATH),
			&hKeyProfileMirror) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't access registry.\n"));
		return FALSE;
	}
	HKEY hKeyProfileMp = (HKEY)0;
	LONG cr = RegCreateKey(
			hKeyProfileMirror,
			szMpName,
			&hKeyProfileMp);
	RegCloseKey(hKeyProfileMirror);
	if (cr != ERROR_SUCCESS)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("Can't access \"%s\" hardware profiles key.\n"),
			szMpName);
		return FALSE;
	}
	HKEY hKeyDevice = (HKEY)0;
	if (RegCreateKey(
			hKeyProfileMp,
			("DEVICE0"),
			&hKeyDevice) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't access DEVICE0 hardware profiles key.\n"));
	}
	RegCloseKey(hKeyProfileMp);
	return hKeyDevice;
}

BOOL vncVideoDriver::Activate(
		BOOL fForDirectAccess,
		const RECT *prcltarget)
{
	HDESK   hdeskInput;
    HDESK   hdeskCurrent;
 
	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
	{
		vnclog.Print(LL_INTERR, VNCLOG("No '%s' or '%s' found.\n"), szDriverString, szDriverStringAlt);
		return FALSE;
	}

	DEVMODE devmode;
	FillMemory(&devmode, sizeof(DEVMODE), 0);
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (prcltarget)
	{
// we always have to set position or
// a stale position info in registry would come into effect.
		devmode.dmFields |= DM_POSITION;
		devmode.dmPosition.x = prcltarget->left;
		devmode.dmPosition.y = prcltarget->top;

		devmode.dmPelsWidth = prcltarget->right - prcltarget->left;
		devmode.dmPelsHeight = prcltarget->bottom - prcltarget->top;
	}

	devmode.dmDeviceName[0] = '\0';

    vnclog.Print(LL_INTINFO, VNCLOG("DevNum:%d\nName:%s\nString:%s\n\n"), devNum, &dd.DeviceName[0], &dd.DeviceString[0]);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Top-Left Position: (%i, %i)\n"), devmode.dmPosition.x, devmode.dmPosition.y);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Dimensions: (%i, %i)\n"), devmode.dmPelsWidth, devmode.dmPelsHeight);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Color depth: %i\n"), devmode.dmBitsPerPel);

	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return FALSE;

// TightVNC does not use these features
	RegDeleteValue(hKeyDevice, ("Screen.ForcedBpp"));
	RegDeleteValue(hKeyDevice, ("Pointer.Enabled"));

	DWORD dwVal = fForDirectAccess ? 3 : 0;
// NOTE that old driver ignores it and mapping is always ON with it
	if (RegSetValueEx(
			hKeyDevice,
			("Cap.DfbBackingMode"),
			0,
			REG_DWORD,
			(unsigned char *)&dwVal,
			4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set \"Cap.DfbBackingMode\" to %d\n"), dwVal);
		return FALSE;
	}

	dwVal = 1;
	if (RegSetValueEx(
			hKeyDevice,
			("Attach.ToDesktop"),
			0,
			REG_DWORD,
			(unsigned char *)&dwVal,
			4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Attach.ToDesktop to 0x1\n"));
		return FALSE;
	}

	pChangeDisplaySettingsEx pCDS = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "ChangeDisplaySettingsExA", pCDS);
	if (!hInstUser32) return FALSE;

	// Save the current desktop
	hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
	if (hdeskCurrent != NULL)
	{
		hdeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
		if (hdeskInput != NULL) 
			SetThreadDesktop(hdeskInput);
	}
// 24 bpp screen mode is MUNGED to 32 bpp.
// the underlying buffer format must be 32 bpp.
// see vncDesktop::ThunkBitmapInfo()
	if (devmode.dmBitsPerPel==24) devmode.dmBitsPerPel = 32;
	LONG cr = (*pCDS)(
		(const char *)dd.DeviceName,
		&devmode,
		NULL,
		CDS_UPDATEREGISTRY,NULL);
	if (cr != DISP_CHANGE_SUCCESSFUL)
	{
		vnclog.Print(
			LL_INTERR,
			VNCLOG("ChangeDisplaySettingsEx failed on device \"%s\" with status: 0x%x\n"),
			dd.DeviceName,
			cr);
	}

	strcpy(m_devname, (const char *)dd.DeviceName);

	// Reset desktop
	SetThreadDesktop(hdeskCurrent);
	// Close the input desktop
	CloseDesktop(hdeskInput);
	RegCloseKey(hKeyDevice);
	FreeLibrary(hInstUser32);

	return TRUE;
}

void vncVideoDriver::Deactivate()
{
	HDESK   hdeskInput;
	HDESK   hdeskCurrent;
 
// it is important to us to be able to deactivate
// even what we have never activated. thats why we look it up, all over
//	if (!m_devname[0])
//		return;
// ... and forget the name
	*m_devname = 0;

	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
	{
		vnclog.Print(LL_INTERR, VNCLOG("No '%s' or '%s' found.\n"), szDriverString, szDriverStringAlt);
		return;
	}

	DEVMODE devmode;
	FillMemory(&devmode, sizeof(DEVMODE), 0);
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	devmode.dmDeviceName[0] = '\0';

	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return;

    DWORD one = 0;
	if (RegSetValueEx(hKeyDevice,("Attach.ToDesktop"), 0, REG_DWORD, (unsigned char *)&one,4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Attach.ToDesktop to 0x1\n"));
	}

// reverting to default behavior
	RegDeleteValue(hKeyDevice, ("Cap.DfbBackingMode"));

	pChangeDisplaySettingsEx pCDS = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "ChangeDisplaySettingsExA", pCDS);
	if (!hInstUser32) return;

	// Save the current desktop
	hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
	if (hdeskCurrent != NULL)
	{
		hdeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
		if (hdeskInput != NULL)
			SetThreadDesktop(hdeskInput);
	}
// 24 bpp screen mode is MUNGED to 32 bpp. see vncDesktop::ThunkBitmapInfo()
	if (devmode.dmBitsPerPel==24) devmode.dmBitsPerPel = 32;

	// Add 'Default.*' settings to the registry under above hKeyProfile\mirror\device
	(*pCDS)((const char *)dd.DeviceName, &devmode, NULL, CDS_UPDATEREGISTRY, NULL);
    
	// Reset desktop
	SetThreadDesktop(hdeskCurrent);
	// Close the input desktop
	CloseDesktop(hdeskInput);
	RegCloseKey(hKeyDevice);
	FreeLibrary(hInstUser32);
}

void vncVideoDriver::HandleDriverChanges(
		vncRegion &rgn,
		int xoffset,
		int yoffset,
		BOOL &bPointerShapeChange)
{
	if (oldCounter == bufdata.buffer->counter)
		return;

	if (oldCounter < bufdata.buffer->counter)
	{
		HandleDriverChangesSeries(
			rgn,
			xoffset, yoffset,
			bufdata.buffer->pointrect + oldCounter,
			bufdata.buffer->pointrect + bufdata.buffer->counter,
			bPointerShapeChange);
	}
	else
	{
		HandleDriverChangesSeries(
			rgn,
			xoffset, yoffset,
			bufdata.buffer->pointrect + oldCounter,
			bufdata.buffer->pointrect + MAXCHANGES_BUF,
			bPointerShapeChange);

		HandleDriverChangesSeries(
			rgn,
			xoffset, yoffset,
			bufdata.buffer->pointrect,
			bufdata.buffer->pointrect + bufdata.buffer->counter,
			bPointerShapeChange);
	}

	oldCounter = bufdata.buffer->counter;
}

void vncVideoDriver::HandleDriverChangesSeries(
		vncRegion &rgn,
		int xoffset,
		int yoffset,
		const CHANGES_RECORD *i,
		const CHANGES_RECORD *last,
		BOOL &bPointerShapeChange)
{
	for (; i < last; i++)
	{
// TODO dmf_dfo_SCREEN_SCREEN
// TODO: bPointerShapeChange

		if (i->type >= dmf_dfo_BLIT && i->type <= dmf_dfo_TEXTOUT)
		{
			rgn.AddRect(i->rect, xoffset, yoffset);
		}
	}
}
