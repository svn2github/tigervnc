
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id$

class vncServerSingleton ;

#ifndef __VNCSERVER_SINGLETON_H
#define __VNCSERVER_SINGLETON_H

class vncServer ;

class vncServerSingleton
{
public:
	static vncServer* GetInstance( void ) ;

private:
	vncServerSingleton() {} ;
	~vncServerSingleton() {} ;

	vncServerSingleton( const vncServerSingleton& rhs ) ;
	const vncServerSingleton& operator=( const vncServerSingleton& rhs ) ;

	static vncServer* m_server ;
} ;

#endif // __VNCSERVER_SINGLETON_H
