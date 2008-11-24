#include "StdAfx.h"
#include "StringParser.h"
#include <tchar.h>

bool StringParser::tryParseInt(tstring str)
{
  TCHAR* c_str = (TCHAR *)str.c_str();
  int value = 0;
  if(_stscanf(c_str, _T("%d"), &value)  == EOF) {
    /* error */
    return false;
  }
  return true;
}