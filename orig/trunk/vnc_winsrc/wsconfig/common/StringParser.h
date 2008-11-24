#ifndef _STRING_PARSER_H_
#define _STRING_PARSER_H_

#include "tstring.h"

class StringParser
{
public:
  static bool tryParseInt(tstring str);
  static bool parseInt(tstring str, int *out);
};

#endif