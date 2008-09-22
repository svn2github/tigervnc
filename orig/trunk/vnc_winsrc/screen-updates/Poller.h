#pragma once
#include "updatedetector.h"

class Poller :
  public UpdateDetector
{
public:
  Poller(void);
  virtual ~Poller(void);
};
