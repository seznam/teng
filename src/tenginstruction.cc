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
#include <iostream>
#include <iomanip>

#include "tenginstruction.h"
#include "tengparservalue.h"
#include "tengcontenttype.h"

namespace Teng {

/** Print instruction into file stream.
  * @param fp File stream for output. */
void Instruction_t::dump(FILE *fp) const
{
    switch (operation) {

        case VAL:
            fprintf(fp, "VAL\t'%s'\n",
                    value.stringValue.c_str()); //string version of the value
            break;

        case VAR:
            fprintf(fp, "VAR\t%s\t%zd\n",
                    value.stringValue.c_str(), //rendered 'identifier' vector
                    value.integerValue); //escape flag
            break;

        case DICT:
            fprintf(fp, "DICT\n");
            break;

        case PUSH:
            fprintf(fp, "PUSH\n");
            break;

        case POP:
            fprintf(fp, "POP\n");
            break;

        case STACK:
            fprintf(fp, "STACK\t%zd\n",
                    value.integerValue); //value offset from stack top
            break;

        case ADD:
            fprintf(fp, "ADD\n");
            break;

        case SUB:
            fprintf(fp, "SUB\n");
            break;

        case MUL:
            fprintf(fp, "MUL\n");
            break;

        case DIV:
            fprintf(fp, "DIV\n");
            break;

        case MOD:
            fprintf(fp, "MOD\n");
            break;

        case CONCAT:
            fprintf(fp, "CONCAT\n");
            break;

        case REPEAT:
            fprintf(fp, "REPEAT\n");
            break;

        case AND:
            fprintf(fp, "AND\t%zd\n",
                    value.integerValue); //relative jump added to ip
            break;

        case OR:
            fprintf(fp, "OR\t%zd\n",
                    value.integerValue); //relative jump added to ip
            break;

        case BITAND:
            fprintf(fp, "BITAND\n");
            break;

        case BITXOR:
            fprintf(fp, "BITXOR\n");
            break;

        case BITOR:
            fprintf(fp, "BITOR\n");
            break;

        case BITNOT:
            fprintf(fp, "BITNOT\n");
            break;

        case NOT:
            fprintf(fp, "NOT\n");
            break;

        case NUMEQ:
            fprintf(fp, "NUMEQ\n");
            break;

        case NUMGE:
            fprintf(fp, "NUMGE\n");
            break;

        case NUMGT:
            fprintf(fp, "NUMGT\n");
            break;

        case STREQ:
            fprintf(fp, "STREQ\n");
            break;

        case FUNC:
            fprintf(fp, "FUNC\t%s\t%zd\n",
                    value.stringValue.c_str(), //function name
                    value.integerValue); //number of params on stack
            break;

        case JMPIFNOT:
            fprintf(fp, "JMPIFNOT\t%zd\n",
                    value.integerValue); //relative jump added to ip
            break;

        case JMP:
            fprintf(fp, "JMP\t%zd\n",
                    value.integerValue); //relative jump added to ip
            break;

        case FORM:
            fprintf(fp, "FORM\t%zd\n",
                    value.integerValue); //print formatting mode
            break;

        case ENDFORM:
            fprintf(fp, "ENDFORM\n");
            break;

        case FRAG:
            fprintf(fp, "FRAG\t'%s'\t%zd\n",
                    value.stringValue.c_str(), //fragment name
                    value.integerValue); //jump right after the end of frag
            break;

        case ENDFRAG:
            fprintf(fp, "ENDFRAG\t%zd\n",
                    value.integerValue); //jump right after the fragment start
            break;

        case FRAGCNT:
            fprintf(fp, "FRAGCNT\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case XFRAGCNT:
            fprintf(fp, "XFRAGCNT\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case FRAGITR:
            fprintf(fp, "FRAGITR\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case PRINT:
            fprintf(fp, "PRINT\n");
            break;

        case FRAGFIRST:
            fprintf(fp, "FRAGFIRST\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case FRAGLAST:
            fprintf(fp, "FRAGLAST\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case FRAGINNER:
            fprintf(fp, "FRAGINNER\t'%s'\n",
                    value.stringValue.c_str()); //fragment name
            break;

        case SET:
            fprintf(fp, "SET\t%s\n",
                    value.stringValue.c_str()); //variable name
            break;

        case HALT:
            fprintf(fp, "HALT\n");
            break;

        case DEBUGING:
            fprintf(fp, "DEBUG\n");
            break;

        case DEFINED:
            fprintf(fp, "DEFINED\t%s\n", value.stringValue.c_str());
            break;

        case ISEMPTY:
            fprintf(fp, "ISEMPTY\t%s\n", value.stringValue.c_str());
            break;

        case EXISTS:
            fprintf(fp, "EXISTS\t%s\n",
                    value.stringValue.c_str()); //rendered 'identifier' vector
            break;

        case GETATTR:
            fprintf(fp, "GETATTR\t%s\n", value.stringValue.c_str());
            break;

        case AT:
            fprintf(fp, "AT\n");
            break;

        case REPR:
            fprintf(fp, "REPR\t%s\n",  value.stringValue.c_str());
            break;

        case EXISTMARK:
            fprintf(fp, "EXISTMARK\n");
            break;

        default:
            fprintf(fp, "??? (%d)\n", operation);
    }
}

namespace {
    class hexaddr {
    public:
        hexaddr(int addr, int ip)
            : addr(addr), ip(ip)
        {}

        friend std::ostream& operator<<(std::ostream &os, const hexaddr &ha) {
            os << std::dec << std::setiosflags(std::ios::showpos) << ha.addr;
            if (ha.ip >= 0)
                os << " [abs 0x" << std::hex << std::setw(8)
                   << std::setfill('0') << (ha.addr + 1 + ha.ip) << ']';

            return os;
        }

    private:
        int addr;
        int ip;
    };
}

void Instruction_t::dump(std::ostream &os, int ip) const {
    switch (operation) {
    case VAL:
        os << "VAL             '" << value.stringValue << '\'' << std::endl;
        break;

    case VAR:
        os << "VAR             <" << value.stringValue << "> ("
           << identifier.context << ":" << identifier.depth << ")";
        if (value.integerValue) os << " [escaped]";
        os << std::endl;
        break;

    case DICT:
        os << "DICT" << std::endl;
        break;

    case PUSH:
        os << "PUSH" << std::endl;
        break;

    case POP:
        os << "POP" << std::endl;
        break;

    case STACK:
        os << "STACK           " << value.integerValue << std::endl;
        break;

    case ADD:
        os << "ADD" << std::endl;
        break;

    case SUB:
        os << "SUB" << std::endl;
        break;

    case MUL:
        os << "MUL" << std::endl;
        break;

    case DIV:
        os << "DIV" << std::endl;
        break;

    case MOD:
        os << "MOD" << std::endl;
        break;

    case CONCAT:
        os << "CONCAT" << std::endl;
        break;

    case REPEAT:
        os << "REPEAT" << std::endl;
        break;

    case AND:
        os << "AND             " << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case OR:
        os << "OR              " << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case BITAND:
        os << "BITAND" << std::endl;
        break;

    case BITXOR:
        os << "BITXOR" << std::endl;
        break;

    case BITOR:
        os << "BITOR" << std::endl;
        break;

    case BITNOT:
        os << "BITNOT" << std::endl;
        break;

    case NOT:
        os << "NOT" << std::endl;
        break;

    case NUMEQ:
        os << "NUMEQ" << std::endl;
        break;

    case NUMGE:
        os << "NUMGE" << std::endl;
        break;

    case NUMGT:
        os << "NUMGT" << std::endl;
        break;

    case STREQ:
        os << "STREQ" << std::endl;
        break;

    case FUNC:
        os << "FUNC            " << value.stringValue << "() "
           << value.integerValue << std::endl;
        break;

    case JMPIFNOT:
        os << "JMPIFNOT        '" << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case JMP:
        os << "JMP             '" << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case FORM:
        os << "FORM            '" << value.integerValue << std::endl;
        break;

    case ENDFORM:
        os << "ENDFORM" << std::endl;
        break;

    case FRAG:
        os << "FRAG            <" << value.stringValue << "> "
           << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case ENDFRAG:
        os << "ENDFRAG         " << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case FRAGCNT:
        os << "FRAGCNT         <" << value.stringValue << "> ("
           << identifier.context << ":" << identifier.depth << ")"
           << std::endl;
        break;

    case XFRAGCNT:
        os << "XFRAGCNT        <" << value.stringValue << "> ("
           << identifier.context << ":" << identifier.depth << ")"
           << std::endl;
        break;

    case FRAGITR:
        os << "FRAGITR         <" << value.stringValue << '>' << std::endl;
        break;

    case PRINT:
        os << "PRINT" << std::endl;
        break;

    case SET:
        os << "SET             <" << value.stringValue << '>' << std::endl;
        break;

    case HALT:
        os << "HALT" << std::endl;
        break;

    case DEBUGING:
        os << "DEBUG" << std::endl;
        break;

    case BYTECODE:
        os << "BYTECODE" << std::endl;
        break;

    case EXISTS:
        os << "EXISTS           <" << value.stringValue << '>' << std::endl;
        break;

    case CTYPE:
        if (const ContentType_t::Descriptor_t *ct
            = ContentType_t::getContentType(value.integerValue)) {
            os << "CTYPE           <" << ct->name << '>' << std::endl;
        } else {
            os << "CTYPE           <unknown>" << std::endl;
        }
        break;

    case ENDCTYPE:
        os << "ENDCTYPE" << std::endl;
        break;

    case REPEATFRAG:
        os << "REPEATFRAG      <" << value.stringValue << "> "
           << hexaddr(value.integerValue, ip) << std::endl;
        break;

    case GETATTR:
        os << "GETATTR         <" << value.stringValue << '>' << std::endl;
        break;

    case AT:
        os << "AT" << std::endl;
        break;

    case REPR:
        os << "REPR         " << value.stringValue << std::endl;
        break;

    case EXISTMARK:
        os << "EXISTMARK" << std::endl;
        break;

    default:
        os << "<ILLEGAL>       opcode == " << operation << std::endl;
        break;
    }
}

} // namespace Teng

