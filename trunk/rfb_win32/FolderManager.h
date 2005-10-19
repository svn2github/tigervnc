/* Copyright (C) 2005 TightVNC Team.  All Rights Reserved.
 *    
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 * TightVNC distribution homepage on the Web: http://www.tightvnc.com/
 *
 */

// -=- FolderManager.h

#ifndef __RFB_WIN32_FOLDERMANAGER_H__
#define __RFB_WIN32_FOLDERMANAGER_H__

#include <windows.h>

#include <rfb/FileInfo.h>
#include <rfb/DirManager.h>

namespace rfb {
  namespace win32{
    class FolderManager : public DirManager {
    public:
      FolderManager();
      ~FolderManager();
      
      bool createDir(char *pFullPath);
      bool renameDir(char *pOldName, char *pNewName);
      bool deleteDir(char *pFullPath);
      
      bool getFolderInfo(char *pPath, FileInfo *pFileInfo, unsigned int dirOnly);
      bool getDrivesInfo(FileInfo *pFI);
    };
  }
}

#endif // __RFB_WIN32_FOLDERMANAGER_H__