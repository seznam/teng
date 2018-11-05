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
 * $Id: tenginstruction.cc,v 1.7 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng instruction and program types. Syntax analyzer
 * translate input tokens into sequence of instructions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-20  (stepan)
 *             Created.
 */

#include <cstdio>
#include <string>
#include <vector>
#include <iomanip>
#include <ostream>
#include <streambuf>
#include <unistd.h>

#include "tengvalue.h"
#include "tengregex.h"
#include "tengfilestream.h"
#include "tenginstruction.h"
#include "tengcontenttype.h"

namespace Teng {
namespace {

/** Replaces new lines with "\n".
 */
std::string escapenl(const std::string &str) {
    std::string result;
    result.reserve(str.size() * 1.3);
    for (auto ch: str)
        if (ch == '\n') result.append("\\n");
        else result.push_back(ch);
    return result;
}

/** Pads name up to 32 characters width.
 */
std::string pad(std::string name) {
    return name.append(20 - name.size(), ' ');
}

} // namespace

/** Casts given value to appropriate type and calls given callback with given
 * arguments.
 */
template <typename type_t, typename call_t, typename... args_t>
static auto eval(OPCODE opcode, type_t &self, call_t &&call, args_t &&...args) {
    switch (opcode) {
    case OPCODE::NOOP:
        return call(
            self.template as<Noop_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::VAL:
        return call(
            self.template as<Val_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::VAR:
        return call(
            self.template as<Var_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::DICT:
        return call(
            self.template as<Dict_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PRG_STACK_PUSH:
        return call(
            self.template as<PrgStackPush_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PRG_STACK_POP:
        return call(
            self.template as<PrgStackPop_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PRG_STACK_AT:
        return call(
            self.template as<PrgStackAt_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::UNARY_PLUS:
        return call(
            self.template as<UnaryPlus_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::UNARY_MINUS:
        return call(
            self.template as<UnaryMinus_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PLUS:
        return call(
            self.template as<Plus_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::MINUS:
        return call(
            self.template as<Minus_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::MUL:
        return call(
            self.template as<Mul_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::DIV:
        return call(
            self.template as<Div_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::MOD:
        return call(
            self.template as<Mod_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::CONCAT:
        return call(
            self.template as<Concat_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::REPEAT:
        return call(
            self.template as<Repeat_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::BIT_AND:
        return call(
            self.template as<BitAnd_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::BIT_XOR:
        return call(
            self.template as<BitXor_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::BIT_OR:
        return call(
            self.template as<BitOr_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::BIT_NOT:
        return call(
            self.template as<BitNot_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::AND:
        return call(
            self.template as<And_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OR:
        return call(
            self.template as<Or_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::NOT:
        return call(
            self.template as<Not_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::EQ:
        return call(
            self.template as<EQ_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::NE:
        return call(
            self.template as<NE_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::GE:
        return call(
            self.template as<GE_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::GT:
        return call(
            self.template as<GT_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::LE:
        return call(
            self.template as<LE_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::LT:
        return call(
            self.template as<LT_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::STR_EQ:
        return call(
            self.template as<StrEQ_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::STR_NE:
        return call(
            self.template as<StrNE_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::FUNC:
        return call(
            self.template as<Func_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::JMP_IF_NOT:
        return call(
            self.template as<JmpIfNot_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::JMP:
        return call(
            self.template as<Jmp_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OPEN_FORMAT:
        return call(
            self.template as<OpenFormat_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::CLOSE_FORMAT:
        return call(
            self.template as<CloseFormat_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OPEN_FRAG:
        return call(
            self.template as<OpenFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OPEN_ERROR_FRAG:
        return call(
            self.template as<OpenErrorFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::CLOSE_FRAG:
        return call(
            self.template as<CloseFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG_COUNT:
        return call(
            self.template as<PushFragCount_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG_INDEX:
        return call(
            self.template as<PushFragIndex_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG_FIRST:
        return call(
            self.template as<PushFragFirst_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG_LAST:
        return call(
            self.template as<PushFragLast_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG_INNER:
        return call(
            self.template as<PushFragInner_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_VAL_COUNT:
        return call(
            self.template as<PushValCount_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_VAL_INDEX:
        return call(
            self.template as<PushValIndex_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_VAL_FIRST:
        return call(
            self.template as<PushValFirst_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_VAL_LAST:
        return call(
            self.template as<PushValLast_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_VAL_INNER:
        return call(
            self.template as<PushValInner_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_FRAG:
        return call(
            self.template as<PushFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PRINT:
        return call(
            self.template as<Print_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::SET:
        return call(
            self.template as<Set_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::HALT:
        return call(
            self.template as<Halt_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::DEBUG_FRAG:
        return call(
            self.template as<DebugFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::BYTECODE_FRAG:
        return call(
            self.template as<BytecodeFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OPEN_CTYPE:
        return call(
            self.template as<OpenCType_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::CLOSE_CTYPE:
        return call(
            self.template as<CloseCType_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_ATTR:
        return call(
            self.template as<PushAttr_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_ROOT_FRAG:
        return call(
            self.template as<PushRootFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_THIS_FRAG:
        return call(
            self.template as<PushThisFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_ERROR_FRAG:
        return call(
            self.template as<PushErrorFrag_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::PUSH_ATTR_AT:
        return call(
            self.template as<PushAttrAt_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::POP_ATTR:
        return call(
            self.template as<PopAttr_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::REPR:
        return call(
            self.template as<Repr_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_REPR:
        return call(
            self.template as<QueryRepr_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_COUNT:
        return call(
            self.template as<QueryCount_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_TYPE:
        return call(
            self.template as<QueryType_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_DEFINED:
        return call(
            self.template as<QueryDefined_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_EXISTS:
        return call(
            self.template as<QueryExists_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::QUERY_ISEMPTY:
        return call(
            self.template as<QueryIsEmpty_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::OPEN_FRAME:
        return call(
            self.template as<OpenFrame_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::CLOSE_FRAME:
        return call(
            self.template as<CloseFrame_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::REGEX_MATCH:
        return call(
            self.template as<RegexMatch_t>(),
            std::forward<args_t>(args)...
        );
    case OPCODE::LOG_SUPPRESS:
        return call(
            self.template as<LogSuppress_t>(),
            std::forward<args_t>(args)...
        );
    }
}

const char *opcode_str(OPCODE opcode) {
    switch (opcode) {
    case OPCODE::NOOP: return "NOOP";
    case OPCODE::VAL: return "VAL";
    case OPCODE::VAR: return "VAR";
    case OPCODE::DICT: return "DICT";
    case OPCODE::PRG_STACK_PUSH: return "PRG_STACK_PUSH";
    case OPCODE::PRG_STACK_POP: return "PRG_STACK_POP";
    case OPCODE::PRG_STACK_AT: return "PRG_STACK_AT";
    case OPCODE::UNARY_PLUS: return "UNARY_PLUS";
    case OPCODE::UNARY_MINUS: return "UNARY_MINUS";
    case OPCODE::PLUS: return "PLUS";
    case OPCODE::MINUS: return "MINUS";
    case OPCODE::MUL: return "MUL";
    case OPCODE::DIV: return "DIV";
    case OPCODE::MOD: return "MOD";
    case OPCODE::CONCAT: return "CONCAT";
    case OPCODE::REPEAT: return "REPEAT";
    case OPCODE::BIT_AND: return "BIT_AND";
    case OPCODE::BIT_XOR: return "BIT_XOR";
    case OPCODE::BIT_OR: return "BIT_OR";
    case OPCODE::BIT_NOT: return "BIT_NOT";
    case OPCODE::NOT: return "NOT";
    case OPCODE::EQ: return "EQ";
    case OPCODE::NE: return "NE";
    case OPCODE::GE: return "GE";
    case OPCODE::GT: return "GT";
    case OPCODE::LE: return "LE";
    case OPCODE::LT: return "LT";
    case OPCODE::STR_EQ: return "STR_EQ";
    case OPCODE::STR_NE: return "STR_NE";
    case OPCODE::HALT: return "HALT";
    case OPCODE::DEBUG_FRAG: return "DEBUG_FRAG";
    case OPCODE::BYTECODE_FRAG: return "BYTECODE_FRAG";
    case OPCODE::PUSH_FRAG_FIRST: return "PUSH_FRAG_FIRST";
    case OPCODE::PUSH_FRAG_INNER: return "PUSH_FRAG_INNER";
    case OPCODE::PUSH_FRAG_LAST: return "PUSH_FRAG_LAST";
    case OPCODE::PUSH_FRAG_COUNT: return "PUSH_FRAG_COUNT";
    case OPCODE::PUSH_FRAG_INDEX: return "PUSH_FRAG_INDEX";
    case OPCODE::PUSH_VAL_FIRST: return "PUSH_VAL_FIRST";
    case OPCODE::PUSH_VAL_INNER: return "PUSH_VAL_INNER";
    case OPCODE::PUSH_VAL_LAST: return "PUSH_VAL_LAST";
    case OPCODE::PUSH_VAL_COUNT: return "PUSH_VAL_COUNT";
    case OPCODE::PUSH_VAL_INDEX: return "PUSH_VAL_INDEX";
    case OPCODE::PUSH_FRAG: return "PUSH_FRAG";
    case OPCODE::PRINT: return "PRINT";
    case OPCODE::AND: return "AND";
    case OPCODE::OR: return "OR";
    case OPCODE::FUNC: return "FUNC";
    case OPCODE::JMP_IF_NOT: return "JMP_IF_NOT";
    case OPCODE::JMP: return "JMP";
    case OPCODE::OPEN_FORMAT: return "OPEN_FORMAT";
    case OPCODE::CLOSE_FORMAT: return "CLOSE_FORMAT";
    case OPCODE::OPEN_FRAG: return "OPEN_FRAG";
    case OPCODE::OPEN_ERROR_FRAG: return "OPEN_ERROR_FRAG";
    case OPCODE::CLOSE_FRAG: return "CLOSE_FRAG";
    case OPCODE::SET: return "SET";
    case OPCODE::OPEN_CTYPE: return "OPEN_CTYPE";
    case OPCODE::CLOSE_CTYPE: return "CLOSE_CTYPE";
    case OPCODE::PUSH_ATTR: return "PUSH_ATTR";
    case OPCODE::PUSH_THIS_FRAG: return "PUSH_THIS_FRAG";
    case OPCODE::PUSH_ERROR_FRAG: return "PUSH_ERROR_FRAG";
    case OPCODE::PUSH_ROOT_FRAG: return "PUSH_ROOT_FRAG";
    case OPCODE::PUSH_ATTR_AT: return "PUSH_ATTR_AT";
    case OPCODE::POP_ATTR: return "POP_ATTR";
    case OPCODE::REPR: return "REPR";
    case OPCODE::QUERY_REPR: return "QUERY_REPR";
    case OPCODE::QUERY_COUNT: return "QUERY_COUNT";
    case OPCODE::QUERY_TYPE: return "QUERY_TYPE";
    case OPCODE::QUERY_DEFINED: return "QUERY_DEFINED";
    case OPCODE::QUERY_EXISTS: return "QUERY_EXISTS";
    case OPCODE::QUERY_ISEMPTY: return "QUERY_ISEMPTY";
    case OPCODE::OPEN_FRAME: return "OPEN_FRAME";
    case OPCODE::CLOSE_FRAME: return "CLOSE_FRAME";
    case OPCODE::REGEX_MATCH: return "REGEX_MATCH";
    case OPCODE::LOG_SUPPRESS: return "LOG_SUPPRESS";
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

void Instruction_t::dump(FILE *fp) const {
    FileStream_t stream(fp);
    dump(stream);
}

void Instruction_t::dump(std::ostream &os) const {
    os << pad(opcode_str(opcode_value));
    eval(opcode_value, *this, [&] (auto &self) {self.dump_params(os);});
}

template <typename ImplArg_t>
InstrBox_t::InstrBox_t(ImplArg_t &&other) noexcept
    : Instruction_t(nullptr)
{
    using Impl_t = std::decay_t<ImplArg_t>;
    static_assert(
        sizeof(Impl_t) <= sizeof(InstrBox_t),
        "The size of InstrBox_t padding has to be updated because used "
        "Instruction_t is bigger than current padding!"
    );
    static_assert(
        std::is_base_of<Instruction_t, Impl_t>::value,
        "The given instruction is not inherited from Instruction_t "
        "base class!"
    );
    static_assert(
        std::is_nothrow_move_constructible<Impl_t>::value,
        "The instruction implementation has to be nothrow move constructible!"
    );
    static_assert(
        std::is_nothrow_destructible<Impl_t>::value,
        "The instruction implementation has to be nothrow destructible!"
    );
    new (this) Impl_t(std::move(other));
}

InstrBox_t::InstrBox_t(InstrBox_t &&other) noexcept
    : Instruction_t(nullptr)
{
    eval(other.opcode_value, other, [&] (auto &&other_self) {
        new (this) InstrBox_t(std::move(other_self));
    });
}

InstrBox_t::~InstrBox_t() noexcept {
    eval(opcode_value, *this, [&] (auto &&self) {
        using Impl_t = std::decay_t<decltype(self)>;
        self.~Impl_t();
    });
}

void PushFragIndex_t::dump_params(std::ostream &os) const {
    os << "<frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushFragCount_t::dump_params(std::ostream &os) const {
    os << "<frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushFragFirst_t::dump_params(std::ostream &os) const {
    os << "<frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushFragInner_t::dump_params(std::ostream &os) const {
    os << "<frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushFragLast_t::dump_params(std::ostream &os) const {
    os << "<frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushValCount_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void PushValIndex_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void PushValFirst_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void PushValLast_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void PushValInner_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void PushFrag_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PushRootFrag_t::dump_params(std::ostream &os) const {
    os << "<root-frag-offset=" << root_frag_offset
       << '>';
}

void Val_t::dump_params(std::ostream &os) const {
    os << "<value=" << escapenl(value.printable())
       << ",type=" << value.type_str()
       << '>';
}

void Var_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",escape=" << escape
       << ",frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void PrgStackAt_t::dump_params(std::ostream &os) const {
    os << "<index=" << index << '>';
}

void And_t::dump_params(std::ostream &os) const {
    os << "<jump=" << std::showpos << addr_offset << '>' << std::noshowpos;
}

void Or_t::dump_params(std::ostream &os) const {
    os << "<jump=" << std::showpos << addr_offset << '>' << std::noshowpos;
}

void Func_t::dump_params(std::ostream &os) const {
    os << "<name=" << name << ",#args=" << nargs << '>';
}

void JmpIfNot_t::dump_params(std::ostream &os) const {
    os << "<jump=" << std::showpos << addr_offset << '>' << std::noshowpos;
}

void Jmp_t::dump_params(std::ostream &os) const {
    os << "<jump=" << std::showpos << addr_offset << '>' << std::noshowpos;
}

void OpenFormat_t::dump_params(std::ostream &os) const {
    os << "<mode=" << mode << '>';
}

void OpenFrag_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",close-frag-offset=" << std::showpos << close_frag_offset
       << '>' << std::noshowpos;
}

void CloseFrag_t::dump_params(std::ostream &os) const {
    os << "<open-frag-offset=" << std::showpos << open_frag_offset
       << '>' << std::noshowpos;
}

void Print_t::dump_params(std::ostream &os) const {
    os << "<print_escape=" << print_escape
       << '>';
}

void Set_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",frame-offset=" << frame_offset
       << ",frag-offset=" << frag_offset
       << '>';
}

void OpenCType_t::dump_params(std::ostream &os) const {
    os << "<mime-type=" << (ctype? ctype->name: "unknown/unknown") << '>';
}

void PushAttr_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",path=" << path
       << '>';
}

void PushAttrAt_t::dump_params(std::ostream &os) const {
    os << "<path=" << path
       << '>';
}

void RegexMatch_t::dump_params(std::ostream &os) const {
    os << "<regex=" << value << '>';
}

RegexMatch_t::RegexMatch_t(Regex_t regex, const Pos_t &pos)
    : Instruction_t(OPCODE::REGEX_MATCH, pos),
      value(std::move(regex)),
      compiled_value(new FixedPCRE_t(value.pattern, to_pcre_flags(value.flags)))
{}

RegexMatch_t::~RegexMatch_t() noexcept = default;

bool RegexMatch_t::matches(const string_view_t &view) const {
    auto tmp = compiled_value->regex;
    tmp.search(view.str());
    return tmp.matched();
}

void PushErrorFrag_t::dump_params(std::ostream &os) const {
    os << "<discard_stack_value=" << discard_stack_value << '>';
}

} // namespace Teng

