/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: tengcache.cc,v 1.2 2005-06-22 07:16:07 romanmarek Exp $
 *
 * DESCRIPTION
 * Teng cache of files -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-23  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 */


#include <sys/stat.h>
#include <unistd.h>

#include "tengcache.h"
#include "tengutil.h"
#include "tengplatform.h"

namespace Teng {

int tengCreateKey(const std::string &filename,
                  std::vector<std::string> &key)
{
    // add it to the key
    key.push_back(filename);
    return 0;
}

int tengCreateStringKey(const std::string &data,
                        std::vector<std::string> &key)
{
    // compute md5 hexdigest from data
    std::string hexdigest;
    tengMD5Hexdigest(data, hexdigest);
    // add it to the key
    key.push_back(hexdigest);
    return 0;
}

} // namespace Teng
