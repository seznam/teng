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

#include "position.h"
#include "identifier.h"
#include "contenttype.h"
#include "teng/value.h"

namespace Teng {

// forwards
class Regex_t;

/** Allowed operation codes.
 */
enum class OPCODE {
    NOOP,            //!< Does nothing (used as the first instruction)
    VAL,             //!< Value literal
    VAR,             //!< Get value from variable
    DICT,            //!< Get value of dictionary identified by variable value
    PRG_STACK_PUSH,  //!< Push value on top of the program stack
    PRG_STACK_POP,   //!< Remove value from top of the program stack
    PRG_STACK_AT,    //!< Get value from the top of program stack
    UNARY_PLUS,      //!< +
    UNARY_MINUS,     //!< -
    PLUS,            //!< Addition
    MINUS,           //!< Substraction
    MUL,             //!< Multiplication
    DIV,             //!< Division
    MOD,             //!< Modulo
    BIT_AND,         //!< Bitwise AND
    BIT_XOR,         //!< Bitwise XOR
    BIT_OR,          //!< Bitwise OR
    BIT_NOT,         //!< Bitwise NOT
    AND,             //!< Pseudo-logic AND
    OR,              //!< Pseudo-logic OR
    NOT,             //!< Logic not
    EQ,              //!< ==
    NE,              //!< !=
    GE,              //!< >=
    GT,              //!< >
    LE,              //!< <
    LT,              //!< <=
    REPEAT,          //!< Repeating a pattern
    CONCAT,          //!< Concatenation [OBSOLETE]
    STR_EQ,          //!< String == [OBSOLETE]
    STR_NE,          //!< String != [OBSOLETE]
    FUNC,            //!< Evaluate function
    JMP_IF_NOT,      //!< Conditional jump
    JMP,             //!< Unconditional jump
    OPEN_FORMAT,     //!< Start formating block
    CLOSE_FORMAT,    //!< End of format block
    OPEN_FRAG,       //!< Start fragment block
    OPEN_ERROR_FRAG, //!< Start error fragment block
    CLOSE_FRAG,      //!< End of fragment block
    OPEN_CTYPE,      //!< Change content type (push new)
    CLOSE_CTYPE,     //!< Change content type (pop)
    OPEN_FRAME,      //!< Used to open new frame of fragments
    CLOSE_FRAME,     //!< Uset to close frame of fragements
    PRINT,           //!< Print onto output
    SET,             //!< Create new variable and assign value
    HALT,            //!< End of program. Relax
    DEBUG_FRAG,      //!< Print data tree (vars & vals) to output
    BYTECODE_FRAG,   //!< Print bytecode -- disassembled program
    PUSH_ROOT_FRAG,  //!< Push root frag on value stack
    PUSH_THIS_FRAG,  //!< Push current frag on value stack
    PUSH_ERROR_FRAG, //!< Push error frag on value stack
    PUSH_FRAG,       //!< Push frag on value stack
    PUSH_FRAG_COUNT, //!< Openned fragment size ($_count) - fast access
    PUSH_FRAG_INDEX, //!< Actual fragment iteration number ($_number)
    PUSH_FRAG_FIRST, //!< Is this the first iteration ($_first)
    PUSH_FRAG_LAST,  //!< Is this the last iteration ($_last)
    PUSH_FRAG_INNER, //!< Is this inner iteration ($_inner)
    PUSH_VAL_COUNT,  //!< Same as PUSH_FRAG_COUNT but reads arg from stack
    PUSH_VAL_INDEX,  //!< Same as PUSH_FRAG_INDEX but reads arg from stack
    PUSH_VAL_FIRST,  //!< Same as PUSH_FRAG_FIRST but reads arg from stack
    PUSH_VAL_LAST,   //!< Same as PUSH_FRAG_LAST but reads arg from stack
    PUSH_VAL_INNER,  //!< Same as PUSH_FRAG_INNER but reads arg from stack
    PUSH_ATTR,       //!< Push attr/frag on value stack
    PUSH_ATTR_AT,    //!< Push value at given index on value stack
    POP_ATTR,        //!< Push parent fragment of current value to stack
    REPR,            //!< Optionaly applies escaping
    QUERY_REPR,      //!< Optionaly applies escaping (query version)
    QUERY_COUNT,     //!< Push fragment list size on stak
    QUERY_TYPE,      //!< Push type of the value on top of stack on stak
    QUERY_DEFINED,   //!< Push the value if 'is defined' [OBSOLETE]
    QUERY_EXISTS,    //!< Push the value if it 'exists' on stack
    QUERY_ISEMPTY,   //!< Push true on stack if fragment resp list is empty
    MATCH_REGEX,     //!< Matching of regular expression
    LOG_SUPPRESS,    //!< Suppressing error log
};

/** Converts opcode to its string representation.
 */
const char *opcode_str(OPCODE opcode);

/** Thrown in debug mode if instruction is casted to invalid type.
 */
struct bad_instr_cast_t: public std::runtime_error {
    bad_instr_cast_t(OPCODE instr_opcode, OPCODE dest_opcode)
        : std::runtime_error(
            std::string("attempt to cast instruction to invalid type")
            + ": instr-type=" + opcode_str(instr_opcode)
            + ", casting-to=" + opcode_str(dest_opcode)
        )
    {}
};

/** Instruction for "teng computer".
  * Syntax & semanthics analyzer creates program (sequence of instructions).
  */
struct Instruction_t {
public:
    /** Print instruction into file stream.
      * @param fp File stream for output.
      */
    void dump(FILE *fp) const;

