// FileTransfer.cpp: implementation of the FileTransfer class.
//
//////////////////////////////////////////////////////////////////////

#include "vncviewer.h"
#include "FileTransfer.h"

int 
CompareFTItemInfo(const void *F, const void *S)
{
	if(strcmp(((FTITEMINFO *)F)->Size, ((FTITEMINFO *)S)->Size) == 0) {
		return stricmp(((FTITEMINFO *)F)->Name, ((FTITEMINFO *)S)->Name);
	} else {
		if(strcmp(((FTITEMINFO *)F)->Size, "Folder") == 0) return -1;
		if(strcmp(((FTITEMINFO *)S)->Size, "Folder") == 0) {
			return 1;
		} else {
		return stricmp(((FTITEMINFO *)F)->Name, ((FTITEMINFO *)S)->Name);
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileTransfer::FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp)
{
	m_clientconn = pCC;
	m_pApp = pApp;
	m_TransferEnable = FALSE;
	m_bServerBrowseRequest = FALSE;
	m_hTreeItem = NULL;
	m_FTClientItemInfo = NULL;
	m_FTServerItemInfo = NULL;
}

FileTransfer::~FileTransfer()
{
	if(m_FTClientItemInfo != NULL) delete [] m_FTClientItemInfo;
	if(m_FTServerItemInfo != NULL) delete [] m_FTServerItemInfo;
}
void 
FileTransfer::CreateFileTransferDialog()
{
	m_hwndFileTransfer = CreateDialog(m_pApp->m_instance, 
									  MAKEINTRESOURCE(IDD_FILETRANSFER_DLG),
									  NULL, 
									  (DLGPROC) FileTransferDlgProc); 
	ShowWindow(m_hwndFileTransfer, SW_SHOW);
	UpdateWindow(m_hwndFileTransfer);
	SetWindowLong(m_hwndFileTransfer, GWL_USERDATA, (LONG) this);

	m_hwndFTProgress = GetDlgItem(m_hwndFileTransfer, IDC_FTPROGRESS);
	m_hwndFTClientList = GetDlgItem(m_hwndFileTransfer, IDC_FTCLIENTLIST);
	m_hwndFTServerList = GetDlgItem(m_hwndFileTransfer, IDC_FTSERVERLIST);
	m_hwndFTClientPath = GetDlgItem(m_hwndFileTransfer, IDC_CLIENTPATH);
	m_hwndFTServerPath = GetDlgItem(m_hwndFileTransfer, IDC_SERVERPATH);
	m_hwndFTStatus = GetDlgItem(m_hwndFileTransfer, IDC_FTSTATUS);
	HANDLE hIcon = LoadImage(m_pApp->m_instance, MAKEINTRESOURCE(IDI_FILEUP), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTUP), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_SERVERUP), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	DestroyIcon((HICON) hIcon);
	hIcon = LoadImage(m_pApp->m_instance, MAKEINTRESOURCE(IDI_FILERELOAD), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTRELOAD), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	SendMessage(GetDlgItem(m_hwndFileTransfer, IDC_SERVERRELOAD), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	DestroyIcon((HICON) hIcon);

	RECT Rect;
	GetClientRect(m_hwndFTClientList, &Rect);
	int xwidth = Rect.right;
	int xwidth_ = (int) (0.75 * xwidth);

    LVCOLUMN lvc; 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	lvc.fmt = LVCFMT_CENTER;
	lvc.iSubItem = 0;
    lvc.pszText = "Name";	
	lvc.cchTextMax = 10;
    lvc.cx = xwidth_;
	lvc.iOrder = 0;
    ListView_InsertColumn(m_hwndFTClientList, 0, &lvc);
    ListView_InsertColumn(m_hwndFTServerList, 0, &lvc);

	lvc.iSubItem = 1;
	lvc.pszText = "Size";	
	lvc.cx = xwidth - xwidth_;
	lvc.iOrder = 1;
    ListView_InsertColumn(m_hwndFTClientList, 1, &lvc);
    ListView_InsertColumn(m_hwndFTServerList, 1, &lvc);

	strcpy(m_ClientPath, "");
	strcpy(m_ClientPathTmp, "");
	strcpy(m_ServerPath, "");
	strcpy(m_ServerPathTmp, "");
	ShowClientItems("");
	SendFileListRequestMessage("");
}

LRESULT CALLBACK 
FileTransfer::FileTransferDlgProc(HWND hwnd, 
								  UINT uMsg, 
								  WPARAM wParam, 
								  LPARAM lParam)
{
	FileTransfer *_this = (FileTransfer *) GetWindowLong(hwnd, GWL_USERDATA);
	int i;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			SetForegroundWindow(hwnd);
			CentreWindow(hwnd);
			return TRUE;
		}
	break;

	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
			case IDC_EXIT:
				_this->m_clientconn->m_FileTransferEnable = false;
				if(_this->m_FTClientItemInfo != NULL) {
					delete [] _this->m_FTClientItemInfo;
					_this->m_FTClientItemInfo = NULL;
				}
				if(_this->m_FTServerItemInfo != NULL) {
					delete [] _this->m_FTServerItemInfo;
					_this->m_FTServerItemInfo = NULL;
				}
				EndDialog(hwnd, TRUE);
				return TRUE;
			case IDC_CLIENTUP:
				if (strcmp(_this->m_ClientPathTmp, "") == 0) return TRUE;
				for(i=(strlen(_this->m_ClientPathTmp)-2); i>=0; i--) {
					if(_this->m_ClientPathTmp[i] == '\\') {
						_this->m_ClientPathTmp[i+1] = '\0';
						break;
					}
					if(i == 0) strcpy(_this->m_ClientPathTmp, "");
				}
				_this->ShowClientItems(_this->m_ClientPathTmp);
				return TRUE;
			case IDC_SERVERUP:
				if (strcmp(_this->m_ServerPathTmp, "") == 0) return TRUE;
				for(i=(strlen(_this->m_ServerPathTmp)-2); i>=0; i--) {
					if(_this->m_ServerPathTmp[i] == '\\') {
						_this->m_ServerPathTmp[i+1] = '\0';
						break;
					}
					if(i == 0) strcpy(_this->m_ServerPathTmp, "");
				}
				_this->SendFileListRequestMessage(_this->m_ServerPathTmp);
				return TRUE;
			case IDC_CLIENTRELOAD:
				_this->ShowClientItems(_this->m_ClientPath);
				return TRUE;
			case IDC_SERVERRELOAD:
				_this->SendFileListRequestMessage(_this->m_ServerPathTmp);
				return TRUE;
			case IDC_UPLOAD:
				_this->m_TransferEnable = TRUE;
				EnableWindow(GetDlgItem(hwnd, IDC_FTCANCEL), true);
				_this->FileTransferUpload();			
				return TRUE;
			case IDC_DOWNLOAD:
				char path[rfbMAX_PATH];
				char buffer[rfbMAX_PATH + rfbMAX_PATH + rfbMAX_PATH];
				_this->m_TransferEnable = TRUE;
				EnableWindow(GetDlgItem(hwnd, IDC_FTCANCEL), true);
				_this->BlockingFileTransferDialog(false);
				ListView_GetItemText(_this->m_hwndFTServerList, ListView_GetSelectionMark(_this->m_hwndFTServerList), 0, _this->m_ServerFilename, rfbMAX_PATH);
				strcpy(_this->m_ClientFilename, _this->m_ServerFilename);
				sprintf(buffer, "DOWNLOAD: %s%s to %s%s", _this->m_ServerPath, _this->m_ServerFilename, _this->m_ClientPath, _this->m_ClientFilename);
				SetWindowText(_this->m_hwndFTStatus, buffer);
				rfbFileDownloadRequestMsg fdr;
				fdr.type = rfbFileDownloadRequest;
				sprintf(path, "%s%s", _this->m_ServerPath, _this->m_ServerFilename);
				MessageBox(NULL, path, "filename for download from client", MB_OK);
				_this->ConvertPath(path);
				fdr.fnamesize = strlen(path);
				_this->m_clientconn->WriteExact((char *)&fdr, sz_rfbFileDownloadRequestMsg);
				_this->m_clientconn->WriteExact(path, fdr.fnamesize);
				return TRUE;
			case IDC_FTCANCEL:
				_this->m_TransferEnable = FALSE;
				EnableWindow(GetDlgItem(hwnd, IDC_FTCANCEL), false);
				return TRUE;
			case IDC_CLIENTBROWSE_BUT:
				_this->CreateFTBrowseDialog(FALSE);
				return TRUE;
			case IDC_SERVERBROWSE_BUT:
				_this->CreateFTBrowseDialog(TRUE);
				return TRUE;
			case IDC_FTABOUT:
				DialogBox(_this->m_pApp->m_instance, DIALOG_MAKEINTRESOURCE(IDD_FILETRANSFER_ABOUT), NULL, (DLGPROC) AboutFileTransferDlgProc);
				return TRUE;
		}
		}
	break;

	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_FTCLIENTLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case LVN_GETDISPINFO:
					_this->OnGetDispClientInfo((NMLVDISPINFO *) lParam); 
					return TRUE;
				case NM_DBLCLK:
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					_this->ProcessListViewDBLCLK(_this->m_hwndFTClientList, _this->m_ClientPath, _this->m_ClientPathTmp, lpnmia->iItem);
					return TRUE;
			}
		break;
		case IDC_FTSERVERLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case LVN_GETDISPINFO: 
					_this->OnGetDispServerInfo((NMLVDISPINFO *) lParam); 
					return TRUE;
				case NM_DBLCLK:
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					_this->ProcessListViewDBLCLK(_this->m_hwndFTServerList, _this->m_ServerPath, _this->m_ServerPathTmp, lpnmia->iItem);
					return TRUE;
			}
		break;
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		_this->m_clientconn->m_FileTransferEnable = false;
		if(_this->m_FTClientItemInfo != NULL) {
			delete [] _this->m_FTClientItemInfo;
			_this->m_FTClientItemInfo = NULL;
		}
		if(_this->m_FTServerItemInfo != NULL) {
			delete [] _this->m_FTServerItemInfo;
			_this->m_FTServerItemInfo = NULL;
		}
		EndDialog(hwnd, FALSE);
		return TRUE;
	}
	return 0;
}

