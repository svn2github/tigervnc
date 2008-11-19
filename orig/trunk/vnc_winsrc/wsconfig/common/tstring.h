#ifndef _TSTRING_H_
#define _TSTRING_H_

#include <string.h>
#include <string>
#include <tchar.h>

using namespace std;

typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;

#endif