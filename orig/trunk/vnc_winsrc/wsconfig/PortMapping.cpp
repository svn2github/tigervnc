#include "StdAfx.h"
#include "PortMapping.h"
#include "common/StringParser.h"

PortMapping::PortMapping()
: port(0)
{
}

PortMapping::~PortMapping()
{
}

tstring PortMapping::toString()
{
  TCHAR dest[10];
  tstring out = _T("");
  _ltot(port, &dest[0], 10);
  out += dest;
  out += _T(":");
  out += rect.toString();
  return out;
}

bool PortMapping::parse(tstring str, PortMapping *mapping)
{
  tstring strPort;
  tstring strRect;
  size_t delimPos = str.find(_T(":"));
  if (delimPos == tstring::npos)
    return false;
  strPort = str.substr(0, delimPos);
  strRect = str.substr(delimPos + 1, str.size() - delimPos - 1);
  if (!StringParser::tryParseInt(strPort))
    return false;
  if (!Rect::tryParse((TCHAR *)strRect.c_str()))
    return false;
  if (mapping != NULL) {
    StringParser::parseInt(strPort, &mapping->port);
    Rect::parse((TCHAR *)strRect.c_str(), &mapping->rect);
  }
  return true;
}