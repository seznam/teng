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
 * Teng processor. Executes programs.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (jan)
 *             Created.
 * 2004-05-30  (vasek)
 *             Revised processor source code.
 * 2004-09-19  (vasek)
 *             Some minor fixes.
 */

#include <stack>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "tengerror.h"
#include "tengfunction.h"
#include "tengprocessor.h"
#include "tengfragmentstack.h"
#include "tengutil.h"
#include "tengudf.h"

#include "tengaux.h"

#ifdef HAVE_FENV_H
#include <fenv.h>
#endif


using namespace std;

using namespace Teng;

void Processor_t::Logger_t::logError(Error_t::Level_t level,
                                     const string &message)
{
    if (instr) processor.logErr(*instr, message, level);
}

namespace {
    struct ToLower_t {
        int operator () (int c) const {
            return tolower(c);
        }
    };
}

// wraps teng fragments, lists and values
class FragVal_t {
    public:
        enum FragValueType {
            FRAGMENT_NULL,
            FRAGMENT,
            FRAGMENT_LIST,
            FRAGMENT_VALUE,
        };

        FragValueType type;
        union {
            const Fragment_t *frag;
            const FragmentList_t *list;
            const FragmentValue_t *value;
        };

        FragVal_t()
        : type(FRAGMENT_NULL) {}

        FragVal_t(const Fragment_t *frag)
        : type(FRAGMENT), frag(frag) {}

        FragVal_t(const FragmentList_t *list)
        : type(FRAGMENT_LIST), list(list) {}

        FragVal_t(const FragmentValue_t *value)
        : type(FRAGMENT_VALUE), value(value) {}
};

// performs UDF calls
int callUdf(const vector<ParserValue_t> &args, ParserValue_t &result, UDFCallback_t &udf, string &err) {
    vector<UDFValue_t> udfArgs;
    UDFValue_t udfRes((IntType_t)0);
    udfArgs.reserve(args.size());
    int res = E_OK;

    for (vector<ParserValue_t>::const_reverse_iterator it = args.rbegin();
            it != args.rend(); it++ ) {
        switch (it->type) {
            case ParserValue_t::TYPE_INT:
                udfArgs.push_back(UDFValue_t(it->integerValue));
                break;
            case ParserValue_t::TYPE_REAL:
                udfArgs.push_back(UDFValue_t(it->realValue));
                break;
            case ParserValue_t::TYPE_STRING:
                udfArgs.push_back(UDFValue_t(it->stringValue));
                break;
        }
    }

    try {
        udfRes = udf(udfArgs);
    } catch (invalid_argument & e) {
        err = e.what();
        res = E_ARGS;
    } catch (exception & e) {
        res = E_OTHER;
        err = e.what();
    }

    if ( res == 0 ) {
        switch (udfRes.getType()) {
            case UDFValue_t::Integer:
                result.setInteger(udfRes.getInt());
                break;
            case UDFValue_t::Real:
                result.setReal(udfRes.getReal());
                break;
            case UDFValue_t::String:
                result.setString(udfRes.getString());
                break;
        }
    } else {
        result.setString("undefinded");
    }
    return res;
}

Processor_t::
FunctionParam_t::FunctionParam_t(Processor_t &processor,
                                 const string &encoding,
                                 const ContentType_t *contentType,
                                 const Configuration_t &configuration,
                                 const Dictionary_t &langDictionary)
    : encoding(encoding), escaper(contentType), logger(processor),
      configuration(configuration), langDictionary(langDictionary)
{
    transform(this->encoding.begin(), this->encoding.end(),
              this->encoding.begin(), ToLower_t());
}

namespace {
    inline Error_t::Position_t position(const Instruction_t &instr,
                                        const Program_t &program)
    {
        return Error_t::Position_t
            (((instr.sourceIndex < 0) ? "" :
              program.getSource(instr.sourceIndex)),
             instr.line,
             instr.column);
    }
}

void Processor_t::logErr(const Instruction_t &instr, const string &s,
                         Error_t::Level_t level)
{
    error->logRuntimeError(level, position(instr, program), s);
}

void Processor_t::logErrNoInstr(const string &s,
                                Error_t::Level_t level)
{
    error->logRuntimeError(level,
                           Error_t::Position_t("unknown", 1, 0), s);
}

Processor_t::Processor_t(const Program_t &program,
                         const Dictionary_t &dict,
                         const Configuration_t &configuration,
                         const string &encoding,
                         const ContentType_t *contentType)
    : program(program), langDictionary(dict), configuration(configuration),
      fParam(*this, encoding, contentType, configuration, dict)
{
    srand(time(0) ^ getpid()); // because of user function random
}

