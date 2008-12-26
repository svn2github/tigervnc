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
  ServerConfig(void);
  ~ServerConfig(void);

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
    m_blockRemoteInputOnLocalActivity = blockEnabled;
  }

  bool isBlockingRemoteInputOnLocalActivity() {
    return m_blockRemoteInputOnLocalActivity;
  }

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
protected:

  //
  // Input handling options members group
  //

  bool m_blockRemoteInputEvents;
  bool m_blockRemoteInputOnLocalActivity;
  bool m_noLocalInputDuringClientSessions;
  bool m_blankScreenOnClientConnections;

  //
  // Other server options members group
  //

  bool m_fileTransfersEnabled;
  bool m_allowRemovingDesktopWallpaper;
};

#endif
