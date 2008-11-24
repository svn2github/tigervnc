#ifndef _PORT_MAPPING_H_
#define _PORT_MAPPING_H_

#include "common/tstring.h"
#include "Rect.h"

class PortMapping
{
public:
  PortMapping();
  ~PortMapping();
public:
  tstring toString();
  static bool parse(tstring str, PortMapping *mapping);
public:
  int port;
  Rect rect;
};

#endif