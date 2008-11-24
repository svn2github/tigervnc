#include "StdAfx.h"
#include "PortMapping.h"

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