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
#include <pcre++.h>

#include "tengvalue.h"
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
        return call(self.template as<Noop_t>(), std::forward<args_t>(args)...);
    case OPCODE::VAL:
        return call(self.template as<Val_t>(), std::forward<args_t>(args)...);
    case OPCODE::VAR:
        return call(self.template as<Var_t>(), std::forward<args_t>(args)...);
    case OPCODE::DICT:
        return call(self.template as<Dict_t>(), std::forward<args_t>(args)...);
    case OPCODE::PUSH:
        return call(self.template as<Push_t>(), std::forward<args_t>(args)...);
    case OPCODE::POP:
        return call(self.template as<Pop_t>(), std::forward<args_t>(args)...);
    case OPCODE::STACK:
        return call(self.template as<Stack_t>(), std::forward<args_t>(args)...);
    case OPCODE::ADD:
        return call(self.template as<Add_t>(), std::forward<args_t>(args)...);
    case OPCODE::SUB:
        return call(self.template as<Sub_t>(), std::forward<args_t>(args)...);
    case OPCODE::MUL:
        return call(self.template as<Mul_t>(), std::forward<args_t>(args)...);
    case OPCODE::DIV:
        return call(self.template as<Div_t>(), std::forward<args_t>(args)...);
    case OPCODE::MOD:
        return call(self.template as<Mod_t>(), std::forward<args_t>(args)...);
    case OPCODE::CONCAT:
        return call(self.template as<Concat_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPEAT:
        return call(self.template as<Repeat_t>(), std::forward<args_t>(args)...);
    case OPCODE::BITAND:
        return call(self.template as<BitAnd_t>(), std::forward<args_t>(args)...);
    case OPCODE::BITXOR:
        return call(self.template as<BitXor_t>(), std::forward<args_t>(args)...);
    case OPCODE::BITOR:
        return call(self.template as<BitOr_t>(), std::forward<args_t>(args)...);
    case OPCODE::BITNOT:
        return call(self.template as<BitNot_t>(), std::forward<args_t>(args)...);
    case OPCODE::AND:
        return call(self.template as<And_t>(), std::forward<args_t>(args)...);
    case OPCODE::OR:
        return call(self.template as<Or_t>(), std::forward<args_t>(args)...);
    case OPCODE::NOT:
        return call(self.template as<Not_t>(), std::forward<args_t>(args)...);
    case OPCODE::EQ:
        return call(self.template as<EQ_t>(), std::forward<args_t>(args)...);
    case OPCODE::NE:
        return call(self.template as<NE_t>(), std::forward<args_t>(args)...);
    case OPCODE::GE:
        return call(self.template as<GE_t>(), std::forward<args_t>(args)...);
    case OPCODE::GT:
        return call(self.template as<GT_t>(), std::forward<args_t>(args)...);
    case OPCODE::LE:
        return call(self.template as<LE_t>(), std::forward<args_t>(args)...);
    case OPCODE::LT:
        return call(self.template as<LT_t>(), std::forward<args_t>(args)...);
    case OPCODE::STREQ:
        return call(self.template as<StrEQ_t>(), std::forward<args_t>(args)...);
    case OPCODE::STRNE:
        return call(self.template as<StrNE_t>(), std::forward<args_t>(args)...);
    case OPCODE::FUNC:
        return call(self.template as<Func_t>(), std::forward<args_t>(args)...);
    case OPCODE::JMPIFNOT:
        return call(self.template as<JmpIfNot_t>(), std::forward<args_t>(args)...);
    case OPCODE::JMP:
        return call(self.template as<Jmp_t>(), std::forward<args_t>(args)...);
    case OPCODE::FORMAT:
        return call(self.template as<Format_t>(), std::forward<args_t>(args)...);
    case OPCODE::ENDFORMAT:
        return call(self.template as<EndFormat_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAG:
        return call(self.template as<Frag_t>(), std::forward<args_t>(args)...);
    case OPCODE::ENDFRAG:
        return call(self.template as<EndFrag_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAGCNT:
        return call(self.template as<FragCnt_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAGINDEX:
        return call(self.template as<FragIndex_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAGFIRST:
        return call(self.template as<FragFirst_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAGLAST:
        return call(self.template as<FragLast_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAGINNER:
        return call(self.template as<FragInner_t>(), std::forward<args_t>(args)...);
    case OPCODE::PRINT:
        return call(self.template as<Print_t>(), std::forward<args_t>(args)...);
    case OPCODE::SET:
        return call(self.template as<Set_t>(), std::forward<args_t>(args)...);
    case OPCODE::HALT:
        return call(self.template as<Halt_t>(), std::forward<args_t>(args)...);
    case OPCODE::DEBUG_FRAG:
        return call(self.template as<DebugFrag_t>(), std::forward<args_t>(args)...);
    case OPCODE::BYTECODE_FRAG:
        return call(self.template as<BytecodeFrag_t>(), std::forward<args_t>(args)...);
    case OPCODE::CTYPE:
        return call(self.template as<CType_t>(), std::forward<args_t>(args)...);
    case OPCODE::ENDCTYPE:
        return call(self.template as<EndCType_t>(), std::forward<args_t>(args)...);
    case OPCODE::PUSH_ATTR:
        return call(self.template as<PushAttr_t>(), std::forward<args_t>(args)...);
    case OPCODE::PUSH_ROOT_FRAG:
        return call(self.template as<PushRootFrag_t>(), std::forward<args_t>(args)...);
    case OPCODE::PUSH_THIS_FRAG:
        return call(self.template as<PushThisFrag_t>(), std::forward<args_t>(args)...);
    case OPCODE::PUSH_ATTR_AT:
        return call(self.template as<PushAttrAt_t>(), std::forward<args_t>(args)...);
    case OPCODE::POP_ATTR:
        return call(self.template as<PopAttr_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR:
        return call(self.template as<Repr_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_JSONIFY:
        return call(self.template as<ReprJsonify_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_COUNT:
        return call(self.template as<ReprCount_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_TYPE:
        return call(self.template as<ReprType_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_DEFINED:
        return call(self.template as<ReprDefined_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_EXISTS:
        return call(self.template as<ReprExists_t>(), std::forward<args_t>(args)...);
    case OPCODE::REPR_ISEMPTY:
        return call(self.template as<ReprIsEmpty_t>(), std::forward<args_t>(args)...);
    case OPCODE::FRAME:
        return call(self.template as<Frame_t>(), std::forward<args_t>(args)...);
    case OPCODE::ENDFRAME:
        return call(self.template as<EndFrame_t>(), std::forward<args_t>(args)...);
    case OPCODE::REGEX_MATCH:
        return call(self.template as<RegexMatch_t>(), std::forward<args_t>(args)...);
    }
}

const char *opcode_str(OPCODE opcode) {
    switch (opcode) {
    case OPCODE::NOOP: return "NOOP";
    case OPCODE::VAL: return "VAL";
    case OPCODE::VAR: return "VAR";
    case OPCODE::DICT: return "DICT";
    case OPCODE::PUSH: return "PUSH";
    case OPCODE::POP: return "POP";
    case OPCODE::STACK: return "STACK";
    case OPCODE::ADD: return "ADD";
    case OPCODE::SUB: return "SUB";
    case OPCODE::MUL: return "MUL";
    case OPCODE::DIV: return "DIV";
    case OPCODE::MOD: return "MOD";
    case OPCODE::CONCAT: return "CONCAT";
    case OPCODE::REPEAT: return "REPEAT";
    case OPCODE::BITAND: return "BITAND";
    case OPCODE::BITXOR: return "BITXOR";
    case OPCODE::BITOR: return "BITOR";
    case OPCODE::BITNOT: return "BITNOT";
    case OPCODE::NOT: return "NOT";
    case OPCODE::EQ: return "EQ";
    case OPCODE::NE: return "NE";
    case OPCODE::GE: return "GE";
    case OPCODE::GT: return "GT";
    case OPCODE::LE: return "LE";
    case OPCODE::LT: return "LT";
    case OPCODE::STREQ: return "STREQ";
    case OPCODE::STRNE: return "STRNE";
    case OPCODE::HALT: return "HALT";
    case OPCODE::DEBUG_FRAG: return "DEBUG_FRAG";
    case OPCODE::BYTECODE_FRAG: return "BYTECODE_FRAG";
    case OPCODE::FRAGFIRST: return "FRAGFIRST";
    case OPCODE::FRAGINNER: return "FRAGINNER";
    case OPCODE::FRAGLAST: return "FRAGLAST";
    case OPCODE::PRINT: return "PRINT";
    case OPCODE::AND: return "AND";
    case OPCODE::OR: return "OR";
    case OPCODE::FUNC: return "FUNC";
    case OPCODE::JMPIFNOT: return "JMPIFNOT";
    case OPCODE::JMP: return "JMP";
    case OPCODE::FORMAT: return "FORMAT";
    case OPCODE::ENDFORMAT: return "ENDFORMAT";
    case OPCODE::FRAG: return "FRAG";
    case OPCODE::ENDFRAG: return "ENDFRAG";
    case OPCODE::FRAGCNT: return "FRAGCNT";
    case OPCODE::FRAGINDEX: return "FRAGINDEX";
    case OPCODE::SET: return "SET";
    case OPCODE::CTYPE: return "CTYPE";
    case OPCODE::ENDCTYPE: return "ENDCTYPE";
    case OPCODE::PUSH_ATTR: return "PUSH_ATTR";
    case OPCODE::PUSH_THIS_FRAG: return "PUSH_THIS_FRAG";
    case OPCODE::PUSH_ROOT_FRAG: return "PUSH_ROOT_FRAG";
    case OPCODE::PUSH_ATTR_AT: return "PUSH_ATTR_AT";
    case OPCODE::POP_ATTR: return "POP_ATTR";
    case OPCODE::REPR: return "REPR";
    case OPCODE::REPR_JSONIFY: return "REPR_JSONIFY";
    case OPCODE::REPR_COUNT: return "REPR_COUNT";
    case OPCODE::REPR_TYPE: return "REPR_TYPE";
    case OPCODE::REPR_DEFINED: return "REPR_DEFINED";
    case OPCODE::REPR_EXISTS: return "REPR_EXISTS";
    case OPCODE::REPR_ISEMPTY: return "REPR_ISEMPTY";
    case OPCODE::FRAME: return "FRAME";
    case OPCODE::ENDFRAME: return "ENDFRAME";
    case OPCODE::REGEX_MATCH: return "REGEX_MATCH";
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
        "The size of InstrBox_t padding has to be updated becouse used "
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
    // TODO(burlog): 
    std::cerr << "@@@@ " << this
              << "->" << std::string(typeid(Impl_t).name()).substr(7)
              << "[" << sizeof(Impl_t) << "<"
              << sizeof(InstrBox_t) << "]" << std::endl;
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

void FragIndex_t::dump_params(std::ostream &os) const {
    os << "<frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void FragCnt_t::dump_params(std::ostream &os) const {
    os << "<frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void FragFirst_t::dump_params(std::ostream &os) const {
    os << "<frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void FragInner_t::dump_params(std::ostream &os) const {
    os << "<frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void FragLast_t::dump_params(std::ostream &os) const {
    os << "<frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
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
       << ",frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void Stack_t::dump_params(std::ostream &os) const {
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

void Format_t::dump_params(std::ostream &os) const {
    os << "<mode=" << mode << '>';
}

void Frag_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",endfrag-offset=" << std::showpos << endfrag_offset
       << '>' << std::noshowpos;
}

void EndFrag_t::dump_params(std::ostream &os) const {
    os << "<openfrag-offset=" << std::showpos << openfrag_offset
       << '>' << std::noshowpos;
}

void Set_t::dump_params(std::ostream &os) const {
    os << "<name=" << name
       << ",frame_offset=" << frame_offset
       << ",frag_offset=" << frag_offset
       << '>';
}

void CType_t::dump_params(std::ostream &os) const {
    os << "<mime-type=" << (ctype? ctype->name: "unknown/unknown") << '>';
}

void PushAttr_t::dump_params(std::ostream &os) const {
    os << "<name=" << name << '>';
}

void RegexMatch_t::dump_params(std::ostream &os) const {
    os << "<name=" << value << '>';
}

RegexMatch_t::RegexMatch_t(Regex_t regex, const Pos_t &pos)
    : Instruction_t(OPCODE::REGEX_MATCH, pos),
      value(std::move(regex)),
      compiled_value(new pcrepp::Pcre(value.pattern, to_pcre_flags(value)))
{}

RegexMatch_t::~RegexMatch_t() noexcept = default;

bool RegexMatch_t::matches(const string_view_t &view) const {
    auto tmp = *compiled_value;
    tmp.search(view.str());
    return tmp.matched();
}

uint32_t RegexMatch_t::to_pcre_flags(const Regex_t &regex) {
    uint32_t result = 0;
    if (regex.flags.ignore_case)
        result |= PCRE_GLOBAL;
    if (regex.flags.global)
        result |= PCRE_CASELESS;
    if (regex.flags.multiline)
        result |= PCRE_MULTILINE;
    return result;
}

} // namespace Teng

