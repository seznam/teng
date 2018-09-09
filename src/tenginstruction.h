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
#include "tengcontenttype.h"
#include "tengvalue.h"

// forwards
namespace pcrepp {class Pcre;}

namespace Teng {

/** Allowed operation codes.
 */
enum class OPCODE {
    NOOP,           //!< Does nothing (used as the first instruction)
    VAL,            //!< Value literal
    VAR,            //!< Get value from variable
    DICT,           //!< Get value of dictionary item based on variable content
    PUSH,           //!< Push value on top of the program stack
    POP,            //!< Remove value from top of the program stack
    STACK,          //!< Get value from the top of program stack
    ADD,            //!< Addition
    SUB,            //!< Substraction
    MUL,            //!< Multiplication
    DIV,            //!< Division
    MOD,            //!< Modulo
    CONCAT,         //!< Concatenation
    REPEAT,         //!< Repeating a pattern
    BITAND,         //!< Bitwise AND
    BITXOR,         //!< Bitwise XOR
    BITOR,          //!< Bitwise OR
    BITNOT,         //!< Bitwise NOT
    AND,            //!< Pseudo-logic AND
    OR,             //!< Pseudo-logic OR
    NOT,            //!< Logic not
    EQ,             //!< ==
    NE,             //!< !=
    GE,             //!< >=
    GT,             //!< >
    LE,             //!< <
    LT,             //!< <=
    STREQ,          //!< String == (OBSOLETE)
    STRNE,          //!< String != (OBSOLETE)
    FUNC,           //!< Evaluate function
    JMPIFNOT,       //!< Conditional jump
    JMP,            //!< Unconditional jump
    FORMAT,         //!< Start formating block
    ENDFORMAT,      //!< End of format block
    FRAG,           //!< Start fragment block
    ENDFRAG,        //!< End of fragment block
    FRAGCNT,        //!< Openned fragment size ($_count) - fast access
    FRAGINDEX,      //!< Actual fragment iteration number ($_number)
    FRAGFIRST,      //!< Is this the first iteration ($._first)?
    FRAGLAST,       //!< Is this the last iteration ($._last)?
    FRAGINNER,      //!< Is this inner iteration ($._inner)?
    PRINT,          //!< Print onto output
    SET,            //!< Create new variable and assign value
    HALT,           //!< End of program. Relax
    DEBUG_FRAG,     //!< Print data tree (vars & vals) to output
    BYTECODE_FRAG,  //!< Print bytecode -- disassembled program
    CTYPE,          //!< Change content type (push new)
    ENDCTYPE,       //!< Change content type (pop)
    PUSH_ATTR,      //!< Push attr/frag on stack
    PUSH_ROOT_FRAG, //!< Push root frag on frag stack
    PUSH_THIS_FRAG, //!< Push this frag on frag stack
    PUSH_ATTR_AT,   //!< Push value at given index
    POP_ATTR,       //!< Push parent fragment on stack
    REPR,           //!< Convert frag value into value
    REPR_JSONIFY,   //!< Convert frag value into value
    REPR_COUNT,     //!< Convert frag value into value
    REPR_TYPE,      //!< Convert frag value into value
    REPR_DEFINED,   //!< Convert frag value into value [OBSOLETE]
    REPR_EXISTS,    //!< Convert frag value into value
    REPR_ISEMPTY,   //!< Convert frag value into value
    FRAME,          //!< Used to open new frame of fragments
    ENDFRAME,       //!< Uset to close frame of fragements
    REGEX_MATCH     //!< Matching of regular expression
};

/** Converts opcode to its string representation.
 */
const char *opcode_str(OPCODE opcode);

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
    const Impl_t &as() const & {return static_cast<const Impl_t &>(*this);}

    /** Casts this instruction to its real type. Does not any checks, so don't
     * shoot your foot.
     */
    template <typename Impl_t>
    Impl_t &as() & {return static_cast<Impl_t &>(*this);}

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
    void dump_params(std::ostream &os) const {}

    OPCODE opcode_value; //!< operation to perform
    Pos_t pos_value;     //!< the position of instruction in source file
};

struct Noop_t: public Instruction_t {
    Noop_t(const Pos_t &pos)
        : Instruction_t(OPCODE::NOOP, pos)
    {}
};

struct Dict_t: public Instruction_t {
    Dict_t(const Pos_t &pos)
        : Instruction_t(OPCODE::DICT, pos)
    {}
};

