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
 * $Id: tengformatter.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
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


#ifndef TENGFORMATTER_H
#define TENGFORMATTER_H

#include <string>
#include <stack>
#include <utility>

#include "teng/error.h"
#include "teng/stringview.h"
#include "teng/writer.h"

namespace Teng {

/** @short Filter for formatting whitespaces in data.
 */
class Formatter_t {
public:
    /** @short Mode of filtering whitespaces.
     */
    enum Mode_t {
        MODE_COPY_PREV   = -2, //!< used when format is unrecognized
        MODE_INVALID     = -1, //!< invalid mode
        MODE_PASSWHITE   = 0,  //!< passes whitespaces verbatim
        MODE_NOWHITE,          //!< discards all whitespaces
        MODE_ONESPACE,         //!< truncate run of whitespaces to one space
        MODE_STRIPLINES,       //!< remove leading and trailing spaces from line
        MODE_JOINLINES,        //!< join lines into one long line
        MODE_NOWHITELINES,     //!< remove empty and whitespace only lines
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
    int write(string_view_t str);

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

    /** @short Returns formatting mode on top of the stack.
     *  @return old formatting mode
     */
    Mode_t top() {return modeStack.empty()? MODE_PASSWHITE: modeStack.top();}

private:
    // don't copy
    Formatter_t(const Formatter_t &) = delete;
    Formatter_t &operator=(const Formatter_t &) = delete;

    /** @short Output writer.
     */
    Writer_t &writer;

    /** @short Stack of formatting modes.
     */
    std::stack<Mode_t> modeStack;

    /** @short Buffer of whitespaces from previous run.
     */
    std::string buffer;
};

/** Returns format enum from format name.
 */
Formatter_t::Mode_t resolveFormat(const string_view_t &name);

} // namespace Teng

#endif // TENGFORMATTER_H