void 
FileTransfer::OnGetDispClientInfo(NMLVDISPINFO *plvdi) 
{
  switch (plvdi->item.iSubItem)
    {
    case 0:
		plvdi->item.pszText = m_FTClientItemInfo[plvdi->item.iItem].Name;
      break;
    case 1:
		plvdi->item.pszText = m_FTClientItemInfo[plvdi->item.iItem].Size;
      break;
    default:
      break;
    }
 } 

void 
FileTransfer::OnGetDispServerInfo(NMLVDISPINFO *plvdi) 
{
  switch (plvdi->item.iSubItem)
  {
    case 0:
		plvdi->item.pszText = m_FTServerItemInfo[plvdi->item.iItem].Name;
      break;
    case 1:
		plvdi->item.pszText = m_FTServerItemInfo[plvdi->item.iItem].Size;
      break;
    default:
      break;
    }
 } 

void 
FileTransfer::FileTransferUpload()
{
	DWORD sz_rfbFileSize;
	DWORD sz_rfbBlockSize= 8192;
	DWORD dwNumberOfBytesRead = 0;
	DWORD dwNumberOfAllBytesRead = 0;
	char *pBuff = new char[sz_rfbBlockSize];
	char path[rfbMAX_PATH];
	BOOL bResult;
	HANDLE hFiletoRead;
	UINT nSelItem;
	nSelItem = ListView_GetSelectionMark(m_hwndFTClientList);
	if(nSelItem == -1) {
		SetWindowText(m_hwndFTStatus, "Select file for copy to server side");
		BlockingFileTransferDialog(true);
		return;
	}
	BlockingFileTransferDialog(false);
	ListView_GetItemText(m_hwndFTClientList, nSelItem, 0, m_ClientFilename, rfbMAX_PATH);
	sprintf(path, "%s%s", m_ClientPath, m_ClientFilename);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFile = FindFirstFile(path, &FindFileData);
	if( hFile != INVALID_HANDLE_VALUE) {
		sz_rfbFileSize = FindFileData.nFileSizeLow;
		strcpy(m_ServerFilename, FindFileData.cFileName);
	} else {
		SetWindowText(m_hwndFTStatus, "This is not file. Select a file");
		BlockingFileTransferDialog(true);
		return;
	}
	FindClose(hFile);
	hFiletoRead = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFiletoRead == INVALID_HANDLE_VALUE) {
		SetWindowText(m_hwndFTStatus, "This is not file. Select a file");
		BlockingFileTransferDialog(true);
		return;
	}
	char buffer[rfbMAX_PATH + rfbMAX_PATH + rfbMAX_PATH];
	sprintf(buffer, "UPLOAD: %s%s to %s%s", m_ClientPath, m_ClientFilename, m_ServerPath, m_ServerFilename);
	SetWindowText(m_hwndFTStatus, buffer);
	rfbFileUploadDataMsg fud;
	fud.type = rfbFileUploadData;
	int amount = (sz_rfbFileSize + sz_rfbBlockSize - 1) / sz_rfbBlockSize;
	fud.amount = Swap16IfLE(amount);
	SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
    SendMessage(m_hwndFTProgress, PBM_SETRANGE, 0, MAKELPARAM(0, amount)); 
	SendMessage(m_hwndFTProgress, PBM_SETSTEP, (WPARAM) 1, 0); 
	if (sz_rfbFileSize <= sz_rfbBlockSize) {
			sz_rfbBlockSize = sz_rfbFileSize;
			fud.size = Swap16IfLE(sz_rfbFileSize);
		}
	sprintf(path, "%s%s", m_ServerPath, m_ClientFilename);
	ConvertPath(path);
	rfbFileUploadRequestMsg fur;
	fur.type = rfbFileUploadRequest;
	fur.fnamesize = strlen(path);
	m_clientconn->WriteExact((char *)&fur, sz_rfbFileUploadRequestMsg);
	m_clientconn->WriteExact(path, strlen(path));