    /** Print instruction into stream.
      * @param os stream for output
      */
    void dump(std::ostream &os) const;

    /** Returns instruction code.
     */
    OPCODE opcode() const {return opcode_value;}

    /** Returns the name of instruction.
     */
    const char *instr_name() const {return opcode_str(opcode_value);}

    /** Returns position of token in source code that generates this
     * instruction.
     */
    const Pos_t &pos() const {return pos_value;}

    /** Casts this instruction to its real type. Does not any checks, so don't
     * shoot your foot.
     */
    template <typename Impl_t>
    const Impl_t &as() const & {
#ifdef DEBUG
        if (opcode() != Impl_t::instr_opcode)
            throw bad_instr_cast_t(opcode(), Impl_t::instr_opcode);
#endif /* DEBUG */
        return static_cast<const Impl_t &>(*this);
    }

    /** Casts this instruction to its real type. Does not any checks, so don't
     * shoot your foot.
     */
    template <typename Impl_t>
    Impl_t &as() & {
#ifdef DEBUG
        if (opcode() != Impl_t::instr_opcode)
            throw bad_instr_cast_t(opcode(), Impl_t::instr_opcode);
#endif /* DEBUG */
        return static_cast<Impl_t &>(*this);
    }

protected:
    // don't copy
    Instruction_t(const Instruction_t &) = delete;
    Instruction_t &operator=(const Instruction_t &) = delete;

    // but do move
    Instruction_t(Instruction_t &&) = default;
    Instruction_t &operator=(Instruction_t &&) = default;

    /** Create simple instruction without params.
     * @param op Inctruction code.
     * @param pos Position of instruction in source file.
     */
    Instruction_t(OPCODE opcode_value, const Pos_t &pos)
        : opcode_value(opcode_value), pos_value(pos)
    {}

    /** @short Lefts instruction object uninitialized.
     */
    Instruction_t(std::nullptr_t): pos_value(nullptr) {}

    /** The instruction implementation can override dump_params to write its
     * parametr to stream.
     */
    void dump_params(std::ostream &) const {}

    OPCODE opcode_value; //!< operation to perform
    Pos_t pos_value;     //!< the position of instruction in source file
};

struct Noop_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::NOOP;
    Noop_t(const Pos_t &pos = {})
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Dict_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::DICT;
    Dict_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct PrgStackPush_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PRG_STACK_PUSH;
    PrgStackPush_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct PrgStackPop_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PRG_STACK_POP;
    PrgStackPop_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct UnaryPlus_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::UNARY_PLUS;
    UnaryPlus_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct UnaryMinus_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::UNARY_MINUS;
    UnaryMinus_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Plus_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PLUS;
    Plus_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Minus_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::MINUS;
    Minus_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Mul_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::MUL;
    Mul_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Div_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::DIV;
    Div_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Mod_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::MOD;
    Mod_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Concat_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::CONCAT;
    Concat_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Repeat_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::REPEAT;
    Repeat_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct BitAnd_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::BIT_AND;
    BitAnd_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct BitXor_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::BIT_XOR;
    BitXor_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct BitOr_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::BIT_OR;
    BitOr_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct BitNot_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::BIT_NOT;
    BitNot_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Not_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::NOT;
    Not_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct EQ_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::EQ;
    EQ_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct NE_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::NE;
    NE_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct GE_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::GE;
    GE_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct GT_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::GT;
    GT_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct LE_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::LE;
    LE_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct LT_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::LT;
    LT_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct StrEQ_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::STR_EQ;
    StrEQ_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct StrNE_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::STR_NE;
    StrNE_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Halt_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::HALT;
    Halt_t(const Pos_t &pos = {})
        : Instruction_t(instr_opcode, pos)
    {}
};

struct DebugFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::DEBUG_FRAG;
    DebugFrag_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct BytecodeFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::BYTECODE_FRAG;
    BytecodeFrag_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct CloseFormat_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::CLOSE_FORMAT;
    CloseFormat_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct CloseCType_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::CLOSE_CTYPE;
    CloseCType_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct PopAttr_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::POP_ATTR;
    PopAttr_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct PushThisFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_THIS_FRAG;
    PushThisFrag_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
    // provide same iface as push root frag has
    PushThisFrag_t(uint64_t, const Pos_t &pos)
        : PushThisFrag_t(pos)
    {}
};

