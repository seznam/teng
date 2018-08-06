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
#include "tenglogging.h"
#include "tengutil.h"

namespace Teng {
namespace {

/**
 * @short Stat file.
 * @param pos position in current file
 * @param err error logger
 */
FileStat_t::Stat_t
stat(const std::string &filename, const Pos_t *pos, Error_t *err) {
    // stat given file
    struct stat buf;
    if (::stat(filename.c_str(), &buf)) {
        if (err && pos) {
            std::string sys = "(" + strerr() + ")";
            logError(*err, *pos, "Cannot stat file '" + filename + "' " + sys);
        }
        return {};
    }

    // check if not dir
    if (S_ISDIR(buf.st_mode)) {
        if (err && pos)
            logError(*err, *pos, "File '" + filename + "' is a directory");
        return {};
    }

    // populate members of stat struct
    return {buf.st_ino, buf.st_size, buf.st_mtime, buf.st_ctime, true};
}

} // namespace

/** @short Returns true if lhs equals to rhs.
 */
bool operator==(const FileStat_t::Stat_t &lhs, const FileStat_t::Stat_t &rhs) {
    return (lhs.inode == rhs.inode)
        && (lhs.size == rhs.size)
        && (lhs.mtime == rhs.mtime)
        && (lhs.ctime == rhs.ctime);
}

/** @short Returns true if lhs does not equal to rhs.
 */
bool operator!=(const FileStat_t::Stat_t &lhs, const FileStat_t::Stat_t &rhs) {
    return !(lhs == rhs);
}

std::pair<const std::string *, std::size_t>
SourceList_t::push(std::string filename, const Pos_t &pos, Error_t &err) {
    // normalize filename
    normalizeFilename(filename);

    // try to find existing entry
    for (std::size_t i = 0; i < sources.size(); ++i)
        if (sources[i]->filename == filename)
            return {&sources[i]->filename, i};

    // stat file
    using ptr_t = std::unique_ptr<FileStat_t>;
    auto new_stat = stat(filename, &pos, &err);
    sources.emplace_back(ptr_t(new FileStat_t{filename, new_stat}));
    return {&sources.back()->filename, sources.size() - 1};
}

bool SourceList_t::isChanged() const {
    for (auto &source: sources) {
        auto new_stat = stat(source->filename, nullptr, nullptr);
        if (new_stat != source->stat) return true;
    }
    return false;
}

const std::string *SourceList_t::operator[](std::size_t i) const {
    static const std::string empty;
    if (i < sources.size())
        return &sources[i]->filename;
    return &empty;
}

} // namespace Teng