//	fud.fnamesize = strlen(path);
	for (int i=1; i<=amount; i++) {
		ProcessDlgMessage(m_hwndFileTransfer);
		if(m_TransferEnable == FALSE) {
			SetWindowText(m_hwndFTStatus, "File transfer canceled");
			EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), false);
			BlockingFileTransferDialog(true);
			rfbFileUploadFailedMsg fuf;
			fuf.type = rfbFileUploadFailed;
			m_clientconn->WriteExact((char *)&fuf, sz_rfbFileUploadFailedMsg);
			return;
		}
		fud.num = Swap16IfLE(i);
		bResult = ReadFile(hFiletoRead, pBuff, sz_rfbBlockSize, &dwNumberOfBytesRead, NULL);
		fud.size = Swap16IfLE(dwNumberOfBytesRead);
		m_clientconn->WriteExact((char *)&fud, sz_rfbFileUploadDataMsg);
		m_clientconn->WriteExact(pBuff, dwNumberOfBytesRead);
		SendMessage(m_hwndFTProgress, PBM_STEPIT, 0, 0); 
		dwNumberOfAllBytesRead += dwNumberOfBytesRead;
		if ((sz_rfbFileSize - dwNumberOfAllBytesRead) < sz_rfbBlockSize) {
			sz_rfbBlockSize = sz_rfbFileSize - dwNumberOfAllBytesRead;
			fud.size = Swap16IfLE(sz_rfbBlockSize);
		}
	}
	SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
	CloseHandle(hFiletoRead);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), false);
	delete [] pBuff;
	SendFileListRequestMessage(m_ServerPath);
}