struct OpenFrame_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::OPEN_FRAME;
    OpenFrame_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct CloseFrame_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::CLOSE_FRAME;
    CloseFrame_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct Repr_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::REPR;
    Repr_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryRepr_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_REPR;
    QueryRepr_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryCount_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_COUNT;
    QueryCount_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryType_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_TYPE;
    QueryType_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryDefined_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_DEFINED;
    QueryDefined_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryExists_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_EXISTS;
    QueryExists_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct QueryIsEmpty_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::QUERY_ISEMPTY;
    QueryIsEmpty_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct LogSuppress_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::LOG_SUPPRESS;
    LogSuppress_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos)
    {}
};

struct PushFragIndex_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG_INDEX;
    template <typename Variable_t>
    PushFragIndex_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushFragCount_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG_COUNT;
    template <typename Variable_t>
    PushFragCount_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushFragFirst_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG_FIRST;
    template <typename Variable_t>
    PushFragFirst_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushFragInner_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG_INNER;
    template <typename Variable_t>
    PushFragInner_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushFragLast_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG_LAST;
    template <typename Variable_t>
    PushFragLast_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_FRAG;
    template <typename Variable_t>
    PushFrag_t(const Variable_t &var, uint64_t frag_offset)
        : Instruction_t(instr_opcode, var.pos),
          name(var.ident.name().str()),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(frag_offset))
    {}
    template <typename Variable_t>
    PushFrag_t(const Variable_t &var)
        : PushFrag_t(var, var.offset.frag)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;      //!< the frag identifier
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct PushValCount_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_VAL_COUNT;
    PushValCount_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct PushValFirst_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_VAL_FIRST;
    PushValFirst_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct PushValLast_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_VAL_LAST;
    PushValLast_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct PushValInner_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_VAL_INNER;
    PushValInner_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct PushValIndex_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_VAL_INDEX;
    PushValIndex_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct PushRootFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_ROOT_FRAG;
    PushRootFrag_t(uint64_t root_frag_offset, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          root_frag_offset(static_cast<uint16_t>(root_frag_offset))
    {}
    PushRootFrag_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          root_frag_offset()
    {}
    void dump_params(std::ostream &os) const;
    uint16_t root_frag_offset;  //!< the offset of root fragment in frame
};

struct Val_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::VAL;
    Val_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          value()
    {}
    template <typename type_t>
    Val_t(type_t &&value, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          value(std::forward<type_t>(value))
    {}
    void dump_params(std::ostream &os) const;
    Value_t value; //!< the literal value (string, int, real, ...)
};

struct Var_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::VAR;
    template <typename Variable_t>
    Var_t(const Variable_t &var, bool escape)
        : Instruction_t(instr_opcode, var.pos),
          name(var.ident.name().str()),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag)),
          escape(escape)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;      //!< the variable identifier
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
    bool escape;           //!< true if variable has to be escaped
};

struct PrgStackAt_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PRG_STACK_AT;
    PrgStackAt_t(std::size_t index, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          index(index)
    {}
    void dump_params(std::ostream &os) const;
    std::size_t index; //!< the index of value on the program stack
};

struct And_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::AND;
    And_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int64_t addr_offset; //!< offset where to jump if AND is not satisfied
};

struct Or_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::OR;
    Or_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int64_t addr_offset; //!< offset where to jump if OR is not satisfied
};

struct Func_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::FUNC;
    Func_t(std::string name, uint32_t nargs, const Pos_t &pos, bool is_udf)
        : Instruction_t(instr_opcode, pos),
          name(std::move(name)), nargs(nargs), is_udf(is_udf)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;    //!< the function name
    std::uint32_t nargs; //!< the number of function arguments
    bool is_udf;         //!< true if function is user defined
};

struct JmpIfNot_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::JMP_IF_NOT;
    JmpIfNot_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int64_t addr_offset; //!< offset where to jump if NOT is satisfied
};

struct Jmp_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::JMP;
    Jmp_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int64_t addr_offset; //!< offset where to jump
};

struct OpenFormat_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::OPEN_FORMAT;
    OpenFormat_t(int64_t mode, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          mode(mode)
    {}
    void dump_params(std::ostream &os) const;
    int64_t mode; //!< the id of data format in format block
};

struct OpenFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::OPEN_FRAG;
    OpenFrag_t(std::string name, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          name(std::move(name)), close_frag_offset(-1)
    {}
    OpenFrag_t(OPCODE opcode, std::string name, const Pos_t &pos)
        : Instruction_t(opcode, pos),
          name(std::move(name)), close_frag_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;          //!< the fragment name
    int64_t close_frag_offset; //!< offset where to jump if frament is missing
};