struct Push_t: public Instruction_t {
    Push_t(const Pos_t &pos)
        : Instruction_t(OPCODE::PUSH, pos)
    {}
};

struct Pop_t: public Instruction_t {
    Pop_t(const Pos_t &pos)
        : Instruction_t(OPCODE::POP, pos)
    {}
};

struct Add_t: public Instruction_t {
    Add_t(const Pos_t &pos)
        : Instruction_t(OPCODE::ADD, pos)
    {}
};

struct Sub_t: public Instruction_t {
    Sub_t(const Pos_t &pos)
        : Instruction_t(OPCODE::SUB, pos)
    {}
};

struct Mul_t: public Instruction_t {
    Mul_t(const Pos_t &pos)
        : Instruction_t(OPCODE::MUL, pos)
    {}
};

struct Div_t: public Instruction_t {
    Div_t(const Pos_t &pos)
        : Instruction_t(OPCODE::DIV, pos)
    {}
};

struct Mod_t: public Instruction_t {
    Mod_t(const Pos_t &pos)
        : Instruction_t(OPCODE::MOD, pos)
    {}
};

struct Concat_t: public Instruction_t {
    Concat_t(const Pos_t &pos)
        : Instruction_t(OPCODE::CONCAT, pos)
    {}
};

struct Repeat_t: public Instruction_t {
    Repeat_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPEAT, pos)
    {}
};

struct BitAnd_t: public Instruction_t {
    BitAnd_t(const Pos_t &pos)
        : Instruction_t(OPCODE::BITAND, pos)
    {}
};

struct BitXor_t: public Instruction_t {
    BitXor_t(const Pos_t &pos)
        : Instruction_t(OPCODE::BITXOR, pos)
    {}
};

struct BitOr_t: public Instruction_t {
    BitOr_t(const Pos_t &pos)
        : Instruction_t(OPCODE::BITOR, pos)
    {}
};

struct BitNot_t: public Instruction_t {
    BitNot_t(const Pos_t &pos)
        : Instruction_t(OPCODE::BITNOT, pos)
    {}
};

struct Not_t: public Instruction_t {
    Not_t(const Pos_t &pos)
        : Instruction_t(OPCODE::NOT, pos)
    {}
};

struct EQ_t: public Instruction_t {
    EQ_t(const Pos_t &pos)
        : Instruction_t(OPCODE::EQ, pos)
    {}
};

struct NE_t: public Instruction_t {
    NE_t(const Pos_t &pos)
        : Instruction_t(OPCODE::NE, pos)
    {}
};

struct GE_t: public Instruction_t {
    GE_t(const Pos_t &pos)
        : Instruction_t(OPCODE::GE, pos)
    {}
};

struct GT_t: public Instruction_t {
    GT_t(const Pos_t &pos)
        : Instruction_t(OPCODE::GT, pos)
    {}
};

struct LE_t: public Instruction_t {
    LE_t(const Pos_t &pos)
        : Instruction_t(OPCODE::LE, pos)
    {}
};

struct LT_t: public Instruction_t {
    LT_t(const Pos_t &pos)
        : Instruction_t(OPCODE::LT, pos)
    {}
};

struct StrEQ_t: public Instruction_t {
    StrEQ_t(const Pos_t &pos)
        : Instruction_t(OPCODE::STREQ, pos)
    {}
};

struct StrNE_t: public Instruction_t {
    StrNE_t(const Pos_t &pos)
        : Instruction_t(OPCODE::STRNE, pos)
    {}
};

struct Halt_t: public Instruction_t {
    Halt_t(const Pos_t &pos = {})
        : Instruction_t(OPCODE::HALT, pos)
    {}
};

struct DebugFrag_t: public Instruction_t {
    DebugFrag_t(const Pos_t &pos)
        : Instruction_t(OPCODE::DEBUG_FRAG, pos)
    {}
};

struct BytecodeFrag_t: public Instruction_t {
    BytecodeFrag_t(const Pos_t &pos)
        : Instruction_t(OPCODE::BYTECODE_FRAG, pos)
    {}
};

struct Print_t: public Instruction_t {
    Print_t(const Pos_t &pos)
        : Instruction_t(OPCODE::PRINT, pos)
    {}
};

struct EndFormat_t: public Instruction_t {
    EndFormat_t(const Pos_t &pos)
        : Instruction_t(OPCODE::ENDFORMAT, pos)
    {}
};