void 
FileTransfer::FileTransferDownload()
{
	rfbFileDownloadDataMsg fdd;
	m_clientconn->ReadExact((char *)&fdd, sz_rfbFileDownloadDataMsg);
	fdd.size = Swap16IfLE(fdd.size);
	fdd.amount = Swap16IfLE(fdd.amount);
	fdd.num = Swap16IfLE(fdd.num);
	char * pBuff = new char [fdd.size + 1];
	char path[rfbMAX_PATH];
	DWORD dwNumberOfBytesWritten;
	m_clientconn->ReadExact(pBuff, fdd.size);
	ProcessDlgMessage(m_hwndFileTransfer);
	if(m_TransferEnable == FALSE) {
		SetWindowText(m_hwndFTStatus, "File transfer canceled");
		CloseHandle(m_hFiletoWrite);
		sprintf(path, "%s%s", m_ClientPath, m_ServerFilename);
		DeleteFile(path);
		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), false);
		BlockingFileTransferDialog(true);
		return;
	}
	if (fdd.num == 0) {
		sprintf(path, "%s%s", m_ClientPath, m_ServerFilename);
		m_hFiletoWrite = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
		SendMessage(m_hwndFTProgress, PBM_SETRANGE, 0, MAKELPARAM(0, fdd.amount)); 
		SendMessage(m_hwndFTProgress, PBM_SETSTEP, (WPARAM) 1, 0); 
	}
	WriteFile(m_hFiletoWrite, pBuff,	fdd.size, &dwNumberOfBytesWritten, NULL);
	SendMessage(m_hwndFTProgress, PBM_STEPIT, 0, 0); 
	if (fdd.num == fdd.amount - 1) {
		SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
		CloseHandle(m_hFiletoWrite);
		ShowClientItems(m_ClientPath);
		EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_FTCANCEL), false);
		BlockingFileTransferDialog(true);
	}
	delete [] pBuff;
}

