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
 * $Id: tengsourcelist.cc,v 1.2 2004-12-30 12:42:02 vasek Exp $
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

using namespace std;

using namespace Teng;

int FileStat_t::stat(const Error_t::Position_t &pos,
                     Error_t &err)
{
    // invalidate data;
    valid = false;
    
    // stat given file
    struct stat buf;
    if (::stat(filename.c_str(), &buf)) {
        err.logSyscallError(Error_t::LL_ERROR, pos, "Cannot stat file '" +
                            filename +"'");
        return -1;
    }
    
    // check if not dir
    if (S_ISDIR(buf.st_mode)) {
        err.logError(Error_t::LL_ERROR, pos, "File '" + filename +
                     "' is a directory");
        return -1;
    }
    
    // populate members of fileInfo from stat
    inode = buf.st_ino;
    size = buf.st_size;
    mtime = buf.st_mtime;
    ctime = buf.st_ctime;
    
    // validate data
    valid = true;
    // OK
    return 0;
}

unsigned int SourceList_t::addSource(const string &_source,
                                     const Error_t::Position_t &pos,
                                     Error_t &err)
{
    // normalize filename
    string source = _source;
    tengNormalizeFilename(source);
    
    // create source info
    FileStat_t fs(source);
    
    // try to find existing entry
    vector<FileStat_t>::const_iterator fsources =
        std::find(sources.begin(), sources.end(), fs);
    if (fsources != sources.end()) {
        // entry already present => just return its position
        return fsources - sources.begin();
    }
    
    // stat file
    fs.stat(pos, err);
    
    // push info into source list
    sources.push_back(fs);
    return sources.size() - 1;
}

bool SourceList_t::isChanged() const {
    Error_t err;
    Error_t::Position_t pos;
    // run through source list
    for (vector<FileStat_t>::const_iterator isources = sources.begin();
         isources != sources.end(); ++isources) {
        // stat file
        FileStat_t fs(isources->filename);
        if (fs.stat(pos, err) && isources->valid)
            return true;
        // compare with cached value
        if (fs != *isources) return true;
    }
    
    // nothing changed
    return false;
}

string SourceList_t::getSource(unsigned int position) const
{
    if (position < sources.size())
        return sources[position].filename;
    return string();
}