int Processor_t::evalNumOp(const Instruction_t &instr) {
    if (valueStack.size() < 2) return -1;

    ParserValue_t b = valueStack.top();
    valueStack.pop();
    b.validateThis();

    ParserValue_t a = valueStack.top();
    valueStack.pop();
    a.validateThis();

    if ((a.type == ParserValue_t::TYPE_STRING) ||
        (b.type == ParserValue_t::TYPE_STRING)) {
        return -1;
    }

    if ((a.type == ParserValue_t::TYPE_REAL) ||
        (b.type == ParserValue_t::TYPE_REAL)) {
#ifdef HAVE_FENV_H
        feclearexcept(FE_ALL_EXCEPT);
#endif

        switch (instr.operation) {
        case Instruction_t::BITAND:
        case Instruction_t::BITOR:
        case Instruction_t::BITXOR:
            return -1;

        case Instruction_t::ADD:
            a.setReal(a.realValue + b.realValue);
            break;

        case Instruction_t::SUB:
            a.setReal(a.realValue - b.realValue);
            break;

        case Instruction_t::MUL:
            a.setReal(a.realValue * b.realValue);
            break;

        case Instruction_t::DIV:
            a.setReal(a.realValue / b.realValue);
            break;

        case Instruction_t::MOD:
            if (!b.integerValue) return -1;
            a.setInteger(a.integerValue % b.integerValue);
            break;

        case Instruction_t::NUMEQ:
            a.setInteger(a.realValue == b.realValue);
            break;

        case Instruction_t::NUMGE:
            a.setInteger(a.realValue >= b.realValue);
            break;

        case Instruction_t::NUMGT:
            a.setInteger(a.realValue > b.realValue);
            break;

        default:
            return -1;
        }

#ifdef HAVE_FENV_H
        if (fetestexcept(FE_ALL_EXCEPT) & (~FE_INEXACT)) {
            return -1;
        }
#endif
    } else {
        switch(instr.operation) {
        case Instruction_t::BITAND:
            a.setInteger(a.integerValue & b.integerValue);
            break;

        case Instruction_t::BITOR:
            a.setInteger(a.integerValue | b.integerValue);
            break;

        case Instruction_t::BITXOR:
            a.setInteger(a.integerValue ^ b.integerValue);
            break;

        case Instruction_t::ADD:
            a.setInteger(a.integerValue + b.integerValue);
            break;

        case Instruction_t::SUB:
            a.setInteger(a.integerValue - b.integerValue);
            break;

        case Instruction_t::MUL:
            a.setInteger(a.integerValue * b.integerValue);
            break;

        case Instruction_t::DIV:
            if (!b.integerValue) return -1;
            a.setInteger(a.integerValue / b.integerValue);
            break;

        case Instruction_t::MOD:
            if (!b.integerValue) return -1;
            a.setInteger(a.integerValue % b.integerValue);
            break;

        case Instruction_t::NUMEQ:
            a.setInteger(a.integerValue == b.integerValue);
            break;

        case Instruction_t::NUMGE:
            a.setInteger(a.integerValue >= b.integerValue);
            break;

        case Instruction_t::NUMGT:
            a.setInteger(a.integerValue > b.integerValue);
            break;

        default:
            return -1;
        }
    }

    valueStack.push(a);
    return 0;
}

int Processor_t::numOp(const Instruction_t &instr) {

    if (valueStack.size() < 2) {
        logErr(instr, "Value stack underflow", Error_t::LL_FATAL);
        return -1;
    }

    ParserValue_t b = valueStack.top();
    valueStack.pop();
    b.validateThis();

    ParserValue_t a = valueStack.top();
    valueStack.pop();
    a.validateThis();

    if ((a.type == ParserValue_t::TYPE_STRING) ||
        (b.type == ParserValue_t::TYPE_STRING)) {
        logErr(instr, "Numeric operation on string", Error_t::LL_ERROR);
        a.setString("undefined");
    } else if ((a.type == ParserValue_t::TYPE_REAL) ||
               (b.type == ParserValue_t::TYPE_REAL)) {
#ifdef HAVE_FENV_H
        feclearexcept(FE_ALL_EXCEPT);
#endif

        switch (instr.operation) {
        case Instruction_t::BITAND:
        case Instruction_t::BITOR:
        case Instruction_t::BITXOR:

            logErr(instr, "Bit operation with real",
                   Error_t::LL_ERROR);
            a.setString("undefined");
            break;

        case Instruction_t::ADD:
            a.setReal(a.realValue + b.realValue);
            break;

        case Instruction_t::SUB:
            a.setReal(a.realValue - b.realValue);
            break;

        case Instruction_t::MUL:
            a.setReal(a.realValue * b.realValue);
            break;

        case Instruction_t::DIV:
            a.setReal(a.realValue / b.realValue);
            break;

        case Instruction_t::MOD:
            if (!b.integerValue) {
                logErr(instr, "Modulo by zero", Error_t::LL_ERROR);
                a.setString("undefined");
                break;
            }
            a.setInteger(a.integerValue % b.integerValue);
            break;

        case Instruction_t::NUMEQ:
            a.setInteger(a.realValue == b.realValue);
            break;

        case Instruction_t::NUMGE:
            a.setInteger(a.realValue >= b.realValue);
            break;

        case Instruction_t::NUMGT:
            a.setInteger(a.realValue > b.realValue);
            break;

        default:
            logErr(instr, "Internal error, unknown numeric operation",
                   Error_t::LL_FATAL);
            return -1;
        }

#ifdef HAVE_FENV_H
        if (fetestexcept(FE_ALL_EXCEPT) & (~FE_INEXACT) & (~FE_INVALID)) {
            logErr(instr, "Invalid floating point operation",
                   Error_t::LL_ERROR);
            a.setString("undefined");
        }
#endif
    } else {
        switch (instr.operation) {
        case Instruction_t::BITAND:
            a.setInteger(a.integerValue & b.integerValue);
            break;

        case Instruction_t::BITOR:
            a.setInteger(a.integerValue | b.integerValue);
            break;

        case Instruction_t::BITXOR:
            a.setInteger(a.integerValue ^ b.integerValue);
            break;

        case Instruction_t::ADD:
            a.setInteger(a.integerValue + b.integerValue);
            break;

        case Instruction_t::SUB:
            a.setInteger(a.integerValue - b.integerValue);
            break;

        case Instruction_t::MUL:
            a.setInteger(a.integerValue * b.integerValue);
            break;

        case Instruction_t::DIV:
            if (!b.integerValue) {
                logErr(instr, "Division by zero", Error_t::LL_ERROR);
                a.setString("undefined");
                break;
            }
            a.setInteger(a.integerValue / b.integerValue);
            break;

        case Instruction_t::MOD:
            if (!b.integerValue) {
                logErr(instr, "Modulo by zero", Error_t::LL_ERROR);
                a.setString("undefined");
                break;
            }
            a.setInteger(a.integerValue % b.integerValue);
            break;

        case Instruction_t::NUMEQ:
            a.setInteger(a.integerValue == b.integerValue);
            break;

        case Instruction_t::NUMGE:
            a.setInteger(a.integerValue >= b.integerValue);
            break;

        case Instruction_t::NUMGT:
            a.setInteger(a.integerValue > b.integerValue);
            break;

        default:
            logErr(instr, "Internal error, unknown numeric operation",
                   Error_t::LL_FATAL);
            return -1;
        }
    }

    valueStack.push(a);
    return 0;
}