void 
FileTransfer::ShowClientItems(char path[rfbMAX_PATH])
{
	char drive[] = "?:\\";
	char path_[rfbMAX_PATH];
	strcpy(path_, path);
	if(strcmp(path_, "") == 0) {
		//Show Disks
		ListView_DeleteAllItems(m_hwndFTClientList); 
		TCHAR szDrivesList[256];
		int szDrivesNum = 0;
		char fixeddrive[28] [10];
		GetLogicalDriveStrings(256, szDrivesList);
		for (int i=0, p=0; i<=255; i++, p++) {
			drive[p] = szDrivesList[i];
			if (szDrivesList[i] == '\0') {
				p = -1;
				if (GetDriveType((LPCTSTR) drive) == DRIVE_FIXED) {
					drive[strlen(drive) - 1] = '\0';
					strcpy(fixeddrive[szDrivesNum], drive);
					szDrivesNum += 1;
				}
				if (szDrivesList[i+1] == '\0') break;
			}
		}
		if(szDrivesNum == 0) {
			BlockingFileTransferDialog(true);
			strcpy(m_ClientPathTmp, m_ClientPath);
			return;
		} else {
			strcpy(m_ClientPath, m_ClientPathTmp);
			SetWindowText(m_hwndFTClientPath, m_ClientPath);
		}
		if(m_FTClientItemInfo != NULL) delete [] m_FTClientItemInfo;
		m_FTClientItemInfo = new FTITEMINFO [szDrivesNum];
		for(int ii=0; ii<szDrivesNum; ii++) {
			strcpy(m_FTClientItemInfo[ii].Name, fixeddrive[ii]);
			strcpy(m_FTClientItemInfo[ii].Size, "Folder");
		}
		ShowListViewItems(m_hwndFTClientList, m_FTClientItemInfo, szDrivesNum);
	} else {
		//Show Files
		HANDLE m_handle;
		int FilesNum = 0;
		WIN32_FIND_DATA m_FindFileData;
		strcat(path_, "*");
		m_handle = FindFirstFile(path_, &m_FindFileData);
		while(1) {
			if((m_handle != INVALID_HANDLE_VALUE) && 
			   (strcmp(m_FindFileData.cFileName, ".") != 0) &&
			   (strcmp(m_FindFileData.cFileName, "..") != 0)) FilesNum += 1;
			if (!FindNextFile(m_handle, &m_FindFileData)) break;
			}
		if(FilesNum == 0) {
			BlockingFileTransferDialog(true);
			strcpy(m_ClientPathTmp, m_ClientPath);
			return;
		} else {
			strcpy(m_ClientPath, m_ClientPathTmp);
			SetWindowText(m_hwndFTClientPath, m_ClientPath);
		}
		ListView_DeleteAllItems(m_hwndFTClientList); 
		m_handle = FindFirstFile(path_, &m_FindFileData);
		if(m_FTClientItemInfo !=NULL) delete [] m_FTClientItemInfo;
		m_FTClientItemInfo = new FTITEMINFO [FilesNum];
//		for(int i=0; i<FilesNum; i++) {
		int i=0;
		while(1) {
			if((strcmp(m_FindFileData.cFileName, ".") != 0) &&
			   (strcmp(m_FindFileData.cFileName, "..") != 0)) {
				if (!(m_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					char buffer[32];
					sprintf(buffer, "%d", m_FindFileData.nFileSizeLow);
					strcpy(m_FTClientItemInfo[i].Size, buffer);
				} else {
					strcpy(m_FTClientItemInfo[i].Size, "Folder");
				}
				strcpy(m_FTClientItemInfo[i].Name, m_FindFileData.cFileName);
				i++;
			}
			if (!FindNextFile(m_handle, &m_FindFileData)) break;
		}
		FindClose(m_handle);
		ShowListViewItems(m_hwndFTClientList, m_FTClientItemInfo, FilesNum);
	}
	BlockingFileTransferDialog(true);
	path_[strlen(path) - 1] = '\0';
}

BOOL CALLBACK 
FileTransfer::FTBrowseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransfer *_this = (FileTransfer *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (FileTransfer *) lParam;
			CentreWindow(hwnd);
			_this->m_hwndFTBrowse = hwnd;
			if (_this->m_bServerBrowseRequest) {
				_this->SendFileListRequestMessage("");
				return TRUE;
			} else {
				TVITEM TVItem;
				TVINSERTSTRUCT tvins; 
				TCHAR szDrivesList[256];
				char drive[] = "?:";
				TVItem.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_HANDLE;
				GetLogicalDriveStrings(256, szDrivesList);
				for (int i=0, p=0; i<=255; i++, p++) {
					drive[p] = szDrivesList[i];
					if (szDrivesList[i] == '\0') {
						p = -1;
						if (GetDriveType((LPCTSTR) drive) == DRIVE_FIXED) {
							drive[strlen(drive) - 1] = '\0';
							TVItem.pszText = drive;
							TVItem.cChildren = 1;
							tvins.item = TVItem;
							tvins.hParent = TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
							tvins.item = TVItem;
							TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
							tvins.hParent = NULL;
						}
						if (szDrivesList[i+1] == '\0') break;
					}
				}
			}
			return TRUE;
		}
	break;
	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
			case IDC_FTBROWSECANCEL:
				EndDialog(hwnd, TRUE);
				_this->m_bServerBrowseRequest = FALSE;
				return TRUE;
			case IDC_FTBROWSEOK:
				char path[rfbMAX_PATH];
				if(GetWindowText(GetDlgItem(hwnd, IDC_FTBROWSEEDIT), path, rfbMAX_PATH) == 0) {
					EndDialog(hwnd, TRUE);
					_this->m_bServerBrowseRequest = FALSE;
					return TRUE;
				}
				if(_this->m_bServerBrowseRequest) {
					strcpy(_this->m_ServerPathTmp, path);
					strcat(_this->m_ServerPathTmp, "\\");
					EndDialog(hwnd,TRUE);
					_this->m_bServerBrowseRequest = FALSE;
					_this->SendFileListRequestMessage(_this->m_ServerPathTmp);
					return TRUE;
				} else {
					strcpy(_this->m_ClientPathTmp, path);
					strcat(_this->m_ClientPathTmp, "\\");
					EndDialog(hwnd,TRUE);
					_this->ShowClientItems(_this->m_ClientPathTmp);
				}
				return TRUE;
		}
		}
	break;
	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_FTBROWSETREE:
			switch (((LPNMHDR) lParam)->code)
			{
			case TVN_SELCHANGED:
				{
					NMTREEVIEW *m_lParam = (NMTREEVIEW *) lParam;
					char path[rfbMAX_PATH];
					_this->GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, path);
					path[strlen(path) - 1] = '\0';
					SetWindowText(GetDlgItem(hwnd, IDC_FTBROWSEEDIT), path);
					return TRUE;
				}
				break;
			case TVN_ITEMEXPANDING:
				{
				NMTREEVIEW *m_lParam = (NMTREEVIEW *) lParam;
				char Path[255];
				if(m_lParam -> action == 2) {
					if(_this->m_bServerBrowseRequest) {
						_this->m_hTreeItem = m_lParam->itemNew.hItem;
						_this->GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, Path);
						_this->SendFileListRequestMessage(Path);
						return TRUE;
					} else {
						_this->ShowTreeViewItems(hwnd, m_lParam);
					}
				}
				return TRUE;
				}
			}
			break;
		}
	break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		_this->m_bServerBrowseRequest = FALSE;
		return TRUE;
	}
	return 0;
}

