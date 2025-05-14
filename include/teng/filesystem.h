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

#ifndef TENGFILESYSTEM_H
#define TENGFILESYSTEM_H

#include <string>
#include <map>

namespace Teng {

/** @short Abstract filesystem interface.
 */
class FilesystemInterface_t {
public:
    virtual ~FilesystemInterface_t() = default;

    /**
     * @short Read contents from filesystem.
     * @param filename Name of the file in filesystem
     * @return Contents of the file in filesystem
     */
    virtual std::string read(const std::string &filename) const = 0;

    /**
     * @short Make hash from file stats to allow caching as long as an file remains unchanged.
     * @param filename Name of the file in filesystem
     * @return Hash of the file stats
     */
    virtual size_t hash(const std::string &filename) const = 0;
};

/** @short Implementation of filesystem interface backed by real filesystem.
 */
class Filesystem_t : public FilesystemInterface_t {
public:
    Filesystem_t(const std::string& root);
    virtual std::string read(const std::string &filename) const;
    virtual size_t hash(const std::string &filename) const;

protected:
    std::string root;
};

/** @short Implementation of filesystem interface backed by key-value storage.
 */
class InMemoryFilesystem_t : public FilesystemInterface_t {
public:
    virtual std::string read(const std::string &filename) const
    {
        return storage.at(filename);
    }

    virtual size_t hash(const std::string &) const
    {
        return 0; // permanent cache
    }

    /** @short Key-value storage.
     */
    std::map<std::string, std::string> storage;
};

} // namespace Teng

#endif // TENGFILESYSTEM_H
