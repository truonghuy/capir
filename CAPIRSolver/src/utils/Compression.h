/*
 * Copyright (c) 2012 Truong-Huy D. Nguyen.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v3.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/gpl.html
 * 
 * Contributors:
 *     Truong-Huy D. Nguyen - initial API and implementation
 */



#ifndef __COMPRESSION_H
#define __COMPRESSION_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cstring>
#include "zlib.h"

/**
   @class Compression
   @brief Class dealing with compressing text files. 
   @author Timo Bingmann at http://idlebox.net/about/timo.htt
   @date 05 Sept 2010
*/


class Compression
{
  public:
    static std::string compress_string(const std::string& str, int compressionlevel);
    static std::string decompress_string(const std::string& str);

};
#endif // __COMPRESSION_H