void 
FileTransfer::CreateFTBrowseDialog(BOOL status)
{
	m_bServerBrowseRequest = status;
	DialogBoxParam(m_pApp->m_instance, MAKEINTRESOURCE(IDD_FTBROWSE_DLG), m_hwndFileTransfer, (DLGPROC) FTBrowseDlgProc, (LONG) this);
}

char * 
FileTransfer::GetTVPath(HWND hwnd, HTREEITEM hTItem, char path[rfbMAX_PATH])
{
	char szText[rfbMAX_PATH];
	TVITEM _tvi;
	strcpy(path, "");
	do {
		_tvi.mask = TVIF_TEXT | TVIF_HANDLE;
		_tvi.hItem = hTItem;
		_tvi.pszText = szText;
		_tvi.cchTextMax = rfbMAX_PATH;
		TreeView_GetItem(hwnd, &_tvi);
		strcat(path, "\\");
		strcat(path, _tvi.pszText);
		hTItem = TreeView_GetParent(hwnd, hTItem);
	}
	while(hTItem != NULL);
	char path_tmp[rfbMAX_PATH] = "", path_out[rfbMAX_PATH] = "";
	int len = strlen(path);
	int ii = 0;
	for (int i = (len-1); i>=0; i--) {
		if (path[i] == '\\') {
			strinvert(path_tmp);
			strcat(path_out, path_tmp);
			strcat(path_out, "\\");
			strcpy(path_tmp, "");
			ii = 0;
		} else {
			path_tmp[ii] = path[i];
			path_tmp[ii+1] = '\0';
			ii++;
		}
	}
	strcpy(path, path_out);
	return path;
}

