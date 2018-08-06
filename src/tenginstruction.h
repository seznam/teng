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
 * $Id: tenginstruction.h,v 1.6 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng instruction for teng processor.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-20  (stepan)
 *             Created.
 */

#ifndef TENGINSTRUCTION_H
#define TENGINSTRUCTION_H

#include <cstdio>
#include <string>
#include <vector>
#include <iosfwd>

#include "tengposition.h"
#include "tengidentifier.h"
#include "tengparservalue.h"

namespace Teng {

/** Instruction for "teng computer".
  * Syntax & semanthics analyzer creates program (sequence of instructions).
  */
struct Instruction_t {
    // type
    using Value_t = Parser::Value_t;

    /** Allowed operation codes. */
    // TODO(burlog): enum class?
    enum OpCode_t {
        VAL, /**< Value literal. */
        VAR, /**< Get value from variable. */
        DICT, /**< Get value of dictionary item based on variable content. */
        PUSH, /**< Push value on top of the program stack. */
        POP, /**< Remove value from top of the program stack. */
        STACK, /**< Get value from the top of program stack. */
        ADD, /**< Addition. */
        SUB, /**< Substraction. */
        MUL, /**< Multiplication. */
        DIV, /**< Division. */
        MOD, /**< Modulo. */
        CONCAT, /**< Concatenation. */
        REPEAT, /**< Repeating a pattern. */
        BITAND, /**< Bitwise AND. */
        BITXOR, /**< Bitwise XOR. */
        BITOR, /**< Bitwise OR. */
        BITNOT, /**< Bitwise NOT. */
        AND, /**< Pseudo-logic AND. */
        OR, /**< Pseudo-logic OR. */
        NOT, /**< Logic not. */
        NUMEQ, /**< Numeric ==. */
        NUMGE, /**< Numeric >=. */
        NUMGT, /**< Numeric >. */
        STREQ, /**< String ==. */
        FUNC, /**< Evaluate function. */
        JMPIFNOT, /**< Conditional jump. */
        JMP, /**< Unconditional jump. */
        FORM, /**< Start formating block. */
        ENDFORM, /**< End of format block. */
        FRAG, /**< Start fragment block. */
        ENDFRAG, /**< End of fragment block. */
        FRAGCNT, /**< Openned fragment size ($_count) - fast access. */
        NESTED_FRAGCNT, /**< Not open fragment size ($_count) -  slow access. */
        FRAGINDEX, /**< Actual fragment iteration number ($_number). */
        FRAGFIRST, /**< Is this the first iteration ($._first)? */
        FRAGLAST, /**< Is this the last iteration ($._last)? */
        FRAGINNER, /**< Is this inner iteration ($._inner)? */
        PRINT, /**< Print onto output. */
        SET, /**< Create new variable and assign value. */
        HALT, /**< End of program. Relax. */
        DEBUG_FRAG, /**< Print data tree (vars & vals) to output. */
        DEFINED, /**< OBSOLETE */
        ISEMPTY, /**< Test if fargment is empty. */
        EXISTS, /**< Test if exists variable/fragment. */
        BYTECODE_FRAG, /**< Print bytecode -- disassembled program. */
        CTYPE, /**< Change content type (push new). */
        ENDCTYPE, /**< Change content type (pop). */
        PUSH_ATTR, /**< Push attr/frag on stack. */
        PUSH_ROOT_FRAG, /** Push root frag on frag stack */
        PUSH_THIS_FRAG, /** Push this frag on frag stack */
        PUSH_ATTR_AT, /**< Get value at given index */
        REPR, /**< Convert frag value into value */
        REPR_JSONIFY, /**< Convert frag value into value */
        REPR_COUNT, /**< Convert frag value into value */
        REPR_TYPE, /**< Convert frag value into value */
        REPR_DEFINED, /**< Convert frag value into value */
        REPR_EXISTS, /**< Convert frag value into value */
        REPR_ISEMPTY, /**< Convert frag value into value */
        SUPRESS_LOG, /**< Marks start of exist/defined block */
    };

    /** Create simple instruction without params.
      * @param op Inctruction code.
      * @param pos Position of instruction in source file.
      */
    Instruction_t(OpCode_t opcode, const Pos_t &pos)
        : opcode(opcode), pos(pos)
    {}

    /** Create instruction with value-struct param.
      * @param op Inctruction code.
      * @param val Instruction's own operand.
      * @param pos Position of instruction in source file.
      */
    Instruction_t(OpCode_t opcode, Value_t val, const Pos_t &pos)
        : opcode(opcode), value(std::move(val)), pos(pos)
    {}

    /** Create instruction with value-struct param.
      * @param op Inctruction code.
      * @param val1 Instruction's first own operand.
      * @param val2 Instruction's second own operand.
      * @param pos Position of instruction in source file.
      */
    Instruction_t(OpCode_t opcode, Value_t val1, Value_t val2, const Pos_t &pos)
        : opcode(opcode), value(std::move(val1)), opt_value(std::move(val2)),
          pos(pos)
    {}

    /** Print instruction into file stream.
      * @param fp File stream for output. */
    void dump(FILE *fp) const;

    /** Print instruction into stream.
      * @param os stream for output
      */
    void dump(std::ostream &os) const;

    OpCode_t opcode;         //!< Operation to perform
    Value_t value;           //!< optional: value of literal/jump/...
    Value_t opt_value;       //!< optional: another value
    Identifier_t identifier; //!< optional: identifier of variable/frag/...
    Pos_t pos;               //!< the position of instruction in source file
};

/** Writes human readable representation of the instruction to ouput stream.
 */
inline std::ostream &operator<<(std::ostream &os, const Instruction_t &instr) {
    instr.dump(os);
    return os;
}

} // namespace Teng

#endif // TENGINSTRUCTION_H

