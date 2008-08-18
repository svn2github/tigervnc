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

#ifndef _EXSTRING_H_
#define _EXSTRING_H_

#include <string.h>
#include <tchar.h>

extern inline void _tcsaddquotes(TCHAR *str);
extern inline void _tcsremquotes(TCHAR *str);
extern inline int _tcsextrword(TCHAR *str, TCHAR *extrWord);
extern inline TCHAR *_tcsdelword(TCHAR *str, TCHAR *delsubstrword);

extern inline void _straddquotes(char *str);
extern inline void _strremquotes(char *str);
extern inline int _strextrword(char *str, char *extrWord);
extern inline char *_strdelword(char *str, char *delsub_strword);
extern char *_strCharToHexStr(const char *mem, const int len);
extern char *_strHexStrToChar(const char *hexStr, int *len);
extern void _strXORString(char *str, const int lenStr, const char *key, const int lenKey);

#endif // _EXSTRING_H_