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
#include <algorithm>

#include "tengutil.h"
#include "tengerror.h"
#include "tengsourcelist.h"

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
     * @short Entry in the cache.
     */
    struct Entry_t {
    public:
        // don't copy
        Entry_t(const Entry_t &) = delete;
        Entry_t &operator=(const Entry_t &) = delete;

        /**
         * @short Creates entry with given data.
         * Pointer is stolen!
         * @param key key of this entry
         * @param data associated value with its key
         * @param serial serial number of data
         * @param dependSerial serial number of data this entry depends on.
         */
        Entry_t(const Key_t &key,
                Data_t *data,
                uint64_t serial,
                uint64_t dependSerial)
            : data(data), refCount(1), serial(serial),
              dependSerial(dependSerial), valid(true), key(key)
        {}

        /**
         * @short Associated value.
         */
        std::unique_ptr<Data_t> data;

        /** @short Number of referrers owning reference to this entry.
         */
        int refCount;

        /** @short Serial number of data.
         */
        uint64_t serial;

        /** @short Serial number of data this entry depends on.
         */
        uint64_t dependSerial;

        /** @short Indicates data validity.
         *  Invalidated on insertion of other data under same key
         */
        bool valid;

        /** @short Key under which this key is in the cache.
         */
        const Key_t key;
    };

    /**
     * @short LRU.
     */
    class LRU_t {
    public:
        /**
         * @short Touches entry -> moves entry at begin of vector
         */
        void hit(Entry_t *entry) {
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
        Entry_t *popLeastRecentlyUsed() {
            for (auto ilru = lru.rbegin(); ilru != lru.rend(); ++ilru) {
                if ((*ilru)->refCount <= 0) {
                    Entry_t *entry = *ilru;
                    lru.erase(std::prev(ilru.base()));
                    return entry;
                }
            }
            return nullptr;
        }

        /**
         * @short Erases entry from lru if present.
         */
        Entry_t *popEntry(Entry_t *entry) {
            auto ilru = std::remove(lru.begin(), lru.end(), entry);
            if (ilru == lru.end())
                return nullptr;
            lru.erase(ilru, lru.end());
            return entry;
        }

        /**
         * @short Adds new entry to the lru.
         */
        void insert(Entry_t *entry) {
            lru.insert(lru.begin(), entry);
        }

    private:
        /**
         * @short the lru storage.
         */
        std::vector<Entry_t *> lru;
    };

    /**
     * @short Mapping keys to entries.
     */
    using EntryCache_t = std::map<Key_t, Entry_t *>;

    /**
     * @short Mapping data to entries.
     */
    using EntryBackCache_t = std::map<const Data_t *, std::unique_ptr<Entry_t>>;

    /**
     * @short Creates empty cache.
     */
    Cache_t(unsigned int maximalSize = DEFAULT_MAXIMAL_SIZE)
        : cache(), backcache(), lru(), maximalSize(maximalSize)
    {}

    /**
     * @short Maximal size of cache.
     */
    static const unsigned int DEFAULT_MAXIMAL_SIZE = 50;

    /**
     * @short Finds entry in the cache.
     *
     * @param key searched key
     * @param dependSerial serial number of data this data depends on (output)
     * @param serial serial number of cached data (output)
     * @return found data or 0 when not found
     */
    const Data_t *
    find(const Key_t &key, uint64_t &dependSerial, uint64_t *serial = 0) const {
        // search for entry
        auto fcache = cache.find(key);
        // if not found, report it
        if (fcache == cache.end())
            return nullptr;

        // assign result
        const Data_t *data = fcache->second->data.get();
        dependSerial = fcache->second->dependSerial;
        if (serial) *serial = fcache->second->serial;

        // increment reference count to this data
        ++fcache->second->refCount;

        // touch entry
        lru.hit(fcache->second);

        // OK
        return data;
    }

    /**
     * @short Adds new entry into cache.
     *
     * Existing data for given key are removed from main cache and
     * invalidated if data pointers differ. Otherwise reference count
     * for entry is incremented.
     *
     * Attempt of insertion of same pointer under different key
     * results in no-op.
     *
     * Pointer is stolen!
     *
     * @param key key of data
     * @param data pointer to data
     * @param dependSerial serial number of data this data depends on
     * @param serial serial number of this data (output)
     * @return !0 OK nullptr error
     */
    const Data_t *
    add(const Key_t &key, Data_t *data,
        uint64_t dependSerial = 0, uint64_t *serial = 0)
    {
        // NULL pointer is not allowed
        if (!data) return nullptr;

        // try to find data in cache
        auto fbackcache = backcache.find(data);
        if (fbackcache != backcache.end()) {
            // data already present
            if (fbackcache->second->key != key) {
                // attempt to insert data under different key
                return nullptr;
            }
        }

        // first serial number
        int newSerial = 0;

        // search for entry
        auto fcache = cache.find(key);
        if (fcache != cache.end()) {
            if (fcache->second->data.get() == data) {
                // attempt to insert same data
                // increment reference
                ++fcache->second->refCount;

                // set serial if asked
                if (serial) *serial = fcache->second->serial;
                // return data
                return data;
            }
            // invalidate entry
            fcache->second->valid = false;
            remove(lru.popEntry(fcache->second));

            // increase serial number
            newSerial = fcache->second->serial + 1;
        }

        // if size is greater than limit kill some entry
        if (cache.size() >= maximalSize) {
            // remove least recently used
            remove(lru.popLeastRecentlyUsed());
        }

        // create new entry (defaults to have one reference)
        auto entry
            = std::make_unique<Entry_t>(key, data, newSerial, dependSerial);

        // set serial if asked
        if (serial) *serial = newSerial;

        // insert new entry into the LRU
        lru.insert(entry.get());
        // insert new entry into the cache
        cache.insert({key, entry.get()});
        // insert new entry into the backcache
        backcache.emplace(data, std::move(entry));
        // return data;
        return data;
    }

    /**
     * @short Releases entry.
     *
     * Reference of data holder (entry) is decremented.  if entry is
     * invalid and has losts last reference it is removed from back
     * mapping cache and deleted.
     *
     * @param data pointer to data
     * @return 0 OK !0 error
     */
    int release(const Data_t *data) {
        // try to find data in back mapping cache
        auto fbackcache = backcache.find(data);
        if (fbackcache == backcache.end()) {
            // not found
            return -1;
        }

        // get entry
        Entry_t *entry = fbackcache->second.get();
        // decremente reference count if positive
        if (entry->refCount > 0) --entry->refCount;
        if ((entry->refCount <= 0) && !entry->valid) {
            // no referrers and invalid entry => terminate it
            backcache.erase(fbackcache);
            delete entry;
        }
        return 0;
    }

private:
    // don't copy
    Cache_t(const Cache_t &) = delete;
    Cache_t &operator=(const Cache_t &) = delete;

    /** @short Remove entry from cache.
     *  If entry has no referrers data are deleted.
     */
    void remove(Entry_t *entry) {
        // nullptr can't be removed
        if (!entry) return;

        // remove entry from cache
        for (auto icache = cache.begin(); icache != cache.end(); ++icache) {
            if (icache->second == entry) {
                cache.erase(icache);
                break;
            }
        }

        // if there are no referrers and entry is invalid => terminate it
        if ((entry->refCount <= 0) && !entry->valid) {
            // try to find data in back mapping cache
            auto fbackcache = backcache.find(entry->data.get());
            if (fbackcache != backcache.end())
                backcache.erase(fbackcache);
            delete entry;
        }
    }

    /**
     * @short Cache.
     */
    EntryCache_t cache;

    /**
     * @short Back mapping cache.
     */
    EntryBackCache_t backcache;

    /** @short LRU of entries.
     */
    mutable LRU_t lru;

    /** @short Maximal size of cache.
     */
    unsigned int maximalSize;
};

} // namespace Teng

#endif // TENGCACHE_H

