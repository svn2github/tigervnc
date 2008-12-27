//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

#ifndef _ADMINISTRATION_CONFIG_H_
#define _ADMINISTRATION_CONFIG_H_

class AdministrationConfig
{
public:

  enum ClientConnectionPriority {
    CCP_DISCONNECT_EXISTING_CONNECTIONS = 0,
    CCP_AUTOMATIC_SHARED_SESSIONS = 1,
    CCP_REFUSE_CONCURENT_CONNECTIONS = 2
  };

public:
  AdministrationConfig();
  ~AdministrationConfig();

  bool isOnlyLoopbackConnectionsAllowed() {
    return m_allowOnlyLoopbackConnections;
  }

  void enableOnlyLoopbackConnections(bool enabled) {
    m_allowOnlyLoopbackConnections = enabled;
  }

  bool isHttpServerEnabled() {
    return m_httpServerEnabled;
  }
  void enableHttpServer(bool enabled) {
    m_httpServerEnabled = enabled;
  }

  bool isAppletParamInUrlEnabled() {
    return m_appletParamInUrlEnabled;
  }
  void enableAppletParamInUrl(bool enabled) {
    m_appletParamInUrlEnabled = enabled;
  }

  ClientConnectionPriority getClientConnectionPriority() {
    return m_clientConnectionPriority;
  }
  void setClientConnectionPriority(ClientConnectionPriority priority) {
    m_clientConnectionPriority = priority;
  }

  int getLogLevel() { return m_logLevel; }
  void setLogLevel(int logLevel) { m_logLevel = logLevel; }
protected:
  bool m_disableEmptyPasswords;
  bool m_allowOnlyLoopbackConnections;
  bool m_httpServerEnabled;
  bool m_appletParamInUrlEnabled;
  ClientConnectionPriority m_clientConnectionPriority;
  int m_logLevel;
};

#endif
