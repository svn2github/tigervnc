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

#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include "common/tstring.h"
#include "Rect.h"

//
// FIXME: truncate members and methods names
//

class ServerConfig
{
public:

  //
  // Enum defines server action when last client disconnects
  // from VNC server.
  //

  enum DisconnectAction {
    CDAT_DO_NOTHING = 0,
    CDAT_LOCK_WORKSTATION = 1,
    CDAT_LOGOUT_WORKSTATION = 2
  };

public:
  ServerConfig(void);
  ~ServerConfig(void);

  //
  // Display (or port numbers) group
  //

  void setDisplayNumber(int displayNum) {
    m_vncPort = 5900 + displayNum;
    m_httpPort = 5800 + displayNum;
  }

  void setVncPort(int port) { m_vncPort = port; }
  int getVncPort() { return m_vncPort; }

  void setHttpPort(int port) { m_httpPort = port; }
  int getHttpPort() { return m_httpPort; }

  bool isBlackScreenOnClientConnectionEnabled() {
    return m_blankScreenOnClientConnections;
  }

  //
  // Other server options access methods
  //

  void enableFileTransfers(bool enabled) { m_fileTransfersEnabled = enabled; }
  bool isFileTransfersEnabled() { return m_fileTransfersEnabled; }

  void enableRemovingDesktopWallpaper(bool enabled) { m_allowRemovingDesktopWallpaper = enabled; }
  bool isRemovingDesktopWallpaperEnabled() { return m_allowRemovingDesktopWallpaper; }

  //
  // Last client disconnect action members access group
  //

  void setServerActionOnLastClientDisconnect(DisconnectAction action) {
    m_disconnectAction = action;
  }

  DisconnectAction getServerActionOnLastClientDisconnect() {
    return m_disconnectAction;
  }

  //
  // Incoming connections options group
  //

  bool allowSocketConnections() { return m_allowSocketConnections; }
  void enableSocketConnections(bool enabled) { m_allowSocketConnections = enabled; }

  //
  // FIXME: Keep it just as string in memory without encription?
  //

  tstring getPassword() { return m_password; }
  void setPassord(tstring value) { m_password = value; }

protected:

  //
  // Server port numbers
  //

  int m_vncPort;
  int m_httpPort;

  bool m_blankScreenOnClientConnections;

  //
  // Other server options members group
  //

  bool m_fileTransfersEnabled;
  bool m_allowRemovingDesktopWallpaper;

  //
  // Server action when last client disconnects from server
  //

  DisconnectAction m_disconnectAction;

  //
  // Incoming connections options group
  //

  bool m_allowSocketConnections;
  tstring m_password;
};

#endif
