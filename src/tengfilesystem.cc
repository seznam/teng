/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004-2018 Seznam.cz, a.s.
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
 */

#include <sys/stat.h>
#include <unistd.h>

#include <functional>
#include <stdexcept>
#include <stdio.h>

#include "tengfilesystem.h"
#include "tengplatform.h"
#include "tengutil.h"

namespace Teng {
    
static std::string makeFilename(const std::string& root, const std::string& filename_)
{
    std::string filename = 
        (!root.empty() && !filename_.empty() && filename_[0] != '/')
        ? root + "/" + filename_
        : filename_;

    tengNormalizeFilename(filename);
    return filename;
}

template<class S>
static void hashCombine(std::size_t& seed, S const& value)
{
    std::size_t hash = std::hash<S>{}(value);
    seed ^= hash + 0x9e3779b9 + (seed<<6) + (seed>>2); // or use boost::hash_combine
}

Filesystem_t::Filesystem_t(const std::string& root_)
    : root(root_)
{
    // if not absolute path, prepend current working directory
    if (root.empty() || !ISROOT(root)) {
        char cwd[2048];
        if (!getcwd(cwd, sizeof(cwd))) {
            throw std::runtime_error("Cannot get cwd.");
        }
        root = std::string(cwd) + '/' + root;
    }
}

std::string Filesystem_t::read(const std::string& filename_) const
{
    std::string result;
    
    std::string filename = makeFilename(root, filename_);

    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == 0) throw std::runtime_error("Cannot open input file '" + filename + "'");

    char buf[1024];
    int i;
    while ((i = fread(buf, 1, sizeof(buf), fp)) > 0) {
        result.append(buf, i);
    }

    if (ferror(fp)) {
        fclose(fp);
        throw std::runtime_error("ferror(" + filename + ")");
    }
    fclose(fp);

    return result;
}

size_t Filesystem_t::hash(const std::string& filename_) const
{
    std::size_t seed = 0;
    
    std::string filename = makeFilename(root, filename_);

    // stat given file
    struct stat buf;
    if (::stat(filename.c_str(), &buf)) {
        throw std::runtime_error("Cannot stat file '" + filename + "'");
    }

    // check if not dir
    if (S_ISDIR(buf.st_mode)) {
        throw std::runtime_error("File '" + filename + "' is a directory");
    }

    hashCombine(seed, buf.st_ino);   // unsigned long int - inode number
    hashCombine(seed, buf.st_size);  // long - total size, in bytes
    hashCombine(seed, buf.st_mtime); // long - time of last modification
    hashCombine(seed, buf.st_ctime); // long - time of last status change

    return seed;
}

} // namespace Teng
