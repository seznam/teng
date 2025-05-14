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
 * $Id: tengprocessor.cc,v 1.15 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor. Instruction pointer.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Cleated.
 */

#ifndef TENGINSTRUCTIONPOINTER_H
#define TENGINSTRUCTIONPOINTER_H

#include "instruction.h"
#include "program.h"
#include "processor.h"

namespace Teng {

/** Some, even whole, part of program.
 */
struct SubProgram_t {
    const Instruction_t &operator[](int64_t i) const {return program[i];}
    const int64_t start; //!< pointer to the first instruction in program
    const int64_t end;   //!< pointer to one past last instruction in program
    const Program_t &program; //!< the whole program
};

/** Represents safe instruction pointer.
 */
struct InstructionPointer_t {
    /** C'tor.
     */
    InstructionPointer_t(const SubProgram_t &program)
        : value(program.start), program(program)
    {}

    /** Moves instruction pointer to given address.
     */
    InstructionPointer_t &operator=(int64_t new_value) {
        if (new_value >= program.end)
            throw std::runtime_error("instruction pointer overflow");
        if (new_value < program.start)
            throw std::runtime_error("instruction pointer underflow");
        value = new_value;
        return *this;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int64_t operator++() {
        *this += 1;
        return value;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int64_t operator++(int) {
        int64_t tmp = value;
        *this += 1;
        return tmp;
    }

    /** Decrements the instruction pointer (it check boundaries).
     */
    int64_t operator--() {
        *this -= 1;
        return value;
    }

    /** Decrements the instruction pointer (it check boundaries).
     */
    int64_t operator--(int) {
        int64_t tmp = value;
        *this -= 1;
        return tmp;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int64_t operator+=(int64_t incr) {
        if (incr == -1)
            throw std::runtime_error("never ending loop detected");
        return value = *this + incr;
    }

    /** Decrements the instruction pointer (it check boundaries).
     */
    int64_t operator-=(int64_t incr) {
        if (incr == 1)
            throw std::runtime_error("never ending loop detected");
        return value = *this - incr;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int64_t operator+(int64_t incr) const {
        auto new_value = value + incr;
        if (new_value > program.end)
            throw std::runtime_error("instruction pointer overflow");
        return new_value;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int64_t operator-(int64_t incr) const {
        auto new_value = value - incr;
        if (new_value < program.start)
            throw std::runtime_error("instruction pointer underflow");
        return new_value;
    }

    /** Returns true if value of ip is less than given address.
     */
    int64_t operator<(int64_t addr) const {
        return value < addr;
    }

    /** Returns numeric value of instruction pointer.
     */
    int64_t operator*() const {return value;}

protected:
    int64_t value;               //!< never will be changed to unsigned !!
    const SubProgram_t &program; //!< evaluated program
};

} // namespace Teng

#endif /* TENGINSTRUCTIONPOINTER_H */