struct OpenErrorFrag_t: public OpenFrag_t {
    static constexpr auto instr_opcode = OPCODE::OPEN_ERROR_FRAG;
    OpenErrorFrag_t(const Pos_t &pos)
        : OpenFrag_t(instr_opcode, "_error", pos)
    {}
};

struct CloseFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::CLOSE_FRAG;
    CloseFrag_t(const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          open_frag_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int64_t open_frag_offset; //!< offset where to jump to repeat fragment
};

struct Print_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PRINT;
    Print_t(bool print_escape, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          print_escape(print_escape)
    {}
    void dump_params(std::ostream &os) const;
    bool print_escape; //!< do escaping if print escaping is enabled
};

struct Set_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::SET;
    template <typename Variable_t>
    Set_t(const Variable_t &var)
        : Instruction_t(instr_opcode, var.pos),
          name(var.ident.name().str()),
          frame_offset(static_cast<uint16_t>(var.offset.frame)),
          frag_offset(static_cast<uint16_t>(var.offset.frag))
    {}
    void dump_params(std::ostream &os) const;
    std::string name;      //!< the variable identifier
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct OpenCType_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::OPEN_CTYPE;
    OpenCType_t(const ContentType_t::Descriptor_t *ctype, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          ctype(ctype)
    {}
    void dump_params(std::ostream &os) const;
    const ContentType_t::Descriptor_t *ctype; //!< the content type descriptor
};

struct PushAttr_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_ATTR;
    PushAttr_t(std::string name, std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          name(std::move(name)), path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string name; //!< the attribute name
    std::string path; //!< path from rtvar start to this attribute
};

struct PushAttrAt_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_ATTR_AT;
    PushAttrAt_t(std::string path, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          path(std::move(path))
    {}
    void dump_params(std::ostream &os) const;
    std::string path; //!< path from rtvar start to this attribute
};

struct MatchRegex_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::MATCH_REGEX;
    MatchRegex_t(counted_ptr<Regex_t> regex, const Pos_t &pos);
    MatchRegex_t(MatchRegex_t &&) noexcept = default;
    MatchRegex_t &operator=(MatchRegex_t &&) noexcept = default;
    ~MatchRegex_t() noexcept;
    void dump_params(std::ostream &os) const;
    bool matches(const string_view_t &view) const;
    counted_ptr<Regex_t> compiled_value; //!< compiled regex
};

struct PushErrorFrag_t: public Instruction_t {
    static constexpr auto instr_opcode = OPCODE::PUSH_ERROR_FRAG;
    PushErrorFrag_t(bool discard_stack_value, const Pos_t &pos)
        : Instruction_t(instr_opcode, pos),
          discard_stack_value(discard_stack_value)
    {}
    void dump_params(std::ostream &os) const;
    bool discard_stack_value; //!< if true value on the stack top is discarded
};

/** The reason of this struct is lack of explicit template parameters of c'tors
 * in C++.
 */
template <typename Impl_t>
struct InstrType_t {};

/** The static polymorphic instruction box that is used to store instructions
 * in Teng program.
 */
class InstrBox_t: public Instruction_t {
public:
    // don't copy
    InstrBox_t(InstrBox_t &) = delete;
    InstrBox_t(const InstrBox_t &) = delete;
    InstrBox_t &operator=(const InstrBox_t &) = delete;

    /** C'tor - builds instruction inplace.
     */
    template <typename Impl_t, typename... Args_t>
    InstrBox_t(InstrType_t<Impl_t>, Args_t &&...args)
        : Instruction_t(nullptr)
    {new (this) Impl_t(std::forward<Args_t>(args)...);}

    /** C'tor - moves already built instruction into box.
     */
    template <typename ImplArg_t>
    InstrBox_t(ImplArg_t &&other) noexcept;

    /** C'tor: move.
     */
    InstrBox_t(InstrBox_t &&other) noexcept;

    /** Assigment: move.
     */
    InstrBox_t &operator=(InstrBox_t &&other) noexcept {
        if (this != &other) {
            this->~InstrBox_t();
            new (this) InstrBox_t(std::move(other));
        }
        return *this;
    }

    /** D'tor.
     */
    ~InstrBox_t() noexcept;

protected:
    static constexpr auto p_size = sizeof(PushAttr_t) - sizeof(Instruction_t);
    char padding[p_size]; //!< ensure enough space for the biggest instr
};

/** Writes human readable representation of the instruction to ouput stream.
 */
inline std::ostream &operator<<(std::ostream &os, const Instruction_t &instr) {
    instr.dump(os);
    return os;
}

} // namespace Teng

#endif // TENGINSTRUCTION_H

