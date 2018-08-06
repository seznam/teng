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
 * $Id: tengerror.h,v 1.6 2011-01-19 06:39:45 burlog Exp $
 *
 * DESCRIPTION
 * Teng error handling class.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (vasek)
 *             Created.
 * 2006-06-21  (sten__)
 *             Removed error duplicities.
 */

#ifndef TENGERROR_H
#define TENGERROR_H

#include <string>
#include <vector>
#include <cstring>

#include <tengposition.h>

// TODO(burlog): remove it
#include <iostream>

namespace Teng {

/** Storage of error messages of template parsing, processing and generation.
 */
class Error_t {
public:
    /** @short Level of message */
    enum Level_t {
        DEBUGING = 0, //!< level for debug messages
        WARNING  = 1, //!< level for warnings
        ERROR    = 2, //!< level for error messages
        FATAL    = 3  //!< level for fatal messages
    };

    /** Creates new error logger.
     */
    Error_t(): max_level(DEBUGING), entries() {}

    /** Entry in error log.
     */
    struct Entry_t {
        /** @short Composes log line.
          * @return composed log line
          */
        std::string getLogLine() const;

        /** @short Dumps entry into stream.
         * @param out output stream
         */
        void dump(std::ostream &out) const;

        Level_t level;   //!< level of message
        Pos_t pos;       //!< position in file
        std::string msg; //!< additional message
    };

    /** Returns number of errors in log.
     * @return number of errors
     */
    std::size_t count() const {return entries.size();}

    /** Returns whether any error occurred.
     * @return true if any error occurred, false otherwise
     */
    explicit operator bool() const {return !entries.empty();}

    /** Clears error log.
     */
    void clear() {entries.clear();}

    /** Get raw error log.
      * @return error log
      */
    const std::vector<Entry_t> &getEntries() const {return entries;}

    /** Appends content of another error log.
     * @param err appended log
     */
    void append(const Error_t &err) {
        // increase level if lower than that of err
        if (err.max_level > max_level) max_level = err.max_level;
        // append error log
        entries.insert(entries.end(), err.entries.begin(), err.entries.end());
    }

    /** Appends new entry.
     * @param entry new entry to append
     */
    void append(const Entry_t &entry) {
        // increase level if lower than that of err
        if (entry.level > max_level) max_level = entry.level;
        // append error log
        entries.push_back(entry);
    }

    /** Dumps log into stream.
     * @param out output stream
     */
    void dump(std::ostream &out) const;

    Level_t max_level; //!< Max level of messages (or DEBUGING if no message)

private:
    // don't copy
    Error_t(const Error_t &) = delete;
    Error_t &operator=(const Error_t &) = delete;

    std::vector<Entry_t> entries; //!< List of error entries
};

/** Dumps the entry to given stream.
 */
inline std::ostream &operator<<(std::ostream &o, const Error_t::Entry_t &e) {
    e.dump(o);
    return o;
}

/** Dumps all errors to given stream.
 */
inline std::ostream &operator<<(std::ostream &o, const Error_t &e) {
    e.dump(o);
    return o;
}

/** Comparison operator.
 */
inline bool
operator==(const Error_t::Entry_t &lhs, const Error_t::Entry_t &rhs) {
    return lhs.level == rhs.level
        && lhs.pos == rhs.pos
        && lhs.msg == rhs.msg;
}

/** Comparison operator.
 */
inline bool
operator!=(const Error_t::Entry_t &lhs, const Error_t::Entry_t &rhs) {
    return !(lhs == rhs);
}

} // namespace Teng

#endif // TENGERROR_H

