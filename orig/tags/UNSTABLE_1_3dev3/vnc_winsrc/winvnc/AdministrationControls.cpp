// AdministrationControls.cpp: implementation of the AdministrationControls class.
//
//////////////////////////////////////////////////////////////////////

#include "AdministrationControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AdministrationControls::AdministrationControls(HWND hwnd, vncServer * server)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}
void AdministrationControls::Validate()
{
	Enable(IDONLYLOOPBACK, IsChecked(IDALLOWLOOPBACK));
	Enable(IDLOGLOTS, IsChecked(IDLOG));
}
void AdministrationControls::Apply()
{
	if (IsChecked(IDPRIORITY1))
		m_server->SetConnectPriority(1);
	if (IsChecked(IDPRIORITY2))
		m_server->SetConnectPriority(2);
	if (IsChecked(IDPRIORITY0))
		m_server->SetConnectPriority(0);

	m_server->SetAuthRequired(IsChecked(IDREQUIREAUTH));
	m_server->SetHttpdEnabled(IsChecked(IDENABLEHTTPD),
						 m_server->HttpdParamsEnabled());
	m_server->SetLoopbackOk(IsChecked(IDALLOWLOOPBACK));
	m_server->SetLoopbackOnly(IsChecked(IDONLYLOOPBACK));

	if (IsChecked(IDLOG))
		vnclog.SetMode(2);
	else
		vnclog.SetMode(0);

	if (IsChecked(IDLOGLOTS))
		vnclog.SetLevel(10);
	else
		vnclog.SetLevel(2);
}
void AdministrationControls::Init()
{
	SetChecked(IDENABLEHTTPD, m_server->HttpdEnabled());
	SetChecked(IDALLOWLOOPBACK, m_server->LoopbackOk());
	SetChecked(IDONLYLOOPBACK, m_server->LoopbackOnly());
	SetChecked(IDREQUIREAUTH, m_server->AuthRequired());

	int priority = m_server->ConnectPriority();
	
	SetChecked(IDPRIORITY1, (priority == 1));	
	SetChecked(IDPRIORITY2, (priority == 2));	
	SetChecked(IDPRIORITY0, (priority == 0));
	
	SetChecked(IDLOG, (vnclog.GetMode() >= 2));	
	SetChecked(IDLOGLOTS, (vnclog.GetLevel() > 5));
	
	Validate();
}
AdministrationControls::~AdministrationControls()
{

}
