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

namespace Teng {

/** Storage of error messages of template parsing, processing and generation.
 */
class Error_t {
public:
    /** @short Level of message */
    enum Level_t {
        DEBUGING = 0, //!< level for debug messages
        WARNING  = 1, //!< level for warnings
        DIAG     = 2, //!< level for diagnostic messages
        ERROR    = 3, //!< level for error messages
        FATAL    = 4  //!< level for fatal messages
    };

    /** Creates new error logger.
     */
    Error_t(): max_level(DEBUGING), entries() {}

    /** Entry in error log.
     */
    struct Entry_t {
        /** @short Composes log line.
         *
         * @return Composed log line that ends with '\n'.
         */
        std::string getLogLine() const;

        /** @short Writes string representation of entry into stream.
         */
        void dump(std::ostream &out) const;

        /** The Pos_t class isn't intentionally used because it contains pointer
         * to string stored in Teng_t structure that can be destroyed idependly
         * on Error_t structure. The destruction of Teng_t structures
         * invalidates all pos structures. Since the Error_t is public API and
         * we want to provide safe API we translate pos structure to ErrorPos_t.
         */
        struct ErrorPos_t {
            ErrorPos_t(std::string filename, int32_t lineno, int32_t colno)
                : filename(filename), lineno(lineno), colno(colno)
            {}
            ErrorPos_t(int32_t lineno, int32_t colno)
                : lineno(lineno), colno(colno)
            {}
            std::string filename; //!< file path
            int32_t lineno;       //!< the line number (starting with 1)
            int32_t colno;        //!< the column number (starting with 0)
        };

        Level_t level;    //!< level of message
        ErrorPos_t pos;   //!< the error pos
        std::string msg;  //!< additional message
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
        if (err.max_level > max_level)
            max_level = err.max_level;

        // append error log
        for (auto entry: err.entries)
            append_sorted(entry);
    }

    /** Appends new entry.
     * @param entry new entry to append
     */
    void append(Entry_t entry) {
        // increase level if lower than that of err
        if (entry.level > max_level)
            max_level = entry.level;

        // append error log
        append_sorted(std::move(entry));
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

    /** Inserts entry to errors vector according to its position in source code.
     */
    void append_sorted(Entry_t entry) {
        entries.push_back(std::move(entry));
        for (int64_t i = entries.size() - 2; i >= 0; --i) {
            auto &lhs = entries[i];
            auto &rhs = entries[i + 1];
            if (lhs.pos.lineno && rhs.pos.lineno) {
                if (lhs.pos.filename == rhs.pos.filename) {
                    if (lhs.pos.lineno < rhs.pos.lineno)
                        break;
                    if (lhs.pos.lineno == rhs.pos.lineno) {
                        if (lhs.pos.colno < rhs.pos.colno)
                            break;
                        if (lhs.pos.colno == rhs.pos.colno) {
                            if (lhs.level <= rhs.level)
                                break;
                        }
                    }
                    std::swap(lhs, rhs);
                }
            }
        }
    }

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
        && lhs.pos.lineno == rhs.pos.lineno
        && lhs.pos.colno == rhs.pos.colno
        && lhs.pos.filename == rhs.pos.filename
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

