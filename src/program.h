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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2003-09-24  (stepan)
 *             Created.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGPROGRAM_H
#define TENGPROGRAM_H

#include <cstdio>
#include <vector>

#include "instruction.h"
#include "sourcelist.h"
#include "teng/error.h"

namespace Teng {

/** Program is an instruction flow. Whole template is compiled into single
 * program that can interpret it.
 */
class Program_t {
public:
    // types
    using value_type = InstrBox_t;
    using const_iterator = std::vector<value_type>::const_iterator;
    using iterator = std::vector<value_type>::iterator;

    /** @short Create new program. */
    Program_t(Error_t &error)
        : sources(), error(error), instrs()
    {instrs.reserve(1024);}

    /** Print whole program into file stream.
     * @param fp File stream for output. */
    void dump(FILE *fp) const;

    /** Print whole program into stream.
     * @param fp stream for output. */
    void dump(std::ostream &out) const;

    /** @short Check source files for change.
      * @return 0=OK !0=changed. */
    int isChanged() const {return sources.isChanged();}

    /** @short Return error log.
      * @return Reference to error log object. */
    Error_t &getErrors() {return error;}

    /** @short Return error log.
      * @return Reference to error log object. */
    const Error_t &getErrors() const {return error;}

    /** @short Adds new source into the list.
      * @param filename Filename of source.  */
    std::pair<const std::string *, std::size_t>
    addSource(const std::string &filename) {return sources.push(filename);}

    /** Returns list of sources.
      */
    const SourceList_t &getSources() const {return sources;}

    /** Returns true if program does not contain any instruction.
     */
    bool empty() const {return instrs.empty();}

    /** Returns the number of instructions of the program.
     */
    std::size_t size() const {return instrs.size();}

    /** Truncates whole program.
     */
    void clear() {instrs.clear();}

    /** Returns iterator to the first instruction.
     */
    const_iterator begin() const {return instrs.begin();}

    /** Returns iterator to the first instruction.
     */
    iterator begin() {return instrs.begin();}

    /** Returns iterator one past the last instruction.
     */
    const_iterator end() const {return instrs.end();}

    /** Returns iterator one past the last instruction.
     */
    iterator end() {return instrs.end();}

    /** Removes specified instruction from program.
     */
    iterator erase_from(int64_t pos) {
#ifdef DEBUG
        if (uint64_t(pos) > instrs.size()) {
            throw std::out_of_range(
                std::string("attempt to erase instrs behind the program end")
                + ": program-size=" + std::to_string(instrs.size())
                + ", pos=" + std::to_string(pos)
            );
        }
#endif /* DEBUG */
        return instrs.erase(instrs.begin() + pos, instrs.end());
    }

    /** Returns instruction at the specified index.
     */
    const value_type &operator[](std::size_t i) const {return instrs[i];}

    /** Returns instruction at the specified index.
     */
    value_type &operator[](std::size_t i) {return instrs[i];}

    /** Returns the last instruction of the program.
     */
    const value_type &back() const {return instrs.back();}

    /** Returns the last instruction of the program.
     */
    value_type &back() {return instrs.back();}

    /** Appends new instruction at the end of the program.
     */
    void push_back(value_type &&instr) {instrs.emplace_back(std::move(instr));}

    /** Appends new instruction at the end of the program.
     */
    template <typename Instr_t, typename... args_t>
    void emplace_back(args_t &&...args) {
        InstrType_t<Instr_t> type_tag; // c'tor explicit template param fixture
        instrs.emplace_back(type_tag, std::forward<args_t>(args)...);
    }

    /** Pops the last instruction from the program.
     */
    void pop_back() {instrs.pop_back();}

    /** Removes all instruction from given position.
     */
    void erase(const_iterator ipos) {instrs.erase(ipos);}

protected:
    SourceList_t sources;           //!< all source files for this program
    Error_t &error;                 //!< error logger
    std::vector<value_type> instrs; //!< list of program instructions
};

} // namespace Teng

#endif // TENGPROGRAM_H

