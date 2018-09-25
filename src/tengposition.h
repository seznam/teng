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
 * Position in template source.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2003-09-22  (vasek)
 *             Created.
 * 2006-06-21  (sten__)
 *             Removed error duplicities.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGPOSITION_H
#define TENGPOSITION_H

#include <string>

namespace Teng {

// forwards
class Error_t;

/** @short Holds position in file.
 */
struct Pos_t {
    /** @short Lefts position object uninitialized.
     */
    Pos_t(std::nullptr_t) {}

    /** @short Creates new position object.
     *
     * If lineno <= 0 or col < 0, then position within file is not printed.
     *
     * The first argument is pointer to filename that must live as long as
     * Pos_t object.
     *
     * @param filename The pointer to filename.
     * @param lineno Line number (starting with 1).
     * @param col Column on line (starting with 0).
     */
    Pos_t(const std::string *filename, int32_t lineno = 0, int32_t colno = 0)
        : filename(filename? filename: no_filename()),
          lineno(lineno), colno(colno)
    {}

    /** @short Creates new position object.
     *
     * If lineno <= 0 or col < 0, then position within file is not printed.
     *
     * @param lineno Line number (starting with 1).
     * @param col Column on line (starting with 0).
     */
    Pos_t(int32_t lineno = 0, int32_t colno = 0)
        : filename(no_filename()), lineno(lineno), colno(colno)
    {}

    /** Returns true if position points anywhere -> it
     * isn't default constructed. (lineno == 0 is invalid)
     */
    explicit operator bool() const {return lineno;}

    /** Returns filename for positions without given filename.
     */
    static const std::string *no_filename() {
        static const std::string no_file = "(no file)";
        return &no_file;
    }

    /** @short Advances to the beginning of new line.
     */
    void newLine() {++lineno; colno = 0;}

    /** @short Advances column by given offset.
     *
      * @param offset column offset
      */
    void advanceColumn(int32_t offset = 1) {colno += offset;}

    /** @short Advances column to the next tab position assuming that <TAB> is
     * 8 chars long.
     */
    void advanceToTab(int32_t tab = 8) {colno = tab * (colno / tab + 1);}

    /** @short Advances position by given char.
     *
     * - <LF> advances line,
     * - <TAB> advances to next tab,
     * - and any other char advances to nex column.
     *
     * @param c character
     */
    void advance(char c) {
        switch (c) {
        case '\n':
            newLine();
            break;
        case '\t':
            advanceToTab();
            break;
        default:
            ++colno;
            break;
        }
    }

    /** @short Advances position to the end of given string.
     *
     * @param str string
     */
    void advance(const std::string &str) {
        for (char ch: str) advance(ch);
    }

    /** @short Advances position to the end of given string.
     *
     * @param str string
     * @param length length of string
     */
    void advance(const char *str, std::size_t length) {
        advance(str, str + length);
    }

    /** @short Advances position to the end of given string.
     */
    void advance(const char *ipos, const char *epos) {
        for (; ipos != epos; ++ipos)
            advance(*ipos);
    }

    /** Converts position to string representation.
     */
    std::string str() const;

    const std::string *filename; //!< list of sources
    int32_t lineno;              //!< line number
    int32_t colno;               //!< column position in file
};

/**
 * @short Generates string representation of posistion and
 * writes it to the given stream.
 */
std::ostream &operator<<(std::ostream &o, const Pos_t &pos);

/** Comparison operator.
 */
inline bool operator==(const Pos_t &lhs, const Pos_t &rhs) {
    return lhs.colno == rhs.colno
        && lhs.lineno == rhs.lineno
        && *lhs.filename == *rhs.filename;
}

/** Comparison operator.
 */
inline bool operator!=(const Pos_t &lhs, const Pos_t &rhs) {
    return !(lhs == rhs);
}

/** Comparison operator.
 */
inline bool operator<(const Pos_t &lhs, const Pos_t &rhs) {
    if (*lhs.filename < *rhs.filename) return true;
    if (*lhs.filename > *rhs.filename) return false;
    if (lhs.lineno < rhs.lineno) return true;
    if (lhs.lineno > rhs.lineno) return false;
    return lhs.colno < rhs.colno;
}

/** Comparison operator.
 */
inline bool operator>(const Pos_t &lhs, const Pos_t &rhs) {
    if (*lhs.filename > *rhs.filename) return true;
    if (*lhs.filename < *rhs.filename) return false;
    if (lhs.lineno > rhs.lineno) return true;
    if (lhs.lineno < rhs.lineno) return false;
    return lhs.colno > rhs.colno;
}

/** Comparison operator.
 */
inline bool operator>=(const Pos_t &lhs, const Pos_t &rhs) {
    return !(lhs < rhs);
}

/** Comparison operator.
 */
inline bool operator<=(const Pos_t &lhs, const Pos_t &rhs) {
    return !(lhs > rhs);
}

} // namespace Teng

#endif // TENGPOSITION_H

