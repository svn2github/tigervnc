#ifndef INTERFACE_DLL_PROXY_INFO_H
#define INTERFACE_DLL_PROXY_INFO_H

//enum
//{
//	STATUS_MSG_BASE = WM_USER + 3000,

#define STATUS_CONNECTED					0x00000002
#define STATUS_AUTHENTICATING				0x00000004
#define STATUS_AUTHENTICATION_FAILED		0x00000008
#define STATUS_ESTABLISHING_DATA_CHANNEL	0x00000010
#define STATUS_SEARCHING_FOR_PARTNER		0x00000020
#define STATUS_DISCONNECTED_FROM_PROXY		0x00000040

//	STATUS_MSG_LAST	
//};

enum
{
	ERROR_CONNECTING_TO_PROXY = -1,
	CONNECTION_SUCCESSFUL,
	NO_PROXY_SERVER_FOUND_TO_CONNECT,
	AUTHENTICATION_FAILED,
	PROXY_ALREADY_CONNECTED,
	CONNECTION_TIMED_OUT,
	ID_FOUND_EMPTY
};

class IDllProxyInfo
{
public:
	//Set function for proxy details
	virtual void SetName(char* name) = 0;
	virtual void SetIpPort(char*ipport) = 0;
	virtual void SetIP(char*ipport) = 0;
	virtual void SetPort(char*port) = 0;
	virtual void SetPassword(char*pass) = 0;
	virtual void SetStatus(int Status, BOOL IsStoring) = 0;
	virtual BOOL SetMyID(char* MyID) = 0;
	virtual BOOL SetSocketTimeout(int connectTimeout, 
									int ReceiveTimeout, 
										int SendTimeout) =0;
	virtual void SetReconnectProxy(BOOL bReconnectProxy) = 0;

	//Get function for proxy details
	virtual char* GetName() = 0;
	virtual char* GetIpPort() = 0;
	virtual char* GetIP() = 0;
	virtual char* GetPort() = 0;
	virtual char* GetPassword() = 0;
	virtual int	  GetStatus() = 0;
	virtual	char* GetMyID() = 0;
	virtual BOOL GetReconnectProxy() = 0;
};

#endif INTERFACE_DLL_PROXY_INFO_H