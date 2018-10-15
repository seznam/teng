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
 * $Id: tengsourcelist.cc,v 1.3 2005-06-22 07:16:12 romanmarek Exp $
 *
 * DESCRIPTION
 * Teng list of sources -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-23  (vasek)
 *             Created.
 */


#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>

#include "tengsourcelist.h"
#include "tengutil.h"

namespace Teng {

int FileStat_t::stat(const FilesystemInterface_t* filesystem,
                     const Error_t::Position_t &pos,
                     Error_t &err)
{
    // invalidate data;
    valid = false;
    
    try {
        hash = filesystem->hash(filename);
    }
    catch(std::exception& ex) {
        err.logSyscallError(Error_t::LL_ERROR, pos, ex.what());
        return -1;
    }
   
    // validate data
    valid = true;
    // OK
    return 0;
}

unsigned int SourceList_t::addSource(const FilesystemInterface_t* filesystem,
                                     const std::string &source,
                                     const Error_t::Position_t &pos,
                                     Error_t &err)
{
    // create source info
    FileStat_t fs(source);

    // try to find existing entry
    std::vector<FileStat_t>::const_iterator fsources =
        std::find(sources.begin(), sources.end(), fs);
    if (fsources != sources.end()) {
        // entry already present => just return its position
        return static_cast<unsigned int>(fsources - sources.begin());
    }

    // stat file
    fs.stat(filesystem, pos, err);

    // push info into source list
    sources.push_back(fs);
    return sources.size() - 1;
}

bool SourceList_t::isChanged(const FilesystemInterface_t* filesystem) const {
    Error_t err;
    Error_t::Position_t pos;
    // run through source list
    for (std::vector<FileStat_t>::const_iterator isources = sources.begin();
         isources != sources.end(); ++isources) {
        // stat file
        FileStat_t fs(isources->filename);
        if (fs.stat(filesystem, pos, err) && isources->valid)
            return true;
        // compare with cached value
        if (fs != *isources) return true;
    }

    // nothing changed
    return false;
}

std::string SourceList_t::getSource(unsigned int position) const {
    if (position < sources.size())
        return sources[position].filename;
    return std::string();
}

} // namespace Teng