char * 
FileTransfer::strinvert(char str[rfbMAX_PATH])
{
	int len = strlen(str), i;
	char str_out[rfbMAX_PATH];
	strcpy(str_out, "");
	for (i = (len-1); i>=0; i--) str_out[len-i-1] = str[i];
	str_out[len] = '\0';
	strcpy(str, str_out);
	return str;
}

void 
FileTransfer::ShowTreeViewItems(HWND hwnd, LPNMTREEVIEW m_lParam)
{
	HANDLE m_handle;
	WIN32_FIND_DATA m_FindFileData;
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	char path[rfbMAX_PATH];
	GetTVPath(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem, path);
	strcat(path, "*");
	while (TreeView_GetChild(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem) != NULL) {
		TreeView_DeleteItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), TreeView_GetChild(GetDlgItem(hwnd, IDC_FTBROWSETREE), m_lParam->itemNew.hItem));
	}
	m_handle = FindFirstFile(path, &m_FindFileData);
	if(m_handle == INVALID_HANDLE_VALUE) return;
	while(1) {
		if ((strcmp(m_FindFileData.cFileName, ".") != 0) && 
			(strcmp(m_FindFileData.cFileName, "..") != 0)) {
			if (m_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {	
				tvi.mask = TVIF_TEXT;
				tvi.pszText = m_FindFileData.cFileName;
				tvins.hParent = m_lParam->itemNew.hItem;
				tvins.item = tvi;
				tvins.hParent = TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
				TreeView_InsertItem(GetDlgItem(hwnd, IDC_FTBROWSETREE), &tvins);
			}
		}
		if (!FindNextFile(m_handle, &m_FindFileData)) break;
	}
	FindClose(m_handle);
}

void 
FileTransfer::ProcessDlgMessage(HWND hwnd)
{
	MSG msg;
	while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void 
FileTransfer::BlockingFileTransferDialog(bool status)
{
	EnableWindow(m_hwndFTClientList, status);
	EnableWindow(m_hwndFTServerList, status);
	EnableWindow(m_hwndFTClientPath, status);
	EnableWindow(m_hwndFTServerPath, status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_UPLOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_DOWNLOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTUP), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERUP), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTRELOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERRELOAD), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTBROWSE_BUT), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_SERVERBROWSE_BUT), status);
	EnableWindow(GetDlgItem(m_hwndFileTransfer, IDC_EXIT), status);
}

void 
FileTransfer::ShowServerItems()
{
	char filename[rfbMAX_PATH + 1];
	rfbFileListDataMsg fld;
	m_clientconn->ReadExact((char *) &fld, sz_rfbFileListDataMsg);
	fld.amount = Swap16IfLE(fld.amount);
	fld.num = Swap16IfLE(fld.num);
	fld.attr = Swap16IfLE(fld.attr);
	fld.size = Swap32IfLE(fld.size);
	m_clientconn->ReadExact(filename, fld.fnamesize);
	filename[fld.fnamesize] = '\0';
	if(!m_bServerBrowseRequest) {
		if(fld.fnamesize == 0) {
			BlockingFileTransferDialog(true);
			strcpy(m_ServerPathTmp, m_ServerPath);
			return;
		} else {
			strcpy(m_ServerPath, m_ServerPathTmp);
			SetWindowText(m_hwndFTServerPath, m_ServerPath);
		}
		if(fld.num == 0) {
			if(m_FTServerItemInfo != NULL) delete [] m_FTServerItemInfo;
			m_FTServerItemInfo = new FTITEMINFO[fld.amount];
			ListView_DeleteAllItems(m_hwndFTServerList); 
		}
		strcpy(m_FTServerItemInfo[fld.num].Name, filename);
		switch (fld.attr)
			{
			case 0x0000:
				char buffer_[32];
				sprintf(buffer_, "%d", fld.size);
				strcpy(m_FTServerItemInfo[fld.num].Size, buffer_);
				break;
			case 0x0001:
				strcpy(m_FTServerItemInfo[fld.num].Size, "Folder");
				break;
			default:
				strcpy(m_FTServerItemInfo[fld.num].Size, "Unknown");
			}
		if(fld.num == fld.amount - 1) ShowListViewItems(m_hwndFTServerList, m_FTServerItemInfo, fld.amount);
	} else {
		if(fld.num == 0) {
			while (TreeView_GetChild(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), m_hTreeItem) != NULL) {
				TreeView_DeleteItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), TreeView_GetChild(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), m_hTreeItem));
			}
		}
