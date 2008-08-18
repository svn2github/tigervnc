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

#include "exstring.h"

const char hexTable[] = "0123456789abcdef";

inline void _tcsaddquotes(TCHAR *str)
{
  _tcsremquotes(str);
  // Shift on one symbol
  TCHAR *pos;
  for (pos = str + _tcslen(str) + 1; pos > str; pos--) {
    *pos= *(pos - 1);
  }
  *str = '\"';
  _tcscat(str, _T("\""));
}

inline void _straddquotes(char *str)
{
  _strremquotes(str);
  // Shift on one symbol
  char *pos;
  for (pos = str + strlen(str) + 1; pos > str; pos--) {
    *pos= *(pos - 1);
  }
  *str = '\"';
  strcat(str, "\"");
}

inline void _tcsremquotes(TCHAR *str)
{
  // Find begin of word
  TCHAR *posStr, *posWord;
  for (posStr = str; *posStr == ' '; posStr++) {}

  // Remove quotes
  for (posWord = str; *posStr != '\0'; posStr++) {
    if (*posStr != '\"') {
      *posWord = *posStr;
      posWord++;
    }
  }
  *posWord = '\0';
}

inline void _strremquotes(char *str)
{
  // Find begin of word
  char *posStr, *posWord;
  for (posStr = str; *posStr == ' '; posStr++) {}

  // Remove quotes
  for (posWord = str; *posStr != '\0'; posStr++) {
    if (*posStr != '\"') {
      *posWord = *posStr;
      posWord++;
    }
  }
  *posWord = '\0';
}

inline int _tcsextrword(TCHAR *str, TCHAR *extrWord)
{
  int i = 0;
  *extrWord = '\0';
  // Find begin of word
  TCHAR *posStr, *posWord;
  for (posStr = str; *posStr == ' '; posStr++, i++) {}

  // Copy word with quotes
  bool inquote = false;
  for (posWord = extrWord; (*posStr != ' ' || inquote) && *posStr != '\0'; posStr++, posWord++, i++) {
    if (*posStr == '\"') {
      inquote = !inquote;
    }
    *posWord = *posStr;
  }
  *posWord = '\0';
  return i;
}

inline int _strextrword(char *str, char *extrWord)
{
  int i = 0;
  *extrWord = '\0';
  // Find begin of word
  char *posStr, *posWord;
  for (posStr = str; *posStr == ' '; posStr++) {}

  // Copy word with quotes
  bool inquote = false;
  for (posWord = extrWord; (*posStr != ' ' || inquote) && *posStr != '\0'; posStr++, posWord++) {
    if (*posStr == '\"') {
      inquote = !inquote;
    }
    *posWord = *posStr;
  }
  *posWord = '\0';
  return i;
}

inline TCHAR *_tcsdelword(TCHAR *str, TCHAR *delsubstrword)
{
  TCHAR *posBegin, *posEnd;

  // Extract one word or words in quotes from delsubstr
  TCHAR *findWord, *delWord;
  delWord = new TCHAR[_tcslen(delsubstrword) + 1];
  _tcsextrword(delsubstrword, delWord);

  // Find begin of word
  if ((findWord = posBegin = _tcsstr(str, delWord)) == NULL) {
    delete[] delWord;
    return NULL;
  }
  delete[] delWord;

  // Find end of word
  bool inquote = false;
  for (posEnd = posBegin; (*posEnd != ' ' || inquote) && *posEnd != '\0'; posEnd++) {
    if (*posEnd == '\"') inquote = !inquote;
  }

  // Find next word 
  for (; *posEnd == ' '; posEnd++) {}

  // Move characters
  for (; *posEnd != '\0'; posBegin++, posEnd++){
    *posBegin = *posEnd;
  }
  // Terminate string
  *posBegin = '\0';

  return findWord;
}

inline char *_strdelword(char *str, char *delsubstrword)
{
  char *posBegin, *posEnd;

  // Extract one word or words in quotes from delsubstr
  char *findWord, *delWord;
  delWord = new char[strlen(delsubstrword) + 1];
  _strextrword(delsubstrword, delWord);

  // Find begin of word
  if ((findWord = posBegin = strstr(str, delWord)) == NULL) {
    delete[] delWord;
    return NULL;
  }
  delete[] delWord;

  // Find end of word
  bool inquote = false;
  for (posEnd = posBegin; (*posEnd != ' ' || inquote) && *posEnd != '\0'; posEnd++) {
    if (*posEnd == '\"') inquote = !inquote;
  }

  // Find next word 
  for (; *posEnd == ' '; posEnd++) {}

  // Move characters
  for (; *posEnd != '\0'; posBegin++, posEnd++){
    *posBegin = *posEnd;
  }
  // Terminate string
  *posBegin = '\0';

  return findWord;
}

char *_strCharToHexStr(const char *mem, const int len)
{
  char *resultStr = new char[len*2 + 1];
  for (int i = 0; i < len; i++) {
    resultStr[i * 2] = hexTable[(mem[i] >> 4) & 0xf];
    resultStr[i * 2 + 1] = hexTable[mem[i] & 0xf];
  }
  resultStr[len * 2] = '\0';
  return resultStr;
}

char *_strHexStrToChar(const char *hexStr, int *len)
{
  *len = strlen(hexStr)/2;
  char *resultMem = new char[*len + 1];
  for (int i = 0; i < *len * 2; i++) {
    char hexValue = 0;
    if (hexStr[i] >= '0' && hexStr[i] <= '9') {
      hexValue = hexStr[i] - '0';
    } else if (hexStr[i] >= 'a' && hexStr[i] <= 'f') {
      hexValue = hexStr[i] - 'a' + 10;
    } else {
      return NULL;
    }
    resultMem[i/2] = resultMem[i/2] << 4;
    resultMem[i/2] = resultMem[i/2] | hexValue;
  }

  resultMem[*len] ='\0'; 
  return resultMem;
}

void _strXORString(char *str, const int lenStr, const char *key, const int lenKey)
{
  if (key == NULL) return;

  for (int iStr = 0, iKey = 0; iStr < lenStr; iStr++, iKey++) {
    if (iKey >= lenKey) 
      iKey = 0;
    str[iStr] = str[iStr] ^ key[iKey];
  }
}