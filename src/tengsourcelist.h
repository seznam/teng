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
 * $Id: tengsourcelist.h,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Teng list of sources.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-23  (vasek)
 *             Created.
 */


#ifndef _TENGSOURCELIST_H
#define _TENGSOURCELIST_H

#include <string>
#include <vector>

#include <sys/types.h>
#include <time.h>

#include "tengerror.h"

using namespace std;

namespace Teng {

/**
 * @short Holds statistic about file.
 *
 * Used for detection of content change.
 */
struct FileStat_t {
    /**
     * @short Creates new file statistics.
     *
     * @param filename associated file name.
     */
    FileStat_t(const string &filename = string())
        : filename(filename), inode(0), size(0),
          mtime(0), ctime(0), valid(false)
    {}


    /**
     * @short Stat file.
     *
     * @param pos position in current file
     * @param err error logger
     * @return 0 OK !0 error
     */
    int stat(const Error_t::Position_t &pos,
             Error_t &err);

    /**
     * @short Compares two statistics
     *
     * @param fs compared value
     * @return true if values are the same false otherwise
     */
    bool operator==(const FileStat_t &fs) const {
        return ((filename == fs.filename) && (inode == fs.inode) &&
                (size == fs.size) && (mtime == fs.mtime) &&
                (ctime == fs.ctime));
    }

    /**
     * @short Compares two statistics
     *
     * @param fs compared value
     * @return true if values are different false otherwise
     */
    bool operator!=(const FileStat_t &fs) const {
        return !operator==(fs);
    }

    /**
     * @short Compares filenames
     *
     * @param fs compared value
     * @return true if values are different false otherwise
     */
    bool operator<(const FileStat_t &fs) const {
        return filename < fs.filename;
    }

    /**
     * @short Name of associated file.
     */
    string filename;

    /**
     * @short Inode of file.
     */
    ino_t inode;

    /**
     * @short Size of file.
     */
    off_t size;

    /**
     * @short Last modification of file.
     */
    time_t mtime;

    /**
     * @short Last attribute modification of file.
     */
    time_t ctime;

    /**
     * @short Indicates that data came from stat(2).
     */
    bool valid;
};

/**
 * @short List of source files.
 */
class SourceList_t {
public:
    /**
     * @short Crrates new (empty) source list.
     */
    SourceList_t()
        : sources()
    {}
    
    /**
     * @short Adds new source into the list.
     *
     * @param source filename of source
     * @param pos position in current file
     * @param err error logger
     * @return position of added source in list
     */
    unsigned int addSource(const string &source,
                           const Error_t::Position_t &pos,
                           Error_t &err);

    /**
     * @short Check validity of all sources.
     *
     * Stats files and compares currant data with cached.
     *
     * @return 0 OK, !0 changed or error during check
     */
    int check() const;

    /**
     * @short Appends another source list.
     */
    void append(const SourceList_t &sourceList) {
        sources.insert(sources.end(), sourceList.sources.begin(),
                       sourceList.sources.end());
    }
    
    string getSource(unsigned int position) const;

    inline unsigned int size() const {
        return sources.size();
    }

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    SourceList_t(const SourceList_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    SourceList_t operator=(const SourceList_t&);

    /**
     * @short List of source files.
     */
    vector<FileStat_t> sources;
};

} // namespace Teng

#endif // _TENGSOURCELIST_H
