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
 * $Id: tengprogram.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
 *
 * DESCRIPTION
 * Teng program. Program is sequence of instructions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-24  (stepan)
 *             Created.
 */

#ifndef TENGPROGRAM_H
#define TENGPROGRAM_H

#include <stdio.h>
#include <vector>

#include "tenginstruction.h"
#include "tengsourcelist.h"
#include "tengerror.h"

using namespace std;

namespace Teng {

/** Program is an instruction flow. Whole template is
  * compiled into single program that can interpret it. */
class Program_t : private vector<Instruction_t> {
public:

    /** @short Create new program. */
    Program_t()
        : sources(), error()
    {}

    /** Print whole program into file stream.
     * @param fp File stream for output. */
    void dump(FILE *fp) const;

    /** @short Check source files for change.
      * @return 0=OK !0=changed. */
    inline int check() const {
        return sources.isChanged();
    }

    /** @short Return error log.
      * @return Reference to error log object. */
    inline Error_t& getErrors() {
        return error;
    }

    /** @short Return error log.
      * @return Reference to error log object. */
    inline const Error_t& getErrors() const {
        return error;
    }

    /** @short Adds new source into the list.
      * @param source Filename of source.
      * @param pos Position in current file. */
    inline unsigned int addSource(const string &source,
                           const Error_t::Position_t &pos) {
        return sources.addSource(source, pos, error);
    }

    /** Get source's filename based on index in source list.
      * @return Absolute filename string.
      * @param position Index into program's source list. */
    inline string getSource(unsigned int position) const {
        return sources.getSource(position);
    }

    inline const SourceList_t& getSources() const {
        return sources;
    }

    using vector<Instruction_t>::empty;

    using vector<Instruction_t>::begin;

    using vector<Instruction_t>::end;

    using vector<Instruction_t>::erase;

    using vector<Instruction_t>::size;

    using vector<Instruction_t>::operator [];

    using vector<Instruction_t>::back;

    using vector<Instruction_t>::push_back;

    using vector<Instruction_t>::pop_back;

    using vector<Instruction_t>::const_iterator;

private:
    
    /** @short All source files for this program. */
    SourceList_t sources;

    /** @short Error logger. */
    Error_t error;
};

} // namespace Teng

#endif // TENGPROGRAM_H
