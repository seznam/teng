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

#include <stdio.h>
#include <string>
#include <vector>
#include <iosfwd>

#include "tengparservalue.h"

using namespace std;

namespace Teng {

struct Identifier_t {
    /** @short Name of identifier.
     */ 
    string name;
    
    /** @short Context of identifier.
     *
     * For fragments and variable means how many context we must go
     * from the root one. 0 means the root, 1 one below the root etc.
     *
     * When opening new fragment 1 means open new context, 0 mean
     * continue in then current context.
     */
    unsigned short int context;

    /** @short Depth in associated context.
     * 
     * Indicates how deep the variable/fragment is in the context --
     * distance form the root.
     */
    unsigned short int depth;
};

/** Instruction for "teng computer".
  * Syntax & semanthics analyzer creates program
  * (sequence of instructions). */
struct Instruction_t {

    /** Allowed operation codes. */
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
        FRAGCNT, /**< Openned fragment size ($_count) -- fast access. */
        XFRAGCNT, /**< Not open fragment size ($_count) --  slow access. */
        FRAGITR, /**< Actual fragment iteration number ($_number). */
        FRAGFIRST, /**< Is this the first iteration ($._first)? */
        FRAGLAST, /**< Is this the last iteration ($._last)? */
        FRAGINNER, /**< Is this inner iteration ($._inner)? */
        PRINT, /**< Print onto output. */
        SET, /**< Create new variable and assign value. */
        HALT, /**< End of program. Relax. */
        DEBUGING, /**< Print data tree (vars & vals) to output. */
        DEFINED, /**< Test if variable/fargment exists and has non-zero/non-empty value. */
        EXIST, /**< Test if exists variable/fragment. */
        BYTECODE, /**< Print bytecode -- disassembled program. */
        CTYPE, /**< Change content type (push new). */
        ENDCTYPE, /**< Change content type (pop). */
        REPEATFRAG, /**< Recursively repeat fragment tree. */
    };
    
    /** Create simple instruction without params.
      * @param op Inctruction code.
      * @param srcidx Index of the source file into program's source list.
      * @param line Line number in the source.
      * @param col Column number in the source. */
    inline Instruction_t(OpCode_t op, int srcidx, int line, int col)
        : operation(op), sourceIndex(srcidx), line(line), column(col) {}
    
    /** Create instruction with value-struct param.
      * @param op Inctruction code.
      * @param val Instruction's own operand.
      * @param srcidx Index of the source file into program's source list.
      * @param line Line number in the source.
      * @param col Column number in the source. */
    inline Instruction_t(OpCode_t op, ParserValue_t val,
                         int srcidx, int line, int col)
        : operation(op), value(val),
          sourceIndex(srcidx), line(line), column(col) {}
    
    /** Print instruction into file stream.
      * @param fp File stream for output. */
    void dump(FILE *fp) const;
    
    /** Print instruction into stream.
      * @param os stream for output
      * @param ip current instruction pointer (<0 for not to use)
      */
    void dump(ostream &os, int ip = -1) const;

    /** Operation to perform. */
    OpCode_t operation;
    
    /** More data for the operation.
      * (type, string, integer, real). */
    ParserValue_t value;
    
    /** Variable identifier.
      * Special additional data for some operations. */
    Identifier_t identifier;
    
    /* Position in template --
      * that means the instruction was generated from statement
      * at given position in original source file. */
    /** Source file. It is index into source list for saving memory. */
    int sourceIndex;
    /** Line number (counted from 1). */
    int line;
    /** Column number (counted from 0). */
    int column;
};

} // namespace Teng

#endif // TENGINSTRUCTION_H
