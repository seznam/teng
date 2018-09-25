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
 * $Id: tengdictionary.h,v 1.3 2005-11-20 11:11:41 vasek Exp $
 *
 * DESCRIPTION
 * Teng dictionary.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (vasek)
 *             Created.
 * 2018-06-14  (burlog)
 *             Cleaned.
 */

#ifndef TENGDICTIONARY_H
#define TENGDICTIONARY_H

#include <map>
#include <iosfwd>
#include <string>
#include <vector>

#include "tengerror.h"
#include "tengsourcelist.h"
#include "tengstringview.h"

namespace Teng {

/**
 * @short Dictionary -- mapping of string to string value.
 *
 * Used for language and parametric dictionaries.
 */
class Dictionary_t {
public:
    /**
     * @short Creates new dictionary object.
     *
     * @param fs_root path of root for locating files
     */
    Dictionary_t(const std::string &fs_root)
        : sources(), err(), fs_root(fs_root),
          expandVars(false), replaceEntries(false)
    {}

    /**
     * @short Destroy dictionary object.
     */
    virtual ~Dictionary_t() = default;

    /**
     * @short Searches for key in dictionary.
     *
     * @param key the key
     * @return found value or 0 when key not found
     */
    virtual const std::string *lookup(const string_view_t &key) const;

    /**
     * @short Dumps dictionary into string. For debugging purposes.
     *
     * @param out output string
     */
    void dump(std::string &out) const;

    /**
     * @short Dumps dictionary into stream. For debugging purposes.
     *
     * @param out output stream
     */
    virtual void dump(std::ostream &out) const;

    /**
     * @short Return source list.
     *
     * @return source list
     */
    const SourceList_t &getSources() const {return sources;}

    /**
     * @short Get error logger.
     *
     * @return error logger
     */
    const Error_t &getErrors() const {return err;}

    /**
     * @short Check source files for change.
     *
     * @return 0 OK !0 changed
     */
    int isChanged() const {return sources.isChanged();}

    /**
     * @short Fills dictionary with data parsed from filename.
     *
     * @param filename name of file to parse
     * @return 0 OK !0 error
     */
    void parse(const std::string &filename);

protected:
    // don't copy
    Dictionary_t(const Dictionary_t &) = delete;
    Dictionary_t &operator=(const Dictionary_t &) = delete;

    /** The result type for inserting parsed entry or directive.
     */
    enum class error_code {
        none,
        unknown_directive,
        invalid_bool,
        invalid_number,
        invalid_enable,
        invalid_disable,
    };

    /** Called when new entry parsed.
     *
     * Doesn't replace existing entry unless replaceEntries is set to true.
     */
    virtual std::string *new_entry(std::string name, std::string value);

    /** Called when new directive parsed.
     */
    virtual error_code
    new_directive(
        const char *name_ptr, std::size_t name_len,
        const char *value_ptr, std::size_t value_len
    );

    // type for dictionary entries
    using Entries_t = std::map<std::string, std::string>;

    Entries_t entries;    //!< the dictionary entries
    SourceList_t sources; //!< source files of dictionary entries
    Error_t err;          //!< the error log
    std::string fs_root;  //!< the filesystem root for all relative paths
    bool expandVars;      //!< expand variables in dict values
    bool replaceEntries;  //!< replace already present entries in dict
};

} // namespace Teng

#endif /* TENGDICTIONARY_H */