int Processor_t::evalBinaryOp(const Instruction_t &instr) {

    if (valueStack.size() < 2) return -1;
    ParserValue_t b = valueStack.top();
    valueStack.pop();

    ParserValue_t a = valueStack.top();
    valueStack.pop();

    switch (instr.operation) {
    case Instruction_t::CONCAT:
        a.stringValue = a.stringValue + b.stringValue;
        break;

    case Instruction_t::STREQ:
        if (a.stringValue == b.stringValue) a.setInteger(1);
        else a.setInteger(0);
        break;

    default:
        return -1;
    }

    valueStack.push(a);
    return 0;
}

int Processor_t::binaryOp(const Instruction_t &instr) {
    if (valueStack.size() < 2) {
        logErr(instr, "Value stack underflow",
               Error_t::LL_FATAL);
        return -1;
    }

    ParserValue_t b = valueStack.top();
    valueStack.pop();

    ParserValue_t a = valueStack.top();
    valueStack.pop();

    switch (instr.operation) {
    case Instruction_t::CONCAT:
        a.stringValue = a.stringValue + b.stringValue;
        break;

    case Instruction_t::REPEAT:
        b.validateThis();
        if ((b.type != ParserValue_t::TYPE_INT) || (b.integerValue < 0)) {
            logErr(instr, "REPEAT with wrong second argument",
                   Error_t::LL_ERROR);
            a.setString("undefined");
            break;
        }

        {
            string s = a.stringValue;
            a.setString("");
            a.stringValue.reserve(s.size() * b.integerValue);
            for (ParserValue_t::int_t i = 0; i < b.integerValue; i++) {
                a.stringValue += s;
            }
        }
        break;

    case Instruction_t::STREQ:
        if (a.stringValue==b.stringValue)
            a.setInteger(1); else a.setInteger(0);
        break;

    default:
        logErr(instr, "Internal error, unknown operation",
               Error_t::LL_FATAL);

        return -1;
    }
    valueStack.push(a);
    return 0;
}

namespace {

    int dumpFragment(const Configuration_t &configuration,
                     const Escaper_t &escaper,
                     Formatter_t &output, const Fragment_t &fragment,
                     const string &padding = string())
    {
        // dump all variables (no nestedFragments)
        for (Fragment_t::const_iterator ifragment = fragment.begin();
             ifragment != fragment.end(); ++ifragment) {
            if (!ifragment->second->nestedFragments) {
                if (output.write(padding)) return -1;
                if (output.write(ifragment->first)) return -1;
                if (output.write(escaper.escape(": \""))) return -1;

                // clip string to specified length
                std::string strVal = ifragment->second->value;
                int unsigned len = configuration.getMaxDebugValLength();
                if (len > 0)
                    Teng::clipString(strVal, len);

                if (output.write
                    (escaper.escape(strVal + "\"\n")))
                    return -1;

            }
        }

        // dump all fragments (nestedFragments non-null)
        for (Fragment_t::const_iterator ifragment = fragment.begin();
             ifragment != fragment.end(); ++ifragment) {
            if (ifragment->second->nestedFragments) {
                unsigned int k = 0;
                for (FragmentList_t::const_iterator
                         inestedFragments = ifragment->second->nestedFragments->begin();
                     inestedFragments != ifragment->second->nestedFragments->end();
                     ++inestedFragments, ++k) {
                    if (output.write(padding)) return -1;

                    char s[20];
                    if (output.write(ifragment->first)) return -1;
                    sprintf(s, "[%u]: \n", k);

                    if (output.write(escaper.escape(s))) return -1;
                    if (dumpFragment(configuration, escaper, output, **inestedFragments,
                                     padding + "    "))
                        return -1;
                    if (output.write(escaper.escape("\n"))) return -1;
                }
            }
        }

        // OK
        return 0;
    }
}

int Processor_t::instructionDebug(const Fragment_t &data, Formatter_t &output) {
    const Escaper_t &escaper = fParam.escaper;

    output.write(escaper.escape("Template sources:\n"));
    const SourceList_t &pl = program.getSources();
    for (unsigned int i = 0; i != pl.size(); ++i) {
        if (output.write(escaper.escape("    " + pl.getSource(i) + "\n")))
	    return -1;
    }

    output.write(escaper.escape("\nLanguage dictionary sources:\n"));
    const SourceList_t &l = langDictionary.getSources();
    for (unsigned int i = 0; i != pl.size(); ++i) {
        if (output.write(escaper.escape("    " + l.getSource(i) + "\n")))
	    return -1;
    }

    if (output.write(escaper.escape("\nConfiguration dictionary sources:\n")))
        return -1;
    const SourceList_t &p = configuration.getSources();
    for (unsigned int i = 0; i != pl.size(); ++i) {
        if (output.write(escaper.escape("    " + p.getSource(i) + "\n")))
	    return -1;
    }

    // configuration
    std::ostringstream os;
    os << configuration;
    if (output.write(escaper.escape("\n" + os.str())))
        return -1;

    if (output.write(escaper.escape("\nApplication data:\n")))
        return -1;
    return dumpFragment(configuration, escaper, output, data);
}

