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
 * $Id: tengcache.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
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

#include <string>
#include <map>
#include <algorithm>

#include "tengsourcelist.h"
#include "tengutil.h"
#include "tengerror.h"

using namespace std;

namespace Teng {

typedef vector<string> Key_t;

/**
 * @short Creates key for given file.
 * 
 * The key (normalizad filenanme) is added into given key vector.
 *
 * @param root root for relative paths
 * @param filename filename
 * @param key key vector (result)
 * @return 0 OK !0 error
 */
int tengCreateKey(const string &root,
                  const string &filename, vector<string> &key);

/**
 * @short 
 *
 * @param data processed string
 * @param key key vector (result)
 * @return 0 OK !0 error
 */
int tengCreateStringKey(const string &data, vector<string> &key);

/**
 * @short Maps key from source list to cached value.
 */
template <typename DataType_t>
class Cache_t {
public:
    /**
     * @short Entry in the cache.
     */
    struct Entry_t {
    public:
        /**
         * @short Creates entry with given data.
         * Pointer is stolen!
         * @param key key of this entry
         * @param data associated value with its key
         * @param serial serial number of data
         * @param dependSerial serial number of data this entry depends on.
         */
        Entry_t(const Key_t &key, DataType_t *data,
                unsigned long int serial,
                unsigned long int dependSerial)
            : data(data), refCount(1), serial(serial), dependSerial(dependSerial),
              valid(true), key(key)
        {}
        
        /**
         * @short Delete assoicated data.
         */
        ~Entry_t() {
            delete data;
        }

        /**
         * @short Associated value.
         */
        DataType_t *data;

        /** @short Number of referrers owning reference to this entry.
         */
        int refCount;

        /** @short Serial number of data.
         */
        unsigned long int serial;

        /** @short Serial number of data this entry depends on.
         */
        unsigned long int dependSerial;

        /** @short Indicates data validity.
         *  Invalidated on insertion of other data under same key
         */
        bool valid;

        /** @short Key under which this key is in the cache.
         */
        const Key_t key;

   private:
        /**
         * @short Copy constructor intentionally private -- copying
         *        disabled.
         */
        Entry_t(const Entry_t&);

        /**
         * @short Assignment operator intentionally private -- assignment
         *        disabled.
         */
        Entry_t operator=(const Entry_t&);
    };

    /**
     * @short Mapping keys to entries.
     */
    typedef map<Key_t, Entry_t*> EntryCache_t;

    /**
     * @short Mapping data to entries.
     */
    typedef map<const DataType_t*, Entry_t*> EntryBackCache_t;

    /**
     * @short LRU.
     */
    typedef vector<Entry_t*> LRU_t;

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
     * @short Destroy cache.
     */
    ~Cache_t() {
        // run through back-mapping-cache and delete all associated
        // data
        for (typename EntryBackCache_t::iterator
                 ibackcache = backcache.begin();
             ibackcache != backcache.end(); ++ibackcache)
            delete ibackcache->second;
    }

    /**
     * @short Finds entry in the cache.
     *
     * @param key searched key
     * @param dependSerial serial number of data this data depends on (output)
     * @param serial serial number of cached data (output)
     * @return found data or 0 when not found
     */
    const DataType_t* find(const Key_t &key, unsigned long int &dependSerial,
                           unsigned long int *serial = 0)
        const
    {
        // search for entry
        typename EntryCache_t::const_iterator fcache = cache.find(key);
        // if not found, report it
        if (fcache == cache.end())
            return 0;

        // assign result
        const DataType_t *data = fcache->second->data;
        dependSerial = fcache->second->dependSerial;
        if (serial) *serial = fcache->second->serial;

        // increment reference count to this data
        ++fcache->second->refCount;

        // get entry
        Entry_t *entry = fcache->second;

        // find entry in the LRU
        typename LRU_t::iterator ilru = std::find(lru.begin(),
                                                  lru.end(), entry);
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
     * @return 0 OK !0 error
     */
    const DataType_t* add(const Key_t &key, DataType_t *data,
                          unsigned long int dependSerial = 0,
                          unsigned long int *serial = 0)
    {
        // NULL pointer is not allowed
        if (!data) return 0;
        // try to find data in cache
        typename EntryBackCache_t::iterator fbackcache = backcache.find(data);
        if (fbackcache != backcache.end()) {
            // data already present
            if (fbackcache->second->key != key) {
                // attempt to insert data under different key
                return 0;
            }
        }

        // first serial number
        int newSerial = 0;

        // search for entry
        typename EntryCache_t::iterator fcache = cache.find(key);
        if (fcache != cache.end()) {
            if (fcache->second->data == data) {
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
            remove(fcache->second);

            // increase serial number
            newSerial = fcache->second->serial + 1;
        }

        // if size is greater than limit kill some entry
        if (cache.size() >= maximalSize) {
            // find entry with no references
            for (typename LRU_t::reverse_iterator ilru = lru.rbegin();
                 ilru != lru.rend(); ++ilru) {
                if ((*ilru)->refCount <= 0) {
                    // remove it
                    remove(ilru.base() + 1);
                    break;
                }
            }
        }
        
        // create new entry (defaults to have one reference)
        Entry_t *entry = new Entry_t(key, data, newSerial, dependSerial);

        // set serial if asked
        if (serial) *serial = newSerial;

        // insert new entry into the cache
        cache.insert(typename EntryCache_t::value_type(key, entry));
        // insert new entry into the backcache
        backcache.insert(typename EntryBackCache_t::value_type(data, entry));
        // insert new entry into the LRU
        lru.insert(lru.begin(), entry);
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
    int release(const DataType_t *data) {
        // try to find data in back mapping cache
        typename EntryBackCache_t::iterator fbackcache = backcache.find(data);
        if (fbackcache == backcache.end()) {
            // not found
            return -1;
        }

        // get entry
        Entry_t *entry = fbackcache->second;
        // decremente reference count if positive
        if (entry->refCount > 0) --entry->refCount;
        if ((entry->refCount <= 0) && !entry->valid) {
            // no referrers and invalid entry => terminate it 
            backcache.erase(fbackcache);
            delete entry;
        }
        // OK
        return 0;
    }

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    Cache_t(const Cache_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    Cache_t operator=(const Cache_t&);

    /** @short Remove entry from cache and lru.
     *  @param entry removed entry
     */
    void remove(Entry_t *entry) {
        // find entry in the LRU
        typename LRU_t::iterator ilru = std::find(lru.begin(),
                                                  lru.end(), entry);
        remove(ilru);
    }

    /** @short Remove entry from cache and lru.
     *  If entry has no referrers data are deleted.
     *  @param lruEntry removed entry (iterator to the LRU)
     */
    void remove(typename LRU_t::iterator lruEntry) {
        Entry_t *entry = *lruEntry;
        // remove entry from LRU
        if (lruEntry != lru.end())
            lru.erase(lruEntry);

        // remove entry from cache
        for (typename EntryCache_t::iterator icache = cache.begin();
             icache != cache.end(); ++icache) {
            if (icache->second == entry) {
                cache.erase(icache);
                break;
            }
        }

        // if there are no referrers and entry is invalid => terminate it 
        if ((entry->refCount <= 0) && !entry->valid) {
            // try to find data in back mapping cache
            typename EntryBackCache_t::iterator fbackcache =
                backcache.find(entry->data);
            if (fbackcache == backcache.end())
                backcache.erase(fbackcache);
            delete entry;
        }
        // OK
        return;
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
