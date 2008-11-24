#include "StdAfx.h"
#include "StringParser.h"
#include <tchar.h>

bool StringParser::parseInt(tstring str, int *out)
{
  TCHAR* c_str = (TCHAR *)str.c_str();
  int value = 0;
  if(_stscanf(c_str, _T("%d"), &value)  == EOF) {
    /* error */
    return false;
  }
  if (out != NULL) {
    *out = value;
  }
  return true;
}

bool StringParser::tryParseInt(tstring str)
{
  return parseInt(str, NULL);
}