struct EndCType_t: public Instruction_t {
    EndCType_t(const Pos_t &pos)
        : Instruction_t(OPCODE::ENDCTYPE, pos)
    {}
};

struct PushAttrAt_t: public Instruction_t {
    PushAttrAt_t(const Pos_t &pos)
        : Instruction_t(OPCODE::PUSH_ATTR_AT, pos)
    {}
};

struct PopAttr_t: public Instruction_t {
    PopAttr_t(const Pos_t &pos)
        : Instruction_t(OPCODE::POP_ATTR, pos)
    {}
};

struct PushThisFrag_t: public Instruction_t {
    PushThisFrag_t(const Pos_t &pos)
        : Instruction_t(OPCODE::PUSH_THIS_FRAG, pos)
    {}
};

struct PushRootFrag_t: public Instruction_t {
    PushRootFrag_t(const Pos_t &pos)
        : Instruction_t(OPCODE::PUSH_ROOT_FRAG, pos)
    {}
};

struct Frame_t: public Instruction_t {
    Frame_t(const Pos_t &pos)
        : Instruction_t(OPCODE::FRAME, pos)
    {}
};

struct EndFrame_t: public Instruction_t {
    EndFrame_t(const Pos_t &pos)
        : Instruction_t(OPCODE::ENDFRAME, pos)
    {}
};

struct Repr_t: public Instruction_t {
    Repr_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR, pos)
    {}
};

struct ReprJsonify_t: public Instruction_t {
    ReprJsonify_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_JSONIFY, pos)
    {}
};

struct ReprCount_t: public Instruction_t {
    ReprCount_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_COUNT, pos)
    {}
};

struct ReprType_t: public Instruction_t {
    ReprType_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_TYPE, pos)
    {}
};

struct ReprDefined_t: public Instruction_t {
    ReprDefined_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_DEFINED, pos)
    {}
};

struct ReprExists_t: public Instruction_t {
    ReprExists_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_EXISTS, pos)
    {}
};

struct ReprIsEmpty_t: public Instruction_t {
    ReprIsEmpty_t(const Pos_t &pos)
        : Instruction_t(OPCODE::REPR_ISEMPTY, pos)
    {}
};

