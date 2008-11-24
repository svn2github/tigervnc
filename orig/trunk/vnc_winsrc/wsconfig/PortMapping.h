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
public:
  int port;
  Rect rect;
};

#endif