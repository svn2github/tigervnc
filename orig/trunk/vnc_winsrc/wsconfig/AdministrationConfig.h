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

  enum ConnectionPriority {
    ACCP_DISCONNECT_EXISTING_CONNECTIONS = 0,
    ACCP_AUTOMATIC_SHARED_SESSIONS = 1,
    ACCP_REFUSE_CONCURENT_CONNECTIONS = 2
  };

public:
  AdministrationConfig();
  ~AdministrationConfig();

  bool allowOnlyLoopbackConnections() {
    return m_allowOnlyLoopbackConnections;
  }

  void enableOnlyLoopbackConnections(bool enabled) {
    m_allowOnlyLoopbackConnections = enabled;
  }

  bool isBuiltInHttpServerEnabled() {
    return m_builtinHttpServerEnabled;
  }
  void enableBuiltinHttpServer(bool enabled) {
    m_builtinHttpServerEnabled = enabled;
  }

  bool isAppletParamInUrlEnabled() {
    return m_appletParamInUrlEnabled;
  }
  void enableAppletParamInUrl(bool enabled) {
    m_appletParamInUrlEnabled = enabled;
  }

  ConnectionPriority getConnectionPriority() {
    return m_connectionPriority;
  }
  void setConnectionPriority(ConnectionPriority priority) {
    m_connectionPriority = priority;
  }

  int getLogLevel() { return m_logLevel; }
  void setLogLevel(int logLevel) { m_logLevel = logLevel; }
protected:
  bool m_disableEmptyPasswords;
  bool m_allowOnlyLoopbackConnections;
  bool m_builtinHttpServerEnabled;
  bool m_appletParamInUrlEnabled;
  ConnectionPriority m_connectionPriority;
  int m_logLevel;
};

#endif