struct FragIndex_t: public Instruction_t {
    template <typename Variable_t>
    FragIndex_t(const Variable_t &var)
        : Instruction_t(OPCODE::FRAGINDEX, var.pos),
          frame_offset(var.frame_offset), frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct FragCnt_t: public Instruction_t {
    template <typename Variable_t>
    FragCnt_t(const Variable_t &var)
        : Instruction_t(OPCODE::FRAGCNT, var.pos),
          frame_offset(var.frame_offset), frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct FragFirst_t: public Instruction_t {
    template <typename Variable_t>
    FragFirst_t(const Variable_t &var)
        : Instruction_t(OPCODE::FRAGFIRST, var.pos),
          frame_offset(var.frame_offset), frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct FragInner_t: public Instruction_t {
    template <typename Variable_t>
    FragInner_t(const Variable_t &var)
        : Instruction_t(OPCODE::FRAGINNER, var.pos),
          frame_offset(var.frame_offset), frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct FragLast_t: public Instruction_t {
    template <typename Variable_t>
    FragLast_t(const Variable_t &var)
        : Instruction_t(OPCODE::FRAGLAST, var.pos),
          frame_offset(var.frame_offset), frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct Val_t: public Instruction_t {
    template <typename type_t>
    Val_t(type_t &&value, const Pos_t &pos)
        : Instruction_t(OPCODE::VAL, pos),
          value(std::forward<type_t>(value))
    {}
    void dump_params(std::ostream &os) const;
    Value_t value; //!< the literal value (string, int, real, ...)
};

struct Var_t: public Instruction_t {
    template <typename Variable_t>
    Var_t(const Variable_t &var, bool escape)
        : Instruction_t(OPCODE::VAR, var.pos),
          name(var.ident.name().str()), frame_offset(var.frame_offset),
          frag_offset(var.frag_offset), escape(escape)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;      //!< the variable identifier
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
    bool escape;           //!< true if variable has to be escaped
};

struct Stack_t: public Instruction_t {
    Stack_t(std::size_t index, const Pos_t &pos)
        : Instruction_t(OPCODE::STACK, pos),
          index(index)
    {}
    void dump_params(std::ostream &os) const;
    std::size_t index; //!< the index of value on the program stack
};

struct And_t: public Instruction_t {
    And_t(const Pos_t &pos)
        : Instruction_t(OPCODE::AND, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int32_t addr_offset; //!< offset where to jump if AND is not satisfied
};

struct Or_t: public Instruction_t {
    Or_t(const Pos_t &pos)
        : Instruction_t(OPCODE::OR, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int32_t addr_offset; //!< offset where to jump if OR is not satisfied
};

struct Func_t: public Instruction_t {
    Func_t(std::string name, uint32_t nargs, const Pos_t &pos, bool is_udf)
        : Instruction_t(OPCODE::FUNC, pos),
          name(std::move(name)), nargs(nargs), is_udf(is_udf)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;    //!< the function name
    std::uint32_t nargs; //!< the number of function arguments
    bool is_udf;         //!< true if function is user defined
};

struct JmpIfNot_t: public Instruction_t {
    JmpIfNot_t(const Pos_t &pos)
        : Instruction_t(OPCODE::JMPIFNOT, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int32_t addr_offset; //!< offset where to jump if NOT is satisfied
};

struct Jmp_t: public Instruction_t {
    Jmp_t(const Pos_t &pos)
        : Instruction_t(OPCODE::JMP, pos),
          addr_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int32_t addr_offset; //!< offset where to jump
};

struct Format_t: public Instruction_t {
    Format_t(int64_t mode, const Pos_t &pos)
        : Instruction_t(OPCODE::FORMAT, pos),
          mode(mode)
    {}
    void dump_params(std::ostream &os) const;
    int64_t mode; //!< the id of data format in format block
};

struct Frag_t: public Instruction_t {
    Frag_t(std::string name, const Pos_t &pos)
        : Instruction_t(OPCODE::FRAG, pos),
          name(std::move(name)), endfrag_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;       //!< the fragment name
    int32_t endfrag_offset; //!< offset where to jump if frament is missing
};

struct EndFrag_t: public Instruction_t {
    EndFrag_t(const Pos_t &pos)
        : Instruction_t(OPCODE::ENDFRAG, pos),
          openfrag_offset(-1)
    {}
    void dump_params(std::ostream &os) const;
    int32_t openfrag_offset; //!< offset where to jump to repeat fragment
};

struct Set_t: public Instruction_t {
    template <typename Variable_t>
    Set_t(const Variable_t &var)
        : Instruction_t(OPCODE::SET, var.pos),
          name(var.ident.name().str()), frame_offset(var.frame_offset),
          frag_offset(var.frag_offset)
    {}
    void dump_params(std::ostream &os) const;
    std::string name;      //!< the variable identifier
    uint16_t frame_offset; //!< the offset of frame (NOT fragment!)
    uint16_t frag_offset;  //!< the offset of fragment in frame
};

struct CType_t: public Instruction_t {
    CType_t(const ContentType_t::Descriptor_t *ctype, const Pos_t &pos)
        : Instruction_t(OPCODE::CTYPE, pos),
          ctype(ctype)
    {}
    void dump_params(std::ostream &os) const;
    const ContentType_t::Descriptor_t *ctype; //!< the content type descriptor
};

struct PushAttr_t: public Instruction_t {
    PushAttr_t(std::string name, const Pos_t &pos)
        : Instruction_t(OPCODE::PUSH_ATTR, pos),
          name(std::move(name))
    {}
    void dump_params(std::ostream &os) const;
    std::string name; //!< the attribute name
};

struct RegexMatch_t: public Instruction_t {
    RegexMatch_t(Regex_t regex, const Pos_t &pos);
    RegexMatch_t(RegexMatch_t &&) noexcept = default;
    RegexMatch_t &operator=(RegexMatch_t &&) noexcept = default;
    ~RegexMatch_t() noexcept;
    void dump_params(std::ostream &os) const;
    bool matches(const string_view_t &view) const;
    static uint32_t to_pcre_flags(const Regex_t &regex);
    Regex_t value;                                //!< the regular expression
    std::unique_ptr<pcrepp::Pcre> compiled_value; //!< compiled regex
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
    template <typename Impl_t>
    InstrBox_t(Impl_t &&other) noexcept;

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
    static constexpr auto p_size = sizeof(Val_t) - sizeof(Instruction_t);
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

