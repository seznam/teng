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
 * $Id: tenginstruction.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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

#include <stdio.h>
#include <string>
#include <vector>

#include "tenginstruction.h"
#include "tengparservalue.h"

using namespace std;

using namespace Teng;

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
            fprintf(fp, "VAR\t%s\t%ld\n",
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
            fprintf(fp, "STACK\t%ld\n",
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
            fprintf(fp, "AND\t%ld\n",
                    value.integerValue); //relative jump added to ip
            break;
            
        case OR:
            fprintf(fp, "OR\t%ld\n",
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
            fprintf(fp, "FUNC\t%s\t%ld\n",
                    value.stringValue.c_str(), //function name
                    value.integerValue); //number of params on stack
            break;
            
        case JMPIFNOT:
            fprintf(fp, "JMPIFNOT\t%ld\n",
                    value.integerValue); //relative jump added to ip
            break;
            
        case JMP:
            fprintf(fp, "JMP\t%ld\n",
                    value.integerValue); //relative jump added to ip
            break;
            
        case FORM:
            fprintf(fp, "FORM\t%ld\n",
                    value.integerValue); //print formatting mode
            break;
            
        case ENDFORM:
            fprintf(fp, "ENDFORM\n");
            break;
            
        case FRAG:
            fprintf(fp, "FRAG\t'%s'\t%ld\n",
                    value.stringValue.c_str(), //fragment name
                    value.integerValue); //jump right after the end of frag
            break;
            
        case ENDFRAG:
            fprintf(fp, "ENDFRAG\t%ld\n",
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
            
        case SET:
            fprintf(fp, "SET\t%s\n",
                    value.stringValue.c_str()); //variable name
            break;
            
        case HALT:
            fprintf(fp, "HALT\n");
            break;
            
        case DEBUG:
            fprintf(fp, "DEBUG\n");
            break;

        case EXIST:
            fprintf(fp, "EXIST\t%s\n",
                    value.stringValue.c_str()); //rendered 'identifier' vector
            break;
            
        default:
            fprintf(fp, "??? (%d)\n", operation);
    }
}
