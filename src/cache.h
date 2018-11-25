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
 * $Id: tengcache.h,v 1.3 2007-03-01 09:40:12 vasek Exp $
 *
 * DESCRIPTION
 * Teng cache of files.
 *
 * AUTHOR
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-23  (vasek)
 *             Created.
 */


#ifndef TENGCACHE_H
#define TENGCACHE_H

#include <map>
#include <string>
#include <memory>
#include <tuple>
#include <algorithm>

#include "util.h"
#include "teng/error.h"
#include "sourcelist.h"

namespace Teng {

/**
 * @short Creates key for given file.
 *
 * The key (normalizad filenanme) is added into given key vector.
 *
 * @param root root for relative paths
 * @param filename filename
 * @return key key vector
 */
std::string
createCacheKeyForFilename(const std::string &root, std::string filename);

/**
 * @short Creates key for given string.
 *
 * @param data processed string
 * @return key vector
 */
std::string
createCacheKeyForString(const std::string &data);

/**
 * @short Entry in the cache.
 */
template <typename Data_t>
struct CacheEntry_t {
public:
    /**
     * @short Creates entry with given data.
     *
     * @param data associated value with its key
     * @param dependSerial serial number of data this entry depends on.
     */
    CacheEntry_t(std::shared_ptr<Data_t> data, uint64_t dependSerial)
        : data(std::move(data)), serial(0), dependSerial(dependSerial)
    {}

    std::shared_ptr<Data_t> data; //!< associated value
    uint64_t serial;              //!< serial number of the value
    uint64_t dependSerial;        //!< serial number of the value depends on
};

/**
 * @short LRU.
 */
template <typename Type_t>
class CacheLRU_t {
public:
    /**
     * @short Touches entry -> moves entry at begin of vector
     */
    void hit(Type_t entry) {
        auto ilru = std::find(lru.begin(), lru.end(), entry);
        if (ilru != lru.end()) {
            // ok, entry found in the LRU
            if (ilru != lru.begin()) {
                // move entries one place to the right
                for (; ilru != lru.begin(); --ilru)
                    *ilru = *(ilru - 1);
                // move found entry to the beginning
                *ilru = entry;
            }
        } else {
            // if entry not found (strange...) push it to the front
            // of the LRU
            lru.insert(lru.begin(), entry);
        }
    }

    /**
     * @short Erases least recently used entry from lru and returns it.
     *
     * @return least recently used entry that has been removed or nullptr.
     */
    template <typename unused_t>
    Type_t popLeastRecentlyUsed(unused_t unused) {
        if (lru.empty())
            throw std::range_error(__PRETTY_FUNCTION__);

        // try pop unused entry
        for (auto ilru = lru.rbegin(); ilru != lru.rend(); ++ilru) {
            if (unused(*ilru)) {
                Type_t entry = *ilru;
                lru.erase(std::prev(ilru.base()));
                return entry;
            }
        }

        // if no unused entry has been found, remove the last one
        Type_t entry = lru.back();
        lru.pop_back();
        return entry;
    }

    /**
     * @short Adds new entry to the lru.
     */
    void insert(Type_t entry) {lru.insert(lru.begin(), entry);}

private:
    std::vector<Type_t> lru; //!< the storage
};

/**
 * @short Maps key from source list to cached value.
 */
template <typename Data_t>
class Cache_t {
public:
    /**
     * @short Key type for entries in cache.
     */
    using Key_t = std::vector<std::string>;

    /**
     * @short Entry type.
     */
    using Entry_t = CacheEntry_t<Data_t>;

    /**
     * @short Mapping keys to entries.
     */
    using EntryCache_t = std::map<Key_t, Entry_t>;

    /**
     * @short The type of entry in cache LRU.
     */
    using LRUEntry_t = const typename EntryCache_t::value_type *;

    /**
     * @short LRU for the cache.
     */
    using LRU_t = CacheLRU_t<LRUEntry_t>;

    /**
     * @short Creates empty cache.
     */
    Cache_t(unsigned int maximalSize = DEFAULT_MAXIMAL_SIZE)
        : cache(), lru(),
          maximalSize(maximalSize? maximalSize: DEFAULT_MAXIMAL_SIZE)
    {}

    /**
     * @short Maximal size of cache.
     */
    static const unsigned int DEFAULT_MAXIMAL_SIZE = 50;

    /**
     * @short Finds entry in the cache.
     *
     * @param key searched key
     * @return tuple of <(found data or empty shared_ptr), dependSerial, serial>
     */
    std::tuple<std::shared_ptr<Data_t>, uint64_t, uint64_t>
    find(const Key_t &key) const {
        // search for entry
        auto ientry = cache.find(key);
        if (ientry == cache.end())
            return {{}, 0, 0};

        // touch entry
        lru.hit(&*ientry);

        // return result
        const Entry_t &entry = ientry->second;
        return {entry.data, entry.dependSerial, entry.serial};
    }

    /**
     * @short Adds new entry into cache.
     *
     * @param key key of data
     * @param data pointer to data
     * @param dependSerial serial number of data this data depends on
     *
     * @return serial serial number of added data
     */
    uint64_t add(
        const Key_t &key,
        std::shared_ptr<Data_t> data,
        uint64_t dependSerial = 0
    ) {
        auto ientry = cache.find(key);
        return ientry != cache.end()
            ? update_old(ientry, std::move(data), dependSerial)
            : insert_new(key, std::move(data), dependSerial);
    }

    /**
     * @short Inserts new entry into cache.
     */
    uint64_t insert_new(
        const Key_t &key,
        std::shared_ptr<Data_t> &&data,
        uint64_t dependSerial
    ) {
        // returns true if data pointer is referenced only from cache
        auto unused = [] (LRUEntry_t lru_entry) {
            return lru_entry->second.data.use_count() <= 1;
        };

        // at first, if size of cache is greater then limit kill some entry
        if (cache.size() >= maximalSize)
            cache.erase(lru.popLeastRecentlyUsed(unused)->first);

        // emplace cache entry and update lru
        auto ientry = cache.emplace(key, Entry_t(data, dependSerial)).first;
        lru.insert(&*ientry);

        // return serial of new entry => 0
        return ientry->second.serial;
    }

    /**
     * @short Inserts new entry into cache.
     */
    uint64_t update_old(
        typename EntryCache_t::iterator ientry,
        std::shared_ptr<Data_t> &&data,
        uint64_t dependSerial
    ) {
        // touch entry
        lru.hit(&*ientry);

        // attempt to insert same data
        if (ientry->second.data == data)
            return ientry->second.serial;

        // replace old entry data with fresh one
        Entry_t &entry = ientry->second;
        entry.dependSerial = dependSerial;
        entry.data = std::move(data);

        // increment and return serial
        return ++entry.serial;
    }

private:
    // don't copy
    Cache_t(const Cache_t &) = delete;
    Cache_t &operator=(const Cache_t &) = delete;

    EntryCache_t cache;        //!< the cache
    mutable LRU_t lru;         //!< LRU for cache entries
    unsigned int maximalSize;  //!< Maximal size of cache.
};

} // namespace Teng

#endif // TENGCACHE_H

