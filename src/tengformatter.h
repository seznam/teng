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
 * $Id: tengformatter.h,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Teng formater (a writer adapter).
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-25  (vasek)
 *             Created.
 */


#ifndef _TENGFORMATTER_H
#define _TENGFORMATTER_H

#include <string>
#include <stack>
#include <utility>

#include <tengerror.h>
#include <tengwriter.h>

using namespace std;

namespace Teng {

/** @short Filter for formatting whitespaces in data.
 */
class Formatter_t {
public:
    /** @short Mode of filtering whitespaces.
     */
    enum Mode_t {
        MODE_INVALID     = -1, /**< Invalid mode. */
        MODE_PASSWHITE   = 0,  /**< Passes whitespaces verbatim. */
        MODE_NOWHITE,          /**< Discards all whitespaces. */
        MODE_ONESPACE,         /**< Truncate run of whitespaces to one
                                    space. */
        MODE_STRIPLINES,       /**< Remove leading and trailing spaces
                                    from line.*/
        MODE_JOINLINES,        /**< Join lines into one long
                                    line. Leading whitespaces on lines
                                    are removed.*/
        MODE_NOWHITELINES,     /**< Remove empty lines. Empty line
                                    consists only from whitespaces. */
    };

    /** @short Create new formatter.
     *  @param writer output writer
     *  @param initialMode initial mode of formatting
     */
    Formatter_t(Writer_t &writer, Mode_t initialMode = MODE_PASSWHITE);

    /** @short Write string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const string &str);

    /** @short Flushes buffered data.
     *  @return 0 OK, !0 error
     */
    int flush();

    /** @short Pushes new formatting mode to the stack.
     *  @param mode new formatting mode
     *  @return 0 OK, !0 error
     */
    int push(Mode_t mode);

    /** @short Pops formatting mode from stack.
     *  Ressambles state before pop.
     *  @return old formatting mode
     */
    Mode_t pop();

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    Formatter_t(const Formatter_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    Formatter_t operator=(const Formatter_t&);

    /** @short Process sequence of spaces.
     *  @param spaceBlock block of spaces
     *  @return 0 OK, !0 error
     */
    int process(pair<string::const_iterator,
                string::const_iterator> spaceBlock);

    /** @short Process sequence of spaces.
     *  @param str white string
     *  @return 0 OK, !0 error
     */
    int process(string &str);

    /** @short Output writer.
     */
    Writer_t &writer;

    /** @short Stack of formatting modes.
     */
    stack<Mode_t> modeStack;

    /** @short Buffer of whitespaces from previous run.
     */
    string buffer;
};

} // namespace Teng

#endif // _TENGFORMATTER_H
