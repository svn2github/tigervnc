// FileTransfer.h: interface for the FileTransfer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FILETRANSFER)
#define FILETRANSFER

#define rfbMAX_PATH 255

#include "windows.h"
#include "commctrl.h"
#include "ClientConnection.h"

class ClientConnection;

typedef struct tagFTITEMINFO
{
    char Name[rfbMAX_PATH];
    char Size[16];
} FTITEMINFO;

class FileTransfer  
{
public:
	FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp);
	void FTInsertColumn(HWND hwnd, char *iText, int iOrder, int xWidth);
	void ClearFTItemInfo(FTITEMINFO *ftiteminfo);
	void CreateFileTransferDialog();
	void ShowListViewItems(HWND hwnd, FTITEMINFO *FTItemInfo, int NumItem);
	void ConvertPath(char *path);
	void ProcessListViewDBLCLK(HWND hwnd, char *Path, char *PathTmp, int iItem);
	void SendFileListRequestMessage(char *filename);
	void ShowServerItems();
	void ShowClientItems(char path[rfbMAX_PATH]);
	void BlockingFileTransferDialog(BOOL status);
	void ProcessDlgMessage(HWND hwnd);
	void ShowTreeViewItems(HWND hwnd, LPNMTREEVIEW m_lParam);
	void CreateFTBrowseDialog(BOOL status);
	char* strinvert(char str[rfbMAX_PATH]);
	char* GetTVPath(HWND hwnd, HTREEITEM hTItem, char path[rfbMAX_PATH]);
	char m_ClientPath[rfbMAX_PATH];
	char m_ServerPath[rfbMAX_PATH];
	char m_ServerPathTmp[rfbMAX_PATH];
	char m_ClientPathTmp[rfbMAX_PATH];
	char m_ClientFilename[rfbMAX_PATH];
	char m_ServerFilename[rfbMAX_PATH];
	FTITEMINFO * m_FTClientItemInfo;
	FTITEMINFO * m_FTServerItemInfo;
	void OnGetDispClientInfo(NMLVDISPINFO *plvdi); 
	void OnGetDispServerInfo(NMLVDISPINFO *plvdi); 
	static LRESULT CALLBACK FileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK FTBrowseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void FileTransferDownload();
	void FileTransferUpload();

	ClientConnection * m_clientconn;
	VNCviewerApp * m_pApp; 
	
	HWND m_hwndFileTransfer;
	HWND m_hwndFTClientList;
	HWND m_hwndFTServerList;
	HWND m_hwndFTClientPath;
	HWND m_hwndFTServerPath;
	HWND m_hwndFTProgress;
	HWND m_hwndFTStatus;
	HWND m_hwndFTBrowse;
	
	BOOL m_TransferEnable;
	BOOL m_bServerBrowseRequest;
	BOOL m_bFTCOPY;

	HANDLE m_hFiletoWrite;
	HTREEITEM m_hTreeItem;
	HINSTANCE m_FTInstance;

	virtual ~FileTransfer();
};

#endif // !defined(FILETRANSFER)
