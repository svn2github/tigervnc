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

class ServerConfig
{
public:

  //
  // Enum defines server action when last client disconnects
  // from VNC server.
  //

  enum ClientDisconnectActionType {
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

  //
  // Input handling options access methods
  //

  void blockRemoteInputEvents(bool blockEnabled) {
    m_blockRemoteInputEvents = blockEnabled;
    if (m_blockRemoteInputEvents) {
      blockRemoteInputOnLocalActivity(false);
    }
  }

  bool isBlockingRemoteInputEvents() { return m_blockRemoteInputEvents; }

  void blockRemoteInputOnLocalActivity(bool blockEnabled) {
    if ((m_blockRemoteInputEvents) || (m_noLocalInputDuringClientSessions)) {
      m_blockRemoteInputOnLocalActivity = false;
    } else {
      m_blockRemoteInputOnLocalActivity = blockEnabled;
    }
  }

  bool isBlockingRemoteInputOnLocalActivity() {
    return m_blockRemoteInputOnLocalActivity;
  }

  unsigned int getInactivityTimeout() { return m_inactivityTimeout; }
  void setInactivityTimeout(unsigned int value) { m_inactivityTimeout = value; }

  void enableLocalInputDuringClientSession(bool enabled) {
    m_noLocalInputDuringClientSessions = !enabled;
    if (m_noLocalInputDuringClientSessions) {
      blockRemoteInputOnLocalActivity(false);
    }
  }

  bool isLocalInputDuringClientSessionEnabled() {
    return !m_noLocalInputDuringClientSessions;
  }

  void enableBlankScreenOnClientConnection(bool enabled) {
    m_blankScreenOnClientConnections = enabled;
  }

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

  void setServerActionOnLastClientDisconnect(ClientDisconnectActionType action) {
    m_lastClientDisconnectAction = action;
  }

  ClientDisconnectActionType getServerActionOnLastClientDisconnect() {
    return m_lastClientDisconnectAction;
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

  //
  // Input handling options members group
  //

  bool m_blockRemoteInputEvents;
  bool m_blockRemoteInputOnLocalActivity;
  bool m_noLocalInputDuringClientSessions;
  bool m_blankScreenOnClientConnections;

  unsigned int m_inactivityTimeout;

  //
  // Other server options members group
  //

  bool m_fileTransfersEnabled;
  bool m_allowRemovingDesktopWallpaper;

  //
  // Last client disconnect action members group
  //

  ClientDisconnectActionType m_lastClientDisconnectAction;

  //
  // Incoming connections options group
  //

  bool m_allowSocketConnections;
  tstring m_password;
};

#endif
