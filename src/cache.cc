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

#include "cache.h"
#include "util.h"
#include "platform.h"

namespace Teng {

std::string
createCacheKeyForFilename(const std::string &root, std::string filename) {
    // if filename is relative prepend root
    if (!filename.empty() && !ISROOT(filename))
        filename = root + '/' + filename;

    // normalize filename and return
    normalizeFilename(filename);
    return filename;
}

std::string
createCacheKeyForString(const std::string &data) {
    return MD5Hexdigest(data);
}

} // namespace Teng
