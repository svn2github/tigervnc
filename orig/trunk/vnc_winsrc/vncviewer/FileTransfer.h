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
	void ShowListViewItems(HWND hwnd, FTITEMINFO *FTItemInfo, int NumItem);
	void ConvertPath(char *path);
	void ProcessListViewDBLCLK(HWND hwnd, char *Path, char *PathTmp, int iItem);
	void SendFileListRequestMessage(char *filename);
	HANDLE m_hFiletoWrite;
	char m_ClientFilename[rfbMAX_PATH];
	char m_ServerFilename[rfbMAX_PATH];
	HTREEITEM m_hTreeItem;
	BOOL m_bServerBrowseRequest;
	HINSTANCE m_FTInstance;
	void ShowServerItems();
	void BlockingFileTransferDialog(bool status);
	void ProcessDlgMessage(HWND hwnd);
	HWND m_hwndFTStatus;
	void ShowTreeViewItems(HWND hwnd, LPNMTREEVIEW m_lParam);
	char* strinvert(char str[rfbMAX_PATH]);
	char* GetTVPath(HWND hwnd, HTREEITEM hTItem, char path[rfbMAX_PATH]);
	HWND m_hwndFTBrowse;
	void CreateFTBrowseDialog(BOOL status);
	char m_ClientPath[rfbMAX_PATH];
	char m_ServerPath[rfbMAX_PATH];
	char m_ServerPathTmp[rfbMAX_PATH];
	char m_ClientPathTmp[rfbMAX_PATH];
	FTITEMINFO * m_FTClientItemInfo;
	FTITEMINFO * m_FTServerItemInfo;
	void OnGetDispClientInfo(NMLVDISPINFO *plvdi); 
	void OnGetDispServerInfo(NMLVDISPINFO *plvdi); 
	void ShowClientItems(char path[rfbMAX_PATH]);
	void CreateFileTransferDialog();
	FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp);
	static LRESULT CALLBACK FileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK AboutFileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
	BOOL m_TransferEnable;
	virtual ~FileTransfer();

};

#endif // !defined(FILETRANSFER)
