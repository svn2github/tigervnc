/* Copyright (C) 2002-2003 RealVNC Ltd.  All Rights Reserved.
 *    
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __RDR_RANDOMSTREAM_H__
#define __RDR_RANDOMSTREAM_H__

#include <stdio.h>
#include <rdr/InStream.h>

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace rdr {

  class RandomStream : public InStream {

  public:

    RandomStream();
    virtual ~RandomStream();

    int pos();

  protected:
    int overrun(int itemSize, int nItems, bool wait);

  private:
    U8* start;
    int offset;

    static unsigned int seed;
#ifdef WIN32
    HCRYPTPROV provider;
#else
    FILE* fp;
#endif

  };

} // end of namespace rdr

#endif
