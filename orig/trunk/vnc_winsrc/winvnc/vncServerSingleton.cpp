
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id$

#include "vncServerSingleton.h"
#include "vncServer.h"

vncServer* vncServerSingleton::m_server = 0 ;

vncServer*
vncServerSingleton::GetInstance( void ) 
{ 
	if ( m_server == NULL )
		m_server = new vncServer() ;
	return m_server ;
}
