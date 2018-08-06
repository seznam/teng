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

#include "tengfilestream.h"
#include "tenginstruction.h"
#include "tengparservalue.h"
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
    return name.append(32 - name.size(), ' ');
}

/** Returns name of content type.
 */
std::string ct_name(int64_t id) {
    if (auto *ct = ContentType_t::getContentType(id))
        return ct->name;
    return ContentType_t::getDefault()->name;
}

} // namespace

void Instruction_t::dump(FILE *fp) const {
    FileStream_t stream(fp);
    dump(stream);
}

void Instruction_t::dump(std::ostream &os) const {
    switch (opcode) {
    case VAL:
        os << pad("VAL") << ' '
           << "<value=" << escapenl(value.str())
           << '>';
        break;

    case VAR:
        os << pad("VAR")
           << "<ident=" << identifier.name
           << ",escape=" << value.integral()
           << ",context=" << identifier.context
           << ",depth=" << identifier.depth
           << '>';
        break;

    case DICT:
        os << pad("DICT");
        break;

    case PUSH:
        os << pad("PUSH");
        break;

    case POP:
        os << pad("POP");
        break;

    case STACK:
        os << pad("STACK") << ' '
           << "<index=" << value.integral()
           << '>';
        break;

    case ADD:
        os << pad("ADD");
        break;

    case SUB:
        os << pad("SUB");
        break;

    case MUL:
        os << pad("MUL");
        break;

    case DIV:
        os << pad("DIV");
        break;

    case MOD:
        os << pad("MOD");
        break;

    case CONCAT:
        os << pad("CONCAT");
        break;

    case REPEAT:
        os << pad("REPEAT");
        break;

    case BITAND:
        os << pad("BITAND");
        break;

    case BITXOR:
        os << pad("BITXOR");
        break;

    case BITOR:
        os << pad("BITOR");
        break;

    case BITNOT:
        os << pad("BITNOT");
        break;

    case NOT:
        os << pad("NOT");
        break;

    case NUMEQ:
        os << pad("NUMEQ");
        break;

    case NUMGE:
        os << pad("NUMGE");
        break;

    case NUMGT:
        os << pad("NUMGT");
        break;

    case STREQ:
        os << pad("STREQ");
        break;

    case HALT:
        os << pad("HALT");
        break;

    case DEBUG_FRAG:
        os << pad("DEBUG_FRAG");
        break;

    case BYTECODE_FRAG:
        os << pad("BYTECODE_FRAG");
        break;

    case SUPRESS_LOG:
        os << pad("SUPRESS_LOG");
        break;

    case FRAGFIRST:
        os << pad("FRAGFIRST");
        break;

    case FRAGINNER:
        os << pad("FRAGINNER");
        break;

    case FRAGLAST:
        os << pad("FRAGLAST");
        break;

    case DEFINED:
        os << pad("DEFINED");
        break;

    case EXISTS:
        os << pad("EXISTS")
           << "<ident=" << value.str()
           << '>';
        break;

    case ISEMPTY:
        os << pad("ISEMPTY");
        break;

    case PRINT:
        os << pad("PRINT");
        break;

    case AND:
        os << pad("AND")
           << "<jump=" << std::showpos << value.integral()
           << '>';
        break;

    case OR:
        os << pad("OR")
           << "<jump=" << std::showpos << value.integral()
           << '>';
        break;

    case FUNC:
        os << pad("FUNC")
           << "<name=" << value.as_str()
           << ",#args=" << opt_value.integral()
           << '>';
        break;

    case JMPIFNOT:
        os << pad("JMPIFNOT")
           << "<jump=" << std::showpos << value.integral()
           << '>';
        break;

    case JMP:
        os << pad("JMP")
           << "<jump=" << std::showpos << value.integral()
           << '>';
        break;

    case FORM:
        os << pad("FORM")
           << "<format-id=" << value.integral()
           << '>';
        break;

    case ENDFORM:
        os << pad("ENDFORM");
        break;

    case FRAG:
        os << pad("FRAG")
           << "<name=" << identifier.name
           << ",jump=" << std::showpos << value.integral()
           << '>';
        break;

    case ENDFRAG:
        os << pad("ENDFRAG")
           << "<jump=" << std::showpos << value.integral()
           << '>';
        break;

    case FRAGCNT:
        os << pad("FRAGCNT")
           << "<name=" << identifier.name
           << ",context=" << identifier.context
           << ",depth=" << identifier.depth
           << '>';
        break;

    case NESTED_FRAGCNT:
        os << pad("NESTED_XFRAGCNT")
           << "<name=" << identifier.name
           << ",context=" << identifier.context
           << ",depth=" << identifier.depth
           << '>';
        break;

    case FRAGINDEX:
        os << pad("FRAGINDEX")
           << "<index=" << identifier.name
           << ",context=" << identifier.context
           << ",depth=" << identifier.depth
           << '>';
        break;

    case SET:
        os << pad("SET")
           << "<name=" << value.as_str()
           << '>';
        break;

    case CTYPE:
        os << pad("ctype")
           << "<mime-type=" << ct_name(value.integral())
           << '>';
        break;

    case ENDCTYPE:
        os << pad("ENDCTYPE");
        break;

    case PUSH_ATTR:
        os << pad("PUSH_ATTR")
           << "<name=" << value.str()
           << '>';
        break;

    case PUSH_THIS_FRAG:
        os << pad("PUSH_THIS_FRAG");
        break;

    case PUSH_ROOT_FRAG:
        os << pad("PUSH_ROOT_FRAG");
        break;

    case PUSH_ATTR_AT:
        os << pad("PUSH_ATTR_AT")
           << "<at=" << value.str()
           << '>';
        break;

    case REPR:
        os << pad("REPR")
           << "<ident=" << value.as_str()
           << '>';
        break;

    case REPR_JSONIFY:
        os << pad("REPR_JSONIFY")
           << "<ident=" << value.str()
           << '>';
        break;

    case REPR_COUNT:
        os << pad("REPR_COUNT")
           << "<ident=" << value.str()
           << '>';
        break;

    case REPR_TYPE:
        os << pad("REPR_TYPE")
           << "<ident=" << value.str()
           << '>';
        break;

    case REPR_DEFINED:
        os << pad("REPR_DEFINED")
           << "<ident=" << value.str()
           << '>';
        break;

    case REPR_EXISTS:
        os << pad("REPR_EXISTS")
           << "<ident=" << value.str()
           << '>';
        break;

    case REPR_ISEMPTY:
        os << pad("REPR_ISEMPTY")
           << "<ident=" << value.str()
           << '>';
        break;
    }
}

} // namespace Teng