namespace {
    int dumpBytecode(const Escaper_t &escaper, const Program_t &program,
                     Formatter_t &output)
    {
        // create bytecode dump
        ostringstream os;
        for (Program_t::const_iterator iprogram = program.begin();
             iprogram != program.end(); ++iprogram) {
            os << "0x" << std::hex << std::setw(8) << std::setfill('0')
               << static_cast<unsigned int>(iprogram - program.begin()) << " ";
            iprogram->dump(os, static_cast<unsigned int>(iprogram - program.begin()));
        }

        // write to output
        return output.write(escaper.escape(os.str()));
    }
}

#define WARN_IF(...)\
    if ( existMarks == 0 )\
        logErr(__VA_ARGS__)

void Processor_t::run(const Fragment_t &data, Formatter_t &output,
                      Error_t &inError)
{
    ParserValue_t a;
    FragVal_t cVal;
    vector<ParserValue_t> programStack;
    stack<FragVal_t> fragmentValueStack;

    programStack.reserve(80);

    while (!valueStack.empty()) valueStack.pop();

    int ip = 0; // Never will be changed to unsigned !!

    // Number of nested exits to control error logging
    int existMarks = 0;

    // remember error
    error = &inError;

    // create fragment stack
    FragmentStack_t fragmentStack
        (&data, *error, configuration.isErrorFragmentEnabled());

    for (;;) {
        if ((ip < 0) || (ip >= (int)program.size())) {
            logErrNoInstr("Instruction pointer went "
                          "out of program address space",
                          Error_t::LL_FATAL);
            goto flushReturn;
        }
        const Instruction_t &instr = program[ip++];

        switch (instr.operation) {
        case Instruction_t::DEFINED:
            a.setInteger(!fragmentStack.exists(instr.identifier));
            if (a) {
                if (fragmentStack.findVariable(instr.identifier, a)) {
                    // Returns false if fragment
                    a.setInteger(static_cast<bool>(a));
                }
            }
            valueStack.push(a);
            break;

        case Instruction_t::EXIST:
            a.setInteger(!fragmentStack.exists(instr.identifier));
            valueStack.push(a);
            break;

        case Instruction_t::DEBUGING:
            if (configuration.isDebugEnabled())
                instructionDebug(data, output);
            break;

        case Instruction_t::BYTECODE:
            if (configuration.isBytecodeEnabled())
                dumpBytecode(fParam.escaper, program, output);
            break;

        case Instruction_t::VAL:
            valueStack.push(instr.value);
            break;

        case Instruction_t::DICT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();
            {
                const string *item;
                item = langDictionary.lookup(a.stringValue);
                if (item == 0)
                    item = configuration.lookup(a.stringValue);
                if (item == 0) {
                    logErr(instr, "Dictionary item '" + a.stringValue +
                           "' was not found",
                           Error_t::LL_WARNING);
                    item = &a.stringValue;
                }
                a.setString(*item);
            }
            valueStack.push(a);
            break;

        case Instruction_t::VAR:
            if (fragmentStack.findVariable(instr.identifier, a)) {
                logErr(instr, "Variable '" + instr.value.stringValue
                       + "' is undefined",
                       Error_t::LL_WARNING);
                a = ParserValue_t();
            } else {
                if ( configuration.isAlwaysEscapeEnabled() ) {
                    // check whether we have to escape variable
                    // FIXME: This is bug, type should be used
                    if (instr.value.integerValue)
                        a.setString(fParam.escaper.escape(a.stringValue));
                } else {
                    // Peek next inst and escape only if PRINT follows
                    if ( (ip < (int)program.size()) &&
                        program[ip].operation == Instruction_t::PRINT &&
                        instr.value.type == ParserValue_t::TYPE_STRING
                        ) {
                        a.setString(fParam.escaper.escape(a.stringValue));
                    }
                }
            }
            valueStack.push(a);
            break;

        case Instruction_t::PUSH:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();
            if (programStack.size() >= programStack.capacity())
                programStack.reserve(programStack.size() + 80);
            programStack.push_back(a);
            break;

        case Instruction_t::POP:
            if (programStack.empty()) {
                logErr(instr, "Program stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            programStack.pop_back();
            break;

        case Instruction_t::STACK:
            if (instr.value.integerValue > 0 ||
                -instr.value.integerValue >= (int)programStack.size()) {
                logErr(instr, "Program stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            valueStack.push(programStack[programStack.size() - 1 +
                                         instr.value.integerValue]);
            break;

        case Instruction_t::BITOR:
        case Instruction_t::BITXOR:
        case Instruction_t::BITAND:
        case Instruction_t::ADD:
        case Instruction_t::SUB:
        case Instruction_t::MUL:
        case Instruction_t::DIV:
        case Instruction_t::MOD:
        case Instruction_t::NUMEQ:
        case Instruction_t::NUMGE:
        case Instruction_t::NUMGT:
            if (numOp(instr) < 0) goto flushReturn;
            break;

        case Instruction_t::CONCAT:
        case Instruction_t::STREQ:
        case Instruction_t::REPEAT:
            if (binaryOp(instr) < 0) goto flushReturn;
            break;

        case Instruction_t::NOT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();
            a.setInteger(!a);
            valueStack.push(a);
            break;

        case Instruction_t::BITNOT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();
            a.validateThis();
            if (a.type != ParserValue_t::TYPE_INT) {
                logErr(instr, "Bit operation not with integer",
                       Error_t::LL_ERROR);
                a.setString("undefined");
            } else a.setInteger(~a.integerValue);
            valueStack.push(a);
            break;

        case Instruction_t::FUNC:
            {
                ParserValue_t::int_t i = instr.value.integerValue;
                int j;
                if (i < 0) {
                    logErr(instr, "Negative function argument count",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }

                if ((int)valueStack.size() < i) {
                    logErr(instr, "Value stack underflow",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }

                vector <ParserValue_t> v(i);
                for (j = 0; j < i; j++) {
                    v[j] = valueStack.top();
                    valueStack.pop();
                }

                Function_t p = tengFindFunction(instr.value.stringValue);
                UDFCallback_t udf = p == 0 ? findUDF(instr.value.stringValue) : udf;

                if (p || udf) {
                    string errmsg;
                    fParam.logger.setInstruction(&instr);
                    int res = p == 0 ? callUdf(v, a, udf, errmsg) : p(v, fParam, a);
                    switch (res) {
                    case 0:
                        break; // OK
                    case -1:
                        if ( p != 0 ) {
                            logErr(instr, "Bad argument count for function '"
                                + instr.value.stringValue + "()'",
                                Error_t::LL_ERROR);
                        } else {
                            logErr(instr, errmsg,
                                Error_t::LL_ERROR);
                        }
                        break;
                    default:
                        if ( p != 0 ) {
                            logErr(instr, "Function '"
                                + instr.value.stringValue
                                + "()' call failed",
                                Error_t::LL_ERROR);
                        } else {
                            logErr(instr, errmsg,
                                Error_t::LL_ERROR);
                        }
                    }
                    valueStack.push(a);
                } else {
                    logErr(instr, "Call to unknown function '"
                           + instr.value.stringValue + "()'",
                           Error_t::LL_ERROR);
                    a.setString("unknown");
                    valueStack.push(a);
                }
            }
            break;

        case Instruction_t::AND:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            if (!a) goto jump;
            valueStack.pop();
            break;

        case Instruction_t::OR:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            if (a) goto jump;
            valueStack.pop();
            break;

        case Instruction_t::JMPIFNOT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();
            if (a) break;

        case Instruction_t::JMP:
        jump:
            ip += instr.value.integerValue;
            if ((ip < 0) || (ip >= (int)program.size())) {
                logErrNoInstr("Jump points out of program address space",
                              Error_t::LL_FATAL);
                goto flushReturn;
            }
            break;

        case Instruction_t::FORM:
            if (configuration.isFormatEnabled())
                output.push((Formatter_t::Mode_t)instr.value.integerValue);
            break;

        case Instruction_t::ENDFORM:
            if (configuration.isFormatEnabled()) {
                if (output.pop() < 0) {
                    logErr(instr, "Format-object stack error",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }
            }
            break;

        case Instruction_t::FRAG:
            if (fragmentStack.pushFrame(instr.identifier)) {
                // fragment has no iterations => ok, jump over fragment
                ip += instr.value.integerValue;
                if ((ip < 0) || (ip >= (int)program.size())) {
                    logErr(instr, "Fragment jump points out of "
                           "program address space",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }
            }
            break;

        case Instruction_t::ENDFRAG:
            if (fragmentStack.nextIteration()) {
                // next iteration
                ip += instr.value.integerValue;
                if ((ip < 0) || (ip >= (int)program.size())) {
                    logErr(instr, "End-fragment jump points out of "
                           "program address space",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }
            } else {
                // no more iterations, we have to pop frame
                if (fragmentStack.popFrame()) {
                    logErr(instr, "Fragment stack underflow",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }
            }
            break;

        case Instruction_t::REPEATFRAG:
            if (!fragmentStack.repeatFragment(instr.identifier, ip)) {
                // OK some iteratiion -> jump to the fragment
                ip += instr.value.integerValue;
                if ((ip < 0) || (ip >= (int)program.size())) {
                    logErr(instr, "Repeat fragment jump points out of "
                           "program address space",
                           Error_t::LL_FATAL);
                    goto flushReturn;
                }
            }
            break;

        case Instruction_t::FRAGCNT:
            {
                unsigned int fragmentSize = 0;
                if (fragmentStack.getFragmentSize(instr.identifier,
                                                  fragmentSize)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' doesn't exist, cannot determine its size.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(fragmentSize);
                valueStack.push(a);
            }
            break;

        case Instruction_t::XFRAGCNT:
            {
                // size of unopened fragment
                unsigned int fragmentSize = 0;
                if (fragmentStack.getSubFragmentSize(instr.identifier,
                                                     fragmentSize)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' doesn't exist, cannot determine its size.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(fragmentSize);
                valueStack.push(a);
            }
            break;

        case Instruction_t::FRAGITR:
            {
                unsigned int fragmentIteration = 0;
                if (fragmentStack.getFragmentIteration(instr.identifier,
                                                       fragmentIteration)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' not open, cannot determine current iteration.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(fragmentIteration);
                valueStack.push(a);
            }
            break;

        case Instruction_t::FRAGFIRST:
            {
                unsigned int fragmentIteration = 0;
                if (fragmentStack.getFragmentIteration(instr.identifier,
                                                       fragmentIteration)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' not open, cannot determine whether "
                           "we are in first iteration.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(!fragmentIteration);
                valueStack.push(a);
            }
            break;

        case Instruction_t::FRAGLAST:
            {
                unsigned int fragmentIteration = 0;
                unsigned int fragmentSize = 0;
                if (fragmentStack.getFragmentIteration(instr.identifier,
                                                       fragmentIteration,
                                                       &fragmentSize)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' not open, cannot determine whether "
                           "we are in the last iteration.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(fragmentIteration == (fragmentSize - 1));
                valueStack.push(a);
            }
            break;

        case Instruction_t::FRAGINNER:
            {
                unsigned int fragmentIteration = 0;
                unsigned int fragmentSize = 0;
                if (fragmentStack.getFragmentIteration(instr.identifier,
                                                       fragmentIteration,
                                                       &fragmentSize)) {
                    logErr(instr, "Fragment '" + instr.value.stringValue
                           + "' not open, cannot determine whether "
                           "we are in an inner iteration.",
                           Error_t::LL_WARNING);
                }
                a.setInteger(fragmentIteration &&
                             (fragmentIteration < (fragmentSize - 1)));
                valueStack.push(a);
            }
            break;

        case Instruction_t::PRINT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            if (output.write(valueStack.top().stringValue)) {
                valueStack.pop();
                return;
            }
            valueStack.pop();
            break;

        case Instruction_t::SET:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            }
            a = valueStack.top();
            valueStack.pop();

            switch (fragmentStack.setVariable(instr.identifier, a)) {
            case S_OK:
                // OK
                break;
            case S_ALREADY_DEFINED:
                logErr(instr,
                       "Cannot rewrite variable '" + instr.value.stringValue
                       + "' which is already set by the application.",
                       Error_t::LL_WARNING);
                break;
            default:
                logErr(instr,
                       "Cannot set variable '" + instr.value.stringValue
                       + "'.",
                       Error_t::LL_WARNING);
                break;
            }
            break;

        case Instruction_t::HALT:
            goto flushReturn;

        case Instruction_t::CTYPE:
            fParam.escaper.push(instr.value.integerValue, *error,
                                position(instr, program));
            break;

        case Instruction_t::ENDCTYPE:
            fParam.escaper.pop(*error, position(instr, program));
            break;

        case Instruction_t::EXISTMARK:
            existMarks++;
            break;

        case Instruction_t::AT:
            if (valueStack.empty()) {
                logErr(instr, "Value stack underflow",
                       Error_t::LL_FATAL);
                goto flushReturn;
            } else if (fragmentValueStack.empty()) {
                logErr(instr, "Fragment value stack underflow",
                        Error_t::LL_FATAL);
                goto flushReturn;
            }
            // Fetch index
            a = valueStack.top();
            valueStack.pop();
            // Fetch fragment value
            cVal = fragmentValueStack.top();
            fragmentValueStack.pop();
            if ( cVal.type != FragVal_t::FRAGMENT_NULL ) {
                if ( a.type == ParserValue_t::TYPE_STRING ) {
                    const std::string &member = a.stringValue;
                    if ( cVal.type == FragVal_t::FRAGMENT ) {
                        Fragment_t::const_iterator it = cVal.frag->find(member);
                        if ( it == cVal.frag->end() ) {
                            WARN_IF(instr, "Unable to locate member (1) '" + member + "'",
                                Error_t::LL_WARNING);
                            cVal = FragVal_t();
                        } else {
                            cVal = FragVal_t(it->second);
                        }
                    } else if ( cVal.type == FragVal_t::FRAGMENT_VALUE && cVal.value->nestedFragments != 0 ) {
                        const FragmentList_t *nested = cVal.value->nestedFragments;
                        if ( nested->size() == 1 ) {
                            Fragment_t *frag = (*nested)[0];
                            Fragment_t::const_iterator it = frag->find(member);
                            if ( it == frag->end() ) {
                                WARN_IF(instr, "Unable to locate member (2) '" + member + "'",
                                    Error_t::LL_WARNING);
                                cVal = FragVal_t();
                            } else {
                                cVal = FragVal_t(it->second);
                            }
                        } else {
                            WARN_IF(instr, "String indices can be used only for fragments",
                                Error_t::LL_WARNING);
                        }
                    } else {
                        WARN_IF(instr, "String indices can be used only for fragments",
                            Error_t::LL_WARNING);
                    }
                } else if ( a.type == ParserValue_t::TYPE_INT ) {
                    if ( cVal.type == FragVal_t::FRAGMENT_LIST ) {
                        if ( a.integerValue < 0 || static_cast<size_t>(a.integerValue) >= cVal.list->size() ) {
                            WARN_IF(instr, "Index " + FragmentValue_t(a.integerValue).value +
                                " is out of range", Error_t::LL_WARNING);
                            cVal = FragVal_t();
                        } else {
                            cVal = FragVal_t((*cVal.list)[a.integerValue]);
                        }
                    } else if ( cVal.type == FragVal_t::FRAGMENT_VALUE && cVal.value->nestedFragments != 0 ) {
                        const FragmentList_t *nested = cVal.value->nestedFragments;
                        if ( a.integerValue < 0 || static_cast<size_t>(a.integerValue) >= nested->size() ) {
                            WARN_IF(instr, "Index " + FragmentValue_t(a.integerValue).value +
                                " is out of range", Error_t::LL_WARNING);
                            cVal = FragVal_t();
                        } else {
                            cVal = FragVal_t((*nested)[a.integerValue]);
                        }
                    } else {
                        WARN_IF(instr, "Only fragment lists can be indexed",
                            Error_t::LL_WARNING);
                        cVal = FragVal_t();
                    }
                } else {
                    WARN_IF(instr, "Invalid combination of index and entity type",
                        Error_t::LL_WARNING);
                    cVal = FragVal_t();
                }
            }
            fragmentValueStack.push(cVal);
            break;

        case Instruction_t::GETATTR:
            if ( instr.value.stringValue == "@(root)" ) {
                fragmentValueStack.push(FragVal_t(&data));
            } else if ( instr.value.stringValue == "@(this)" ) {
                fragmentValueStack.push(FragVal_t(fragmentStack.getCurrentFragment()));
            } else {
                if (fragmentValueStack.empty()) {
                    logErr(instr, "Fragment value stack underflow",
                            Error_t::LL_FATAL);
                    goto flushReturn;
                }

                const std::string &member = instr.value.stringValue;
                cVal = fragmentValueStack.top();
                fragmentValueStack.pop();

                if ( cVal.type == FragVal_t::FRAGMENT ) {
                    Fragment_t::const_iterator it = cVal.frag->find(member);
                    if ( it == cVal.frag->end() ) {
                        WARN_IF(instr, "Unable to locate member (3) '" + member + "'",
                            Error_t::LL_WARNING);
                        cVal = FragVal_t();
                    } else {
                        cVal = FragVal_t(it->second);
                    }
                } else if ( cVal.type == FragVal_t::FRAGMENT_VALUE ) {
                    if ( cVal.value->nestedFragments == 0 ) {
                        WARN_IF(instr, "Unable to locate member (4) '" + member + "'"
                            " in value",
                            Error_t::LL_WARNING);
                        cVal = FragVal_t();
                    } else {
                        if ( cVal.value->nestedFragments->size() == 1 ) {
                            const Fragment_t *frag = (*cVal.value->nestedFragments)[0];
                            Fragment_t::const_iterator it = frag->find(member);
                            if ( it == frag->end() ) {
                                WARN_IF(instr, "Unable to locate member (5) '" + member + "'",
                                    Error_t::LL_WARNING);
                                cVal = FragVal_t();
                            } else {
                                cVal = FragVal_t(it->second);
                            }
                        } else {
                            cVal = FragVal_t(cVal.value->nestedFragments);
                        }
                    }
                } else if ( cVal.type != FragVal_t::FRAGMENT_NULL ) {
                    WARN_IF(instr, "Unable to locate member (6) '" + member + "'"
                        " in fragment list",
                        Error_t::LL_WARNING);
                    cVal = FragVal_t();
                }
                fragmentValueStack.push(cVal);
            }
            break;

        case Instruction_t::REPR:
            if (fragmentValueStack.empty()) {
                logErr(instr, "Fragment value stack underflow",
                        Error_t::LL_FATAL);
                goto flushReturn;
            }

            cVal = fragmentValueStack.top();
            fragmentValueStack.pop();

            if ( instr.value.stringValue == "json" ) {
                stringstream os;
                switch ( cVal.type ) {
                    case FragVal_t::FRAGMENT:
                        cVal.frag->json(os);
                        break;
                    case FragVal_t::FRAGMENT_LIST:
                        cVal.list->json(os);
                        break;
                    case FragVal_t::FRAGMENT_VALUE:
                        cVal.value->json(os);
                        break;
                    default:
                        break;
                }
                a.setString(os.str());
            } else if ( instr.value.stringValue == "type" ) {
                switch ( cVal.type ) {
                    case FragVal_t::FRAGMENT:
                        a.setString("frag");
                        break;

                    case FragVal_t::FRAGMENT_LIST:
                        a.setString("list");
                        break;

                    case FragVal_t::FRAGMENT_VALUE:
                        if ( cVal.value->nestedFragments != 0 )
                            a.setString("valuelist");
                        else
                            a.setString("value");
                        break;

                    default:
                        a.setString("null");
                        break;
                }
            } else if ( instr.value.stringValue == "count" ) {
                switch ( cVal.type ) {
                    case FragVal_t::FRAGMENT:
                        a.setInteger(1);
                        break;

                    case FragVal_t::FRAGMENT_LIST:
                        a.setInteger(cVal.list->size());
                        break;

                    case FragVal_t::FRAGMENT_VALUE:
                        if ( cVal.value->nestedFragments != 0 )
                            a.setInteger(cVal.value->nestedFragments->size());
                        else
                            a.setInteger(1);
                        break;

                    default:
                        a.setString("null");
                        break;
                }
            } else if ( instr.value.stringValue == "exists" ) {
                existMarks--;
                switch ( cVal.type ) {
                    case FragVal_t::FRAGMENT:
                    case FragVal_t::FRAGMENT_LIST:
                    case FragVal_t::FRAGMENT_VALUE:
                        a.setInteger(1);
                        break;

                    default:
                        a.setInteger(0);
                        break;
                }
            } else {
                switch ( cVal.type ) {
                    case FragVal_t::FRAGMENT_NULL:
                        a.setString("$null$");
                        break;

                    case FragVal_t::FRAGMENT:
                        a.setString("$frag$");
                        break;

                    case FragVal_t::FRAGMENT_LIST:
                        a.setString("$fraglist$");
                        break;

                    case FragVal_t::FRAGMENT_VALUE:
                        if ( cVal.value->nestedFragments != 0 )
                            a.setString("$fraglist$");
                        else
                            a.setString(cVal.value->value);
                        break;

                    default:
                        a.setString("$null$");
                        break;
                }
            }
            valueStack.push(a);
            break;

        default:
            logErr(instr, "Unknown instruction",
                   Error_t::LL_FATAL);
            goto flushReturn;
        }
    }
 flushReturn:
    if (!programStack.empty()) {
        logErrNoInstr("Program stack is not empty",
                      Error_t::LL_WARNING);
    }
    if (!valueStack.empty()) {
        logErrNoInstr("Value stack is not empty",
                      Error_t::LL_WARNING);
    }
    if (!fragmentValueStack.empty()) {
        logErrNoInstr("Fragment value stack is not empty",
                      Error_t::LL_WARNING);
    }
    output.flush();
}

int Processor_t::eval(ParserValue_t &result, int startAddress,
                      int endAddress)
{
    ParserValue_t a, b;
    int ip = startAddress; // Never will be changed to unsigned !!

    // we must have fake error
    Error_t fakeError;
    error = &fakeError;

    vector<ParserValue_t> programStack;
    programStack.reserve(80);

    while (!valueStack.empty()) valueStack.pop();
    if (endAddress > (int)program.size() || startAddress < 0) return -1;
    while (ip != endAddress) {
        if (ip < startAddress || ip > endAddress) {
            return -1;
        }
        const Instruction_t &instr = program[ip++];

        switch (instr.operation) {
        case Instruction_t::REPEAT:
            if (b.type != ParserValue_t::TYPE_INT || b.integerValue < 0)
                return -1;
            {
                string s = a.stringValue;
                a.setString("");
                a.stringValue.reserve(s.size() * b.integerValue);
                for (ParserValue_t::int_t i = 0; i < b.integerValue; i++)
                    a.stringValue += s;
            }
            break;

        case Instruction_t::VAL:
            valueStack.push(instr.value);
            break;

        case Instruction_t::PUSH:
            if (valueStack.empty()) return -1;
            a = valueStack.top();
            valueStack.pop();
            if (programStack.size() >= programStack.capacity())
                programStack.reserve(programStack.size() + 80);
            programStack.push_back(a);
            break;

        case Instruction_t::POP:
            if (programStack.empty()) return -1;
            programStack.pop_back();
            break;

        case Instruction_t::STACK:
            if (instr.value.integerValue > 0 ||
                -instr.value.integerValue >= (int)programStack.size())
                return -1;
            valueStack.push(programStack[programStack.size() - 1 +
                                         instr.value.integerValue]);
            break;

        case Instruction_t::BITOR:
        case Instruction_t::BITXOR:
        case Instruction_t::BITAND:
        case Instruction_t::ADD:
        case Instruction_t::SUB:
        case Instruction_t::MUL:
        case Instruction_t::DIV:
        case Instruction_t::MOD:
        case Instruction_t::NUMEQ:
        case Instruction_t::NUMGE:
        case Instruction_t::NUMGT:
            if (evalNumOp(instr) < 0) return -1;
            break;

        case Instruction_t::CONCAT:
        case Instruction_t::STREQ:
            if (evalBinaryOp(instr) < 0) return -1;
            break;

        case Instruction_t::BITNOT:
            if (valueStack.empty()) return -1;
            a = valueStack.top();
            valueStack.pop();
            a.validateThis();
            if (a.type != ParserValue_t::TYPE_INT) return -1;
            a.setInteger(~a.integerValue);
            valueStack.push(a);
            break;

        case Instruction_t::NOT:
            if (valueStack.empty()) return -1;
            a = valueStack.top();
            valueStack.pop();
            a.setInteger(!a);
            valueStack.push(a);
            break;

        case Instruction_t::FUNC:
            {
                ParserValue_t::int_t i = instr.value.integerValue;
                int j;
                if (i < 0) return -1;
                if ((int)valueStack.size() < i) return -1;
                vector <ParserValue_t> v(i);
                for (j = 0; j < i; j++) {
                    v[j] = valueStack.top();
                    valueStack.pop();
                }
                Function_t p = tengFindFunction(instr.value.stringValue, false);
                ///TODO: UDF const optimizations
                //UDF_t *udf = p == 0 ? findUDF(instr.value.stringValue) : 0;
                if ( p /*|| udf*/ ) {
                    string errmsg;
                    fParam.logger.setInstruction(&instr);
                    //int res = p == 0 ? callUdf(v, a, udf, errmsg) : p(v, fParam, a);
                    int res = p(v, fParam, a);
                    switch (res) {
                    case 0:
                        break; // OK
                    case -1:
                        return -1;
                    default:
                        return -1;
                    }
                    valueStack.push(a);
                }
                else return -1;
            }
            break;

        case Instruction_t::AND:
            if (valueStack.empty()) return -1;
            a = valueStack.top();
            if (!a) goto jump;
            valueStack.pop();
            break;

        case Instruction_t::OR:
            if (valueStack.empty()) return -1;
            a = valueStack.top();
            if (a) goto jump;
            valueStack.pop();
            break;

        case Instruction_t::JMPIFNOT:
            if (valueStack.empty()) return -1;
            a=valueStack.top();
            valueStack.pop();
            if (a) break;

        case Instruction_t::JMP:
        jump:
            ip += instr.value.integerValue;
            break;

        default:
            return -1;
        }
    }
    if (valueStack.size() != 1) return -1;
    result = valueStack.top();
    return 0;
}