//		if(fld.fnamesize == 0) return;
		TVITEM TVItem;
		TVINSERTSTRUCT tvins; 
		TVItem.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_HANDLE;
		TVItem.cChildren = 1;
		if(fld.fnamesize > rfbMAX_PATH) { 
			BlockingFileTransferDialog(true);
			return;
		}
		switch (fld.attr)
		{
			case 0x0001:
				if((strcmp(filename, ".") != 0) && (strcmp(filename, "..") != 0)) {
					TVItem.pszText = filename;
					TVItem.cChildren = 1;
					tvins.item = TVItem;
					tvins.hParent = m_hTreeItem;
					tvins.hParent = TreeView_InsertItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), &tvins);
					tvins.item = TVItem;
					TreeView_InsertItem(GetDlgItem(m_hwndFTBrowse, IDC_FTBROWSETREE), &tvins);
					tvins.hParent = m_hTreeItem;;
				}
			break;
		}
	}
	BlockingFileTransferDialog(true);
}
BOOL CALLBACK 
FileTransfer::AboutFileTransferDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (iMsg) {
	case WM_INITDIALOG:
		{
			CentreWindow(hwnd);
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hwnd, TRUE);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hwnd, TRUE);
		}
	}
	return FALSE;
}

void 
FileTransfer::SendFileListRequestMessage(char *filename)
{
	char _filename[rfbMAX_PATH];
	strcpy(_filename, filename);
	ConvertPath(_filename);
	int len = strlen(_filename);
	if((len > 2) && (_filename[len-1] == '/')) _filename[len-1] = '\0';
	rfbFileListRequestMsg flr;
	flr.type = rfbFileListRequest;
	flr.dnamesize = len;
	m_clientconn->WriteExact((char *)&flr, sz_rfbFileListRequestMsg);
	m_clientconn->WriteExact(_filename, len);
}

void 
FileTransfer::ProcessListViewDBLCLK(HWND hwnd, char *Path, char *PathTmp, int iItem)
{
	SendMessage(m_hwndFTProgress, PBM_SETPOS, 0, 0);
	SetWindowText(m_hwndFTStatus, "");
	strcpy(PathTmp, Path);
	char buffer[rfbMAX_PATH];
	char buffer_tmp[16];
	ListView_GetItemText(hwnd, iItem, 0, buffer, rfbMAX_PATH);
	ListView_GetItemText(hwnd, iItem, 1, buffer_tmp, 16);
	if(strcmp(buffer_tmp, "Folder") == 0) {
			BlockingFileTransferDialog(false);
			strcat(PathTmp, buffer);
			strcat(PathTmp,"\\");
			if(hwnd == m_hwndFTClientList) ShowClientItems(PathTmp);
			if(hwnd == m_hwndFTServerList) SendFileListRequestMessage(PathTmp);
	}
}

char *
FileTransfer::ConvertPath(char *path)
{
	int len = strlen(path);
	if(len >= rfbMAX_PATH) return path;
	if(path[0] == '/') {
		for(int i = 0; i < (len - 1); i++) {
			if(path[i+1] == '/') path[i+1] = '\\';
			path[i] = path[i+1];
		}
		path[len-1] = '\0';
	} else {
		for(int i = (len - 1); i >= 0; i--) {
			if(path[i] == '\\') path[i] = '/';
			path[i+1] = path[i];
		}
		path[len + 1] = '\0';
		path[0] = '/';
	}
	return path;
}

void 
FileTransfer::ShowListViewItems(HWND hwnd, FTITEMINFO *FTItemInfo, int NumItem)
{
	LVITEM LVItem;
	LVItem.mask = LVIF_TEXT | LVIF_STATE; 
	LVItem.state = 0; 
	LVItem.stateMask = 0; 
	qsort(FTItemInfo, NumItem, rfbMAX_PATH + 16, CompareFTItemInfo);
	for(int i=0; i<NumItem; i++) {
		LVItem.iItem = i;
		LVItem.iSubItem = 0;
		LVItem.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(hwnd, &LVItem);
	}
}
