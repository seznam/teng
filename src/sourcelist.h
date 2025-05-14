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
 * $Id: tengsourcelist.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
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


#ifndef TENGSOURCELIST_H
#define TENGSOURCELIST_H

#include <ctime>
#include <string>
#include <vector>
#include <memory>

#include "position.h"
#include "teng/error.h"
#include "teng/filesystem.h"

namespace Teng {

/**
 * @short Holds statistic about file (see stat(2)).
 *
 * Used for detection of content change.
 */
struct FileStat_t {
    std::string filename;   //!< name of associated file
    size_t hash; //!< hash of file statistic.
};

/**
 * @short List of source files.
 */
class SourceList_t {
public:
    /** @short Creates new (empty) source list.
     */
    SourceList_t(): sources() {}

    /** @short Adds new source into the list.
     *
     * @param source filename of source
     *
     * @return index of added source in list
     */
    std::pair<const std::string *, std::size_t> push(const FilesystemInterface_t* filesystem, std::string filename);

    /** @short Check validity of all sources.
     *
     * Stats files and compares current data with cached.
     *
     * @return true means modified; false not modified or error
     */
    bool isChanged(const FilesystemInterface_t* filesystem) const;

    /** @short Get source by given index.
     *
     * @param position index in the source list
     * @return filename or empty string on error
     */
    const std::string *operator[](std::size_t i) const;

    /** @short Returns the number of sources.
     */
    std::size_t size() const {return sources.size();}

    /** @short Returns iterator to the first source.
     */
    auto begin() const {return sources.begin();}

    /** @short Returns iterator one past the last source.
     */
    auto end() const {return sources.end();}

private:
    // don't copy
    SourceList_t(const SourceList_t &) = delete;
    SourceList_t &operator=(const SourceList_t &) = delete;

    std::vector<std::unique_ptr<FileStat_t>> sources; //!< list of sources/files
};

} // namespace Teng

#endif // TENGSOURCELIST_H

