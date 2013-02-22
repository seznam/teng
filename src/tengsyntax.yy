/* !don't remove! -*- C++ -*-
 *
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
 * $Id: tengsyntax.yy,v 1.14 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng syntax analyzer.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-19  (stepan)
 *             Created.
 * 2006-06-21  (sten__)
 *             Better error reporting when unexpected EOF
 */

// all external symbols will be prefixed with tengSyntax_
%name-prefix="tengSyntax_"

%lex-param {void * scanner}

// we want to have y.tab.h
%defines

%{

// includes
#include <stdio.h>
#include <string>
#include <map>

#include "tengyystype.h"
#include "tengerror.h"
#include "tenginstruction.h"
#include "tengparsercontext.h"
#include "tengparservalue.h"
#include "tengformatter.h"
#include "tengcode.h"
#include "tenglex2.h"
#include "tengaux.h"

using namespace std;

using namespace Teng;

namespace Teng {

#define YYPARSE_PARAM context
#define YYLEX_PARAM context

// Cause the `yydebug' variable to be defined
//#define YYDEBUG 1

// verbose errors
#define YYERROR_VERBOSE

// define size of the parser stack
// (now, 200 is enough - grammar was fixed)
//#define YYINITDEPTH 200
// (default value 200 is too small for complex templates)
#define YYINITDEPTH 10000

// local function prototypes
static int yylex(YYSTYPE *leftValue, void *context);
static int yyerror(const char *msg);

// errlog function
static void printUnexpectedElement(ParserContext_t *context,
        int element, const YYSTYPE &leftValue);


// if enabled debug mode
#if YYDEBUG
// print element's semantic value
#define YYPRINT(FILE,YYCHAR,YYLVAL) yyprint(FILE,YYCHAR,YYLVAL)
static void yyprint(FILE *fp, int element, const YYSTYPE &leftValue);
#endif


// last error message (syntax or parse error)
// global variable -- ugly hack :-((
std::string tengSyntax_lastErrorMessage;


// max depth for templates included templates (looping protection)
#define MAX_INCLUDE_DEPTH 10

// macro for casting 'context' to the proper type
#define CONTEXT            (reinterpret_cast<ParserContext_t *> (context))

// macro for appending error class in actual context
// error levels are: DEBUG, WARNING, ERROR, FATAL
#define ERR(level, pos, msg)    do { \
                        (reinterpret_cast<ParserContext_t *> \
                        (context))->program->getErrors().logError( \
                        (Error_t::LL_##level), (pos), (msg)); \
                        } while (0)

// macros for byte-code generation
#define CODE(code)        do { \
                        tengCode_generate( \
                        (reinterpret_cast<ParserContext_t *> (context)), \
                        Instruction_t::code); \
                        } while (0)

#define CODE_VAL(code, val) \
                        do { \
                        tengCode_generate( \
                        (reinterpret_cast<ParserContext_t *> (context)), \
                        Instruction_t::code, val); \
                        } while (0)

#define CODE_VAL_VAR(opcode, val) \
                        do { \
                        tengCode_generate( \
                        (reinterpret_cast<ParserContext_t *> (context)), \
                        opcode, val); \
                        } while (0)

static inline void buildIdentifier(LeftValue_t &v) {
    v.val = ParserValue_t(); //clear
    for (LeftValue_t::Identifier_t::const_iterator i
             = v.id.begin(); i != v.id.end(); ++i)
        v.val.stringValue += "." + *i;
}

namespace {
    enum VariableName_t {
        VN_REGULAR,
        VN_COUNT,
        VN_NUMBER,
        VN_FIRST,
        VN_INNER,
        VN_LAST
    };

    struct VariableMapping_t {
        const char *name;
        VariableName_t variable;
        Instruction_t::OpCode_t opcode;
    };

    VariableMapping_t variableMapping[] = {
        {
            "_count",
            VN_COUNT,
            Instruction_t::FRAGCNT
        }, {
            "_number",
            VN_NUMBER,
            Instruction_t::FRAGITR
        }, {
            "_first",
            VN_FIRST,
            Instruction_t::FRAGFIRST
        }, {
            "_inner",
            VN_INNER,
            Instruction_t::FRAGINNER
        }, {
            "_last",
            VN_LAST,
            Instruction_t::FRAGLAST
        }, {
            0,
            VN_REGULAR,
            Instruction_t::VAR
        }
    };

    const VariableMapping_t& findVariableMapping(const std::string &name) {
        // ignore names not starting with underscore
        if (name.empty() || (name[0] != '_')) {
            return *(variableMapping +
                     (sizeof(variableMapping) / sizeof(VariableMapping_t))
                     - 1);
        }

        VariableMapping_t *sv = variableMapping;
        for ( ; sv->name; ++sv)
            if (!name.compare(sv->name)) return *sv;

        // not found
        return *sv;
    }
}

/** @short Generate code for variable.
 * @param result use $$ as this parameter
 * @param val value with variable name ($x).
 */
static inline void codeForVariable(void *context,
        LeftValue_t &result, LeftValue_t &val)
{
    // clear val and copy variable identifier
    result.val = ParserValue_t(); //clear
    result.id = val.id;
    result.val.stringValue = val.val.stringValue;

    // error indicator
    bool found = false;

    if (result.id.size() == 0) {
        // bad variable identifier -- code fake value
        ERR(ERROR, val.pos, "Invalid variable identifier "
            "in ${...} directive");
    } else {
        // identifier of variable/fragment
        Identifier_t id;

        const VariableMapping_t
            &varMapping(findVariableMapping(result.id.back()));

        switch (varMapping.variable) {
        case VN_COUNT:
            {
                // remove automatic variable name
                result.id.pop_back();

                // rebuild identifier
                buildIdentifier(result);

                // try to find fragment
                ParserContext_t::FragmentResolution_t fr =
                    CONTEXT->findFragment(&val.pos, result.id,
                                          result.val.stringValue, id, true);
                switch (fr) {
                case ParserContext_t::FR_NOT_FOUND:
                    // no-op -- we have nothing found
                    break;
                case ParserContext_t::FR_FOUND:
                    // fragment found
                    found = true;

                    CODE_VAL(FRAGCNT, result.val);
                    CONTEXT->program->back().identifier = id;
                    break;
                case ParserContext_t::FR_PARENT_FOUND:
                    // parent fragment found -- this fragment is not open
                    // and we must have some runtime overhead to find count
                    // of it
                    found = true;

                    CODE_VAL(XFRAGCNT, result.val);
                    CONTEXT->program->back().identifier = id;
                    break;
                }
            }
            break;

        case VN_NUMBER:
        case VN_FIRST:
        case VN_INNER:
        case VN_LAST:
            {
                // remove automatic variable name
                result.id.pop_back();

                // rebuild identifier
                buildIdentifier(result);

                // try to find fragment
                if (CONTEXT->findFragment(&val.pos, result.id,
                                          result.val.stringValue, id)) {
                    // we have found fragment
                    found = true;

                    // add instruction
                    CODE_VAL_VAR(varMapping.opcode, result.val);
                    CONTEXT->program->back().identifier = id;
                }
            }
            break;

        default:
            // find fragment for this variable
            if (CONTEXT->findFragmentForVariable(val.pos, result.id,
                    result.val.stringValue, id)) {
                // variable found
                found = true;

                // generate code for variable
                result.val.integerValue = 1; //escape
                CODE_VAL(VAR, result.val);

                // add identifier
                CONTEXT->program->back().identifier = id;
            }
        }
    }

    if (!found) {
        // no such varibale or fragment => generate code for undefined value
        result.val.setString("undefined");
        CODE_VAL(VAL, result.val);
    }
}

%}

// create reentrant parser without global variables
%pure_parser

// include also token table yytname
// (list of all terminal/nonterminal symbols)
%token_table


// list of lexical elements
// use %left, %right and %nonassoc instead of %token to specify
// associativity and precedence (increasing for subsequent entries).

// plain text
%token LEX_TEXT

// teng directives
%token LEX_DEBUG
%token LEX_BYTECODE
%token LEX_INCLUDE
%token LEX_FORMAT
%token LEX_ENDFORMAT
%token LEX_FRAGMENT
%token LEX_ENDFRAGMENT
%token LEX_IF
%token LEX_ELSEIF
%token LEX_ELSE
%token LEX_ENDIF
%token LEX_SET
%token LEX_EXPR
%token LEX_TENG
%token LEX_END
%token LEX_SHORT_EXPR
%token LEX_SHORT_DICT
%token LEX_SHORT_END
%token LEX_CTYPE
%token LEX_ENDCTYPE
%token LEX_REPEATFRAG

// assignment
%token LEX_ASSIGN
%token LEX_COMMA

// conditional expression
%right LEX_COND_EXPR LEX_COLON

// logic operators (must be right-associative for proper code-generation)
%right LEX_OR
%right LEX_AND

// bitwise operators
%left LEX_BITOR
%left LEX_BITXOR
%left LEX_BITAND

// comparison operators
%left LEX_EQ LEX_NE LEX_STR_EQ LEX_STR_NE
%left LEX_GE LEX_LE LEX_GT LEX_LT

// expressions
%left LEX_ADD LEX_SUB LEX_CONCAT
%left LEX_MUL LEX_DIV LEX_MOD LEX_REPEAT
%right LEX_NOT LEX_BITNOT LEX_UNARY //LEX_UNARY is fake terminal

// other keywords/operators
%token LEX_CASE
%token LEX_DEFINED
%token LEX_EXIST
%token LEX_EXISTS
%token LEX_JSONIFY
%token LEX_TYPE
%token LEX_COUNT

// parentheses
%token LEX_L_PAREN LEX_R_PAREN
%token LEX_L_BRACKET LEX_R_BRACKET

// identifiers and literals
%right LEX_VAR LEX_DICT LEX_DICT_INDIRECT
%token LEX_SELECTOR
%token LEX_UDF_IDENT
%token LEX_IDENT
%token LEX_STRING
%token LEX_INT
%token LEX_REAL

// start symbol
%start start

// grammar rules
%%


start:
    template
        {
            CODE(HALT); //end of template
        }

    // start error handling
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(FATAL, CONTEXT->position, "Big, fatal or "
                        "unhandled parse error in template");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
            CONTEXT->program->erase(CONTEXT->program->begin(),
                    CONTEXT->program->end()); //discard program
            CODE(HALT); //end of program
        }
    ;


template:
    template
    LEX_TEXT
        {
            CODE_VAL(VAL, $2.val); //static text
            tengCode_generatePrint(CONTEXT);
        }
    | template teng_directive
    | //empty
    ;


teng_directive:
    teng_unknown
    | teng_debug
    | teng_bytecode
    | teng_include
    | teng_format
    | teng_fragment
    | teng_if
    | teng_set
    | teng_expr
    | teng_dict
    | teng_ctype
    | teng_repeatfrag
    ;


teng_unknown:
    LEX_TENG error LEX_END
        {
            ERR(ERROR, $1.pos, "Unknown <?teng"
                    + $1.val.stringValue + "?> directive");
        }
    ;


teng_debug:
    LEX_DEBUG no_options_LEX_END
        {
            CODE(DEBUGING); //print debug info
        }
    ;


teng_bytecode:
    LEX_BYTECODE no_options_LEX_END
        {
            CODE(BYTECODE); //print disassembled bytecode
        }
    ;


no_options_LEX_END:
    options LEX_END
        {
            if ($1.opt.size() > 0)
                ERR(ERROR, $1.pos, "Teng directive "
                        "does not accept any option(s)");
        }

    // no_options_LEX_END error handling
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position, "Syntax error "
                        "inside <?teng ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END //no code may be produced here
    ;


options:
    identifier LEX_ASSIGN LEX_STRING options
        {
            $$.opt = $4.opt; //get all nested options
            $$.opt.insert(make_pair($1.val.stringValue, $3.val.stringValue));
            $$.pos = $1.pos; //propagate start of options position
        }
    | //empty
        {
            $$.opt.erase($$.opt.begin(), $$.opt.end()); //clear residuals
        }
    ;


teng_include:
    LEX_INCLUDE options LEX_END
        {
            // file may be included only at the end of teng directive,
            // because of flex-generated lexical analyzer is not reentrant.
            LeftValue_t::OptionList_t::const_iterator i = $2.opt.find("file");
            if (i == $2.opt.end()) {
                ERR(ERROR, $1.pos, "Cannot include a file; "
                        "option 'file' is not specified");
            } else if (CONTEXT->lex1.size() >=
                       CONTEXT->paramDictionary->getMaxIncludeDepth()) {
                ERR(ERROR, $1.pos, "Cannot include a file; template "
                        "nesting level is too deep");
            } else {
                // glue filename
                string fname = i->second;
                if (!CONTEXT->root.empty() && !fname.empty() && fname[0] != '/')
                    fname = CONTEXT->root + "/" + fname;
                // create new level #1 lex analyzer with file input
                CONTEXT->lex1.push(new Lex1_t(fname,
                        CONTEXT->lex1.top()->getPosition(),
                        CONTEXT->program->getErrors())); //new lex1
                // lex2 state should be 0 now (or after a while)
                CONTEXT->tengLex2.finish();
                CONTEXT->lex2 = 0; //for sure
                // append source list
                CONTEXT->sourceIndex.push( //remember source index
                        CONTEXT->program->addSource(fname,
                        CONTEXT->lex1.top()->getPosition()));
            }
        }

    // teng_include error handling
    | LEX_INCLUDE error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid <?teng "
                        "include ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
    ;


teng_format:
    LEX_FORMAT options LEX_END
        {
            // get chosen formating type
            LeftValue_t::OptionList_t::const_iterator i = $2.opt.find("space");
            if (i == $2.opt.end() || i->second.length() == 0) {
                ERR(ERROR, $1.pos,"Formatting block has no effect; "
                        "option 'space' is not specified or is empty");
            } else {
                // reset val
                $$.val = ParserValue_t();
                $$.val.integerValue = -1;
                // check space option
                if (i->second == "nowhite") {
                    $$.val.integerValue = Formatter_t::MODE_NOWHITE;
                } else if (i->second == "onespace") {
                    $$.val.integerValue = Formatter_t::MODE_ONESPACE;
                } else if (i->second == "striplines") {
                    $$.val.integerValue = Formatter_t::MODE_STRIPLINES;
                } else if (i->second == "joinlines") {
                    $$.val.integerValue = Formatter_t::MODE_JOINLINES;
                } else if (i->second == "nowhitelines") {
                    $$.val.integerValue = Formatter_t::MODE_NOWHITELINES;
                } else if (i->second == "noformat") {
                    $$.val.integerValue = Formatter_t::MODE_PASSWHITE;
                } else {
                    ERR(ERROR, $1.pos, "Unsupported value '" + i->second
                            + "' of 'space' formatting option");
                }
                // if not err, generate code
                if ($$.val.integerValue >= 0)
                    CODE_VAL(FORM, $$.val);
            }
        }
    template LEX_ENDFORMAT no_options_LEX_END
        {
            // if was not error no block start
            if ($4.val.integerValue >= 0)
                CODE(ENDFORM); //generate code
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }

    // teng_format error handling
    | LEX_FORMAT error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid <?teng format ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END template LEX_ENDFORMAT no_options_LEX_END
    | LEX_FORMAT error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Syntax error in teng format block; "
                        "discarding block content");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDFORMAT no_options_LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                // If EOF, do not print this message
                printUnexpectedElement(CONTEXT, yychar, yylval);
                if (yychar) // I don't know why, but when unexpected EOF
                            // occurs, this error handler is called
                    ERR(ERROR, CONTEXT->position,
                        "Misplaced <?teng endformat?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDFORMAT no_options_LEX_END
        {
            // discard program from point the LEX_ENDFORMAT was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    ;


teng_fragment:
    LEX_FRAGMENT variable_identifier LEX_END
        {
            // save actual prog size/addr
            $$.prgsize = CONTEXT->program->size();
            // clear val and copy variable identifier
            $$.val = ParserValue_t(); //clear
            $$.id = $2.id;
            $$.val.stringValue = $2.val.stringValue;

            Identifier_t id;
            if (CONTEXT->pushFragment($2.pos, $$.id, $$.val.stringValue, id)) {
                // generate code
                CODE_VAL(FRAG, $$.val);
                // set identifier
                CONTEXT->program->back().identifier = id;
            } else {
                // error in fragment, erase identifier to mark error
                $$.id.clear();
            }
        }
    template LEX_ENDFRAGMENT no_options_LEX_END
        {
            // pop previous fragment or remove invalid code
            if ($4.id.empty()) CONTEXT->cropCode($4.prgsize);
            else {
                // generate code for fragment end
                CODE(ENDFRAG);
                // pop frame
                CONTEXT->popFragment($4.prgsize);
            }
        }

    // teng_fragment error handling
    | LEX_FRAGMENT error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid <?teng frag ...?> directive; "
                        "discarding fragment block content");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END template LEX_ENDFRAGMENT no_options_LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | LEX_FRAGMENT error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Syntax error in teng fragment block; "
                        "discarding block content");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDFRAGMENT no_options_LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position,
                        "Misplaced <?teng endfrag?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDFRAGMENT no_options_LEX_END
        {
            // discard program from point the LEX_ENDFRAGMENT was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    ;


teng_set:
    LEX_SET voluntary_dollar_before_var variable_identifier
    LEX_ASSIGN expression LEX_END
        {
            // clear val and copy variable identifier
            $$.val = ParserValue_t(); //clear
            $$.id = $3.id;
            $$.val.stringValue = $3.val.stringValue;

            // error indicator
            bool found = false;

            if ($$.id.size() == 0) {
                // bad variable identifier
                ERR(ERROR, $3.pos, "Invalid variable identifier; "
                        "variable will not be set");
            } else {
                Identifier_t id;
                if (CONTEXT->
                    findFragmentForVariable($$.pos, $$.id,
                                            $$.val.stringValue, id)) {
                    // fully qualified variable identifier is in
                    // $$.val.stringValue
                    CODE_VAL(SET, $$.val);
                    // set identifier
                    CONTEXT->program->back().identifier = id;
                    found = true;
                }
            }
            if (!found) {
                // variable not found => destroy program for expression
                CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                                        CONTEXT->program->end());
            }
        }

    // teng_set error handling
    | LEX_SET voluntary_dollar_before_var variable_identifier
    LEX_ASSIGN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid expression in <?teng set ...?> "
                        "directive; variable '" +$3.val.stringValue
                        + "' will not be set");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | LEX_SET error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid variable identifier in "
                        "<?teng set ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ASSIGN expression LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | LEX_SET error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid <?teng set ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    ;


teng_if:
    LEX_IF expression LEX_END
        {
            // here is $4
            $$.prgsize = CONTEXT->program->size(); //save actual addr
            CODE(JMPIFNOT); //if not true, jump to next case
        }
    template
        {
            // here is $6
            $$.prgsize = CONTEXT->program->size(); //save actual addr
        }
    teng_else LEX_ENDIF no_options_LEX_END
        {
            // calculate jump offset
            // add +1 for end-jump if some else(if) section was generated
            (*CONTEXT->program)[$4.prgsize].value.integerValue =
                    $6.prgsize - $4.prgsize - 1
                    + ($6.prgsize != CONTEXT->program->size());
            // update all end-jumps with proper address
            LeftValue_t::AddressList_t::const_iterator i;
            for (i = $7.addr.begin(); i != $7.addr.end(); ++i) {
                (*CONTEXT->program)[*i].value.integerValue =
                        CONTEXT->program->size() - *i - 1;
            }
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }

    // teng_if error handling
    | LEX_IF error
        {
            // if expression error, behave like the expression is true
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Error in condition expression "
                        "in <?teng if ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard expression program
        }
    template
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
        }
    teng_else LEX_ENDIF no_options_LEX_END
        {
        	// delete all else(if) code for the faily if expression
            CONTEXT->program->erase(CONTEXT->program->begin() + $7.prgsize,
                    CONTEXT->program->end()); //discard further else-parts
        }
    | LEX_IF error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Syntax error in teng conditional "
                        "block; discarding block content");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDIF no_options_LEX_END
        {
        	// drop whole if section code
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard else-parts of program
        }
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position, "Misplaced "
                        "<?teng elseif ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ELSEIF error LEX_END
        {
            // discard program from point the LEX_ELSEIF was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position,
                        "Misplaced <?teng else?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ELSE no_options_LEX_END
        {
            // discard program from point the LEX_ELSE was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position,
                        "Misplaced <?teng endif?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDIF no_options_LEX_END
        {
            // discard program from point the LEX_ENDIF was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    ;


teng_else:
    LEX_ELSE no_options_LEX_END
        {
            // insert previous program block end-jump
            $$.addr.erase($$.addr.begin(), $$.addr.end()); //clear
            $$.addr.push_back(CONTEXT->program->size()); //save end-jump addr
            CODE(JMP); //previous was true, skip else section
        }
    template
        {
            $$.addr = $3.addr; //preserve end-jump info
        }
    | LEX_ELSEIF
        {
            // insert end jump
            $$.prgsize = CONTEXT->program->size(); //end-jump addr
            CODE(JMP); //previous was true, skip future elseif and else sections
        }
    expression LEX_END
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
            CODE(JMPIFNOT); //if not true, jump to the next case
        }
    template
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
        }
    teng_else
        {
            // calculate jump offset
            // add +1 for end-jump if some else(if) section was generated
            (*CONTEXT->program)[$5.prgsize].value.integerValue =
                    $7.prgsize - $5.prgsize - 1
                    + ($7.prgsize != CONTEXT->program->size());
            // preserve end-jumps info
            $$.addr = $8.addr;
            $$.addr.push_back($2.prgsize);
        }
    | //empty
        {
            $$.addr.erase($$.addr.begin(), $$.addr.end()); //clear residuals
        }

    // teng_if error handling
    | LEX_ELSEIF error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Error in condition expression "
                        "in <?teng elseif ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard expression program
        }
    template
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
        }
    teng_else
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $7.prgsize,
                    CONTEXT->program->end()); //discard further else-parts
        }
    ;


teng_expr:
    LEX_EXPR expression LEX_END
        {
            tengCode_generatePrint(CONTEXT); //print expression value
        }
    | LEX_SHORT_EXPR expression LEX_SHORT_END
        {
            tengCode_generatePrint(CONTEXT); //print expression value
        }
    | LEX_SHORT_EXPR variable_identifier LEX_SHORT_END
        {
            // generate code for variable
            codeForVariable(context, $$, $2);
            // print value
            tengCode_generatePrint(CONTEXT);
        }

    // teng_expr error handling
    | LEX_EXPR error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid expression "
                        "in <?teng expr ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
            tengCode_generatePrint(CONTEXT); //print expression 'undef' value
        }
    | LEX_SHORT_EXPR error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid expression in ${...} statement");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_SHORT_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
            tengCode_generatePrint(CONTEXT); //print expression 'undef' value
        }
    ;


teng_dict:
    LEX_SHORT_DICT dictionary_item LEX_SHORT_END
        {
            tengCode_generatePrint(CONTEXT); //print dictionary item's value
        }

    // teng_dict error handling
    | LEX_SHORT_DICT error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid dictionary item "
                        "in #{...} statement");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_SHORT_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
            tengCode_generatePrint(CONTEXT); //print 'undef' value
        }
    ;


teng_ctype:
    LEX_CTYPE LEX_STRING LEX_END
        {
            // reset val
            $$.val = ParserValue_t();
            $$.val.integerValue = -1;

            // get content type descriptor for given type
            const ContentType_t::Descriptor_t *ct
                = ContentType_t::findContentType($2.val.stringValue,
                                                 CONTEXT->program->getErrors(),
                                                 $2.pos, true);

            if (ct) {
                $$.val.integerValue = ct->index;
                CODE_VAL(CTYPE, $$.val);
            }
        }

    template LEX_ENDCTYPE no_options_LEX_END
        {
            // if was not error no block start
            if ($4.val.integerValue >= 0)
                CODE(ENDCTYPE); //generate code
            // no print-values join below following address
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }

    // teng_ctype error handling
    | LEX_CTYPE error
        {
            if (!tengSyntax_lastErrorMessage.empty()) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid <?teng ctype ...?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_END template LEX_ENDCTYPE no_options_LEX_END
    | LEX_CTYPE error
        {
            if (!tengSyntax_lastErrorMessage.empty()) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Syntax error in teng ctype block; "
                        "discarding block content");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDCTYPE no_options_LEX_END
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    | error
        {
            if (!tengSyntax_lastErrorMessage.empty()) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, CONTEXT->position,
                        "Misplaced <?teng endctype?> directive");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_ENDCTYPE no_options_LEX_END
        {
            // discard program from point the LEX_ENDCTYP was read
            // (destroy HALT instruction created by reduction start->template)
            CONTEXT->program->erase(CONTEXT->program->begin() + $3.prgsize,
                    CONTEXT->program->end()); //discard program
        }
    ;

teng_repeatfrag:
    LEX_REPEATFRAG variable_identifier LEX_END
        {
            // get address of referenced fragment
            Identifier_t id;
            int address = CONTEXT->getFragmentAddress($2.pos, $2.id,
                                                      $2.val.stringValue, id);
            if (address >= 0) {
                // set offset
                $2.val.integerValue = address - CONTEXT->program->size();

                // generate instruction
                CODE_VAL(REPEATFRAG, $2.val);

                // set identifier
                CONTEXT->program->back().identifier = id;

                // do not optimize (join) print-vals across current
                // prog end-addr
                CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
            }
        }
    ;

expression:
    LEX_L_PAREN expression LEX_R_PAREN
        {
            $$.prgsize = $2.prgsize; //start of expr prog
        }
    | expression LEX_EQ expression
        {
            CODE(NUMEQ);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_NE expression
        {
            CODE(NUMEQ);
            CODE(NOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_GE expression
        {
            CODE(NUMGE);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_LE expression
        {
            CODE(NUMGT);
            CODE(NOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_GT expression
        {
            CODE(NUMGT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_LT expression
        {
            CODE(NUMGE);
            CODE(NOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_STR_EQ expression
        {
            CODE(STREQ);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_STR_NE expression
        {
            CODE(STREQ);
            CODE(NOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_OR
        {
            $$.prgsize = CONTEXT->program->size(); //save actual prog size
            CODE(OR); //generate infix code
        }
    expression
        {
            // calc and update jump offset
            (*CONTEXT->program)[$3.prgsize].value.integerValue =
                    CONTEXT->program->size() - $3.prgsize - 1;
            // try to optimalize
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }
    | expression LEX_AND
        {
            $$.prgsize = CONTEXT->program->size(); //save actual prog size
            CODE(AND); //generate infix code
        }
    expression
        {
            // calc and update jump offset
            (*CONTEXT->program)[$3.prgsize].value.integerValue =
                    CONTEXT->program->size() - $3.prgsize - 1;
            // try to optimalize
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }
    | expression LEX_BITOR expression
        {
            CODE(BITOR);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_BITXOR expression
        {
            CODE(BITXOR);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_BITAND expression
        {
            CODE(BITAND);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_ADD expression
        {
            CODE(ADD);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_SUB expression
        {
            CODE(SUB);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_CONCAT expression
        {
            CODE(CONCAT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_MUL expression
        {
            CODE(MUL);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_DIV expression
        {
            CODE(DIV);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_MOD expression
        {
            CODE(MOD);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_REPEAT expression
        {
            CODE(REPEAT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | expression LEX_COND_EXPR
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
            CODE(JMPIFNOT); //jump offset stored later
        }
    expression LEX_COLON
        {
            // correct conditional jump offset (relative addr) +1=JMP
            (*CONTEXT->program)[$3.prgsize].value.integerValue =
                    CONTEXT->program->size() - $3.prgsize - 1 + 1;
            $$.prgsize = CONTEXT->program->size(); //save actual addr
            CODE(JMP); //jump stored later
        }
    expression
        {
            // correct jump offset (relative addr)
            (*CONTEXT->program)[$6.prgsize].value.integerValue =
                    CONTEXT->program->size() - $6.prgsize - 1;
            // try to optimalize
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }
    | LEX_NOT expression
        {
            CODE(NOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | LEX_BITNOT expression
        {
            CODE(BITNOT);
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | LEX_SUB %prec LEX_UNARY
        {
            $$.prgsize = CONTEXT->program->size();
            $$.val.setInteger(0); //compile as (0 - expression)
            CODE_VAL(VAL, $$.val);
        }
    expression
        {
            CODE(SUB);
            $$.prgsize = $2.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | LEX_ADD %prec LEX_UNARY
        {
            $$.prgsize = CONTEXT->program->size();
            $$.val.setInteger(0); //compile as (0 + expression)
            CODE_VAL(VAL, $$.val);
        }
    expression
        {
            CODE(ADD);
            $$.prgsize = $2.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | LEX_DICT dictionary_item
        {
            $$.prgsize = $2.prgsize;
            // this compilation is always optimal
        }
    | LEX_DICT_INDIRECT expression
        {
            CODE(DICT); //indirect dictionary lookup
            $$.prgsize = $2.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }
    | LEX_VAR variable_identifier
        {
            // following code cannot be optimized
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            // generate code for variable
            codeForVariable(context, $$, $2);
        }
    | value_literal
        {
            // following code is always optimal
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            CODE_VAL(VAL, $1.val);
        }
    | case
        {
            $$.prgsize = $1.prgsize; //start of expr prog
        }
    | defined
        {
            $$.prgsize = $1.prgsize; //start of expr prog
        }
    | exist
        {
            $$.prgsize = $1.prgsize; //start of expr prog
        }
    | function
        {
            $$.prgsize = $1.prgsize; //start of expr prog
        }

    // expression error handling
    | LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid "
                        "sub-expression (in parentheses)");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_R_PAREN
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.prgsize = $1.prgsize; //start of expr prog
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
        }
    | frag_expression
        {
            $$.prgsize = $1.prgsize; //start of expr prog
        }
    ;



frag_expression
    : LEX_VAR LEX_VAR frag_expr_chain_start {
            ParserValue_t v;
            v.setString(""); // to native value
            CODE_VAL(REPR, v);
    }
    | LEX_JSONIFY LEX_L_PAREN LEX_VAR LEX_VAR frag_expr_chain_start LEX_R_PAREN {
            ParserValue_t v;
            v.setString("json"); // to json
            CODE_VAL(REPR, v);
            $$.prgsize = $1.prgsize; //start of expr prog
    }
    | LEX_TYPE LEX_L_PAREN LEX_VAR LEX_VAR frag_expr_chain_start LEX_R_PAREN {
            ParserValue_t v;
            v.setString("type"); // return type
            CODE_VAL(REPR, v);
            $$.prgsize = $1.prgsize; //start of expr prog
    }
    | LEX_COUNT LEX_L_PAREN LEX_VAR LEX_VAR frag_expr_chain_start LEX_R_PAREN {
            ParserValue_t v;
            v.setString("count"); // return count
            CODE_VAL(REPR, v);
            $$.prgsize = $1.prgsize; //start of expr prog
    }
    ;

frag_expr_chain_start
    : LEX_SELECTOR {
            ParserValue_t v;
            v.setString("@(root)"); // magic value
            CODE_VAL(GETATTR, v);
        } frag_expr_chain {
    }
    | {
            ParserValue_t v;
            v.setString("@(this)"); // magic value
            CODE_VAL(GETATTR, v);
        }
    frag_expr_chain {
    }
    ;

frag_expr_chain
    : frag_expr {
        $$.prgsize = $1.prgsize; //start of expr prog
    }
    | frag_expr_chain LEX_SELECTOR frag_expr {
        $$.prgsize = $1.prgsize; //start of expr prog
    }
    ;

frag_expr
    : frag_id {
        $$.prgsize = $1.prgsize; //start of expr prog
    }
    | frag_id frag_index_chain {
        $$.prgsize = $1.prgsize; //start of expr prog
    }
    ;

frag_index_chain
    : LEX_L_BRACKET expression LEX_R_BRACKET {
        CODE(AT);
    }
    | frag_index_chain LEX_L_BRACKET expression LEX_R_BRACKET {
        CODE(AT);
    }
    ;


frag_id
    : identifier {
        CODE_VAL(GETATTR, $1.val);
    }
    ;

dictionary_item:
    identifier
        {
            // find item in dictionary and code it as val
            // lookup lang dict first, param dict then else use identifier
            const string *item;
            item = CONTEXT->langDictionary->lookup($1.val.stringValue);
            if (item == 0)
                item = CONTEXT->paramDictionary->lookup($1.val.stringValue);
            if (item == 0) {
        		ERR(ERROR, $1.pos, "Cannot find '" + $1.val.stringValue
        		    + "' dictionary item");
            }
            // generate code
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            $$.val.setString(*item);
            CODE_VAL(VAL, $$.val);
        }
    ;


value_literal:
    string_literal
        {
            $$.val = $1.val;
        }
    | LEX_INT
        {
            $$.val = $1.val;
        }
    | LEX_REAL
        {
            $$.val = $1.val;
        }
    ;


string_literal:
    LEX_STRING
        {
            $$.val = $1.val;
        }
    | LEX_STRING string_literal
        {
            $$.val = $1.val;
            $$.val.stringValue += $2.val.stringValue;
        }
    ;


voluntary_dollar_before_var:
    LEX_VAR
    | //empty
        {
#if 0
            ERR(DEBUG, CONTEXT->position, "Variable identifier should "
                    "start with '$'. Leaving it out is obsolete syntax.");
#endif
        }
    ;


variable_identifier:
    identifier
        {
            // variable in local fragment context
            $$.id = CONTEXT->fragContext.back(); //frag context
            $$.id.push_back($1.val.stringValue); //name
            $$.pos = $1.pos; //var position

            // rebuild identifier
            buildIdentifier($$);
        }
    | identifier dot_variable
        {
            // partitialy qualified variable
            // check for special identifier '_this'
            if ($1.val.stringValue == "_this") {
                // supply actual fragment context
                $$.id = CONTEXT->fragContext.back();
                // add more identifier qualifiers and ident itself
                $$.id.insert($$.id.end(), $2.id.begin(), $2.id.end());
            } else {
                // try to find name on fragment stack--
                // sequence of following fragment names must match
                $$.id.erase($$.id.begin(), $$.id.end()); //clear
                const ParserContext_t::FragmentContext_t
                        &fc = CONTEXT->fragContext.back();
                ParserContext_t::IdentifierName_t::const_reverse_iterator
                        ri;
                for (ri = fc.name.rbegin(); ri != fc.name.rend(); ++ri)
                    if (*ri == $1.val.stringValue)
                        break; //found fragment of specified name
                // if found
                int err = 0;
                if (ri == fc.name.rend()) {
                    err = 1; //not found
                } else {
                    // check all consequenting parts of the identifier
                    // beware! method forward iterator created using base()
                    // from reverse iterator does not refer to the same
                    // element, but to the next element in forward order.
                    ParserContext_t::IdentifierName_t::const_iterator
                            i = ri.base() - 1;
                    LeftValue_t::Identifier_t::const_iterator
                            id = $2.id.begin();
                    ++i; //skip already matching frag name
                    while (i != fc.name.end()
                            && id != $2.id.end()
                            && id != $2.id.end() - 1) {
                        if (*i != *id) {
                            err = 1; //mis-match
                            break;
                        }
                        ++i;
                        ++id;
                    }
                    // if overlaying identifier parts are matching
                    if (!err) {
                        $$.id.insert($$.id.begin(), fc.name.begin(), i);
                        $$.id.push_back(*id);
                    } else
                        err = 1; //mis-match
                }
                // handle error
                if (err) {
                    LeftValue_t::Identifier_t::const_iterator id;
                    string var = $1.val.stringValue;
                    for (id = $2.id.begin(); id != $2.id.end(); ++id)
                        var += "." + *id;
                    ERR(ERROR, $1.pos, "Variable identifier '" + var
                            + "' not found in current context");
                }
            }
            // var position
            $$.pos = $1.pos;

            // create fully-qualified identifier string
            // rebuild identifier
            buildIdentifier($$);
        }
    | dot_variable
        {
            // fully qualified variable name
            $$.id = $1.id;
            $$.pos = $1.pos; //variable position

            // create fully-qualified identifier string
            // rebuild identifier
            buildIdentifier($$);
        }
    ;


dot_variable:
    LEX_SELECTOR identifier
        {
            $$.id.erase($$.id.begin(), $$.id.end());
            $$.id.push_back($2.val.stringValue);
            $$.pos = $1.pos;
        }
    | LEX_SELECTOR identifier dot_variable
        {
            $$.id = $3.id;
            // ignore special identifier '_this'
            // inside the variable qualification
            if (('_' == $2.val.stringValue[0])
                && (($2.val.stringValue == "_this")
                    || ($2.val.stringValue == "_parent"))) {
                ERR(DEBUGING, $2.pos, "Using the special identifier '"
                    + $2.val.stringValue
                    + "' inside the identifier qualification is useless");
            } else {
                $$.id.insert($$.id.begin(), $2.val.stringValue);
                $$.pos = $1.pos;
            }
        }
    ;

identifier
    : LEX_IDENT {
        $$ = $1;
    }
    | LEX_TYPE {
        $$ = $1;
        $$.val.stringValue = "type";
    }
    | LEX_COUNT {
        $$ = $1;
        $$.val.stringValue = "count";
    }
    | LEX_JSONIFY {
        $$ = $1;
        $$.val.stringValue = "jsonify";
    }
    | LEX_EXISTS {
        $$ = $1;
        $$.val.stringValue = "exists";
    }
    ;

case:
    LEX_CASE LEX_L_PAREN expression LEX_COMMA
        {
            CODE(PUSH); //store expr value on program stack
        }
    case_options LEX_R_PAREN
        {
            // update all end-jumps with proper address
            LeftValue_t::AddressList_t::const_iterator i;
            for (i = $6.addr.begin(); i != $6.addr.end(); ++i) {
                (*CONTEXT->program)[*i].value.integerValue =
                        CONTEXT->program->size() - *i - 1;
            }
            // remove previously pushed value
            CODE(POP);
            // try to optimize
            $$.prgsize = $1.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
            // do not optimize (join) print-vals across current prog end-addr
            CONTEXT->lowestValPrintAddress = CONTEXT->program->size();
        }

    // case-operator error handling
    | LEX_CASE LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid condition expression "
                        "in 'case()' operator");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_COMMA case_options LEX_R_PAREN
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.prgsize = $1.prgsize; //start of expr prog
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
        }
    | LEX_CASE LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid 'case()' operator arguments");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_R_PAREN
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.prgsize = $1.prgsize; //start of expr prog
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
        }
    ;


case_options:
    case_values LEX_COLON
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
            CODE(JMPIFNOT); //if not true, jump to the next case
        }
    expression
        {
            $$.prgsize = CONTEXT->program->size(); //save actual addr
        }
    case_more_options
        {
            // propagate end-jump info
            $$.addr = $6.addr;
            // calculate jump offset
            // add +1 for end-jump if some other case option was generated
            (*CONTEXT->program)[$3.prgsize].value.integerValue =
                    $5.prgsize - $3.prgsize - 1
                    + ($5.prgsize != CONTEXT->program->size());
        }
    | LEX_MUL LEX_COLON expression
        {
            // propagate end-jump info
            $$.addr.erase($$.addr.begin(), $$.addr.end()); //no end-jumps
        }
    ;


case_more_options:
    LEX_COMMA
        {
            // insert end-jump
            $$.prgsize = CONTEXT->program->size(); //save end-jump addr
            CODE(JMP); //previous was true, skip else section
        }
    case_options
        {
            $$.addr = $3.addr; //propagate end-jump info
            $$.addr.push_back($2.prgsize);
        }
    | //empty
        {
            // default value, for case the default value is not supplied
            $$.addr.erase($$.addr.begin(), $$.addr.end()); //clear residuals
            $$.addr.push_back(CONTEXT->program->size()); //save end-jump addr
            CODE(JMP); //previous was true, skip else section
            CODE(VAL); //empty value
        }
    ;



// Following code is for parsing literals used in case label. It must
// be literal for fast evaluation but we must allow thing like +number
// and -number.
case_literal:
    value_literal
        {
            // ordinal literal (int, real or string)
            $$.val = $1.val;
        }
    | LEX_ADD LEX_INT
        {
            // + integer
            $$.val = $2.val;
        }
    | LEX_ADD LEX_REAL
        {
            // + real
            $$.val = $2.val;
        }
    | LEX_SUB LEX_INT
        {
            // - integer
            $$.val = -$2.val;
        }
    | LEX_SUB LEX_REAL
        {
            // - real
            $$.val = -$2.val;
        }
    ;

case_values:
    case_literal
        {
            // generate code
            $$.val = ParserValue_t();
            $$.val.integerValue = 0;
            CODE_VAL(STACK, $$.val); //compare to stack top
            CODE_VAL(VAL, $1.val);
            if ($1.val.type == ParserValue_t::TYPE_STRING)
                CODE(STREQ); //compare as strings
            else
                CODE(NUMEQ); //compare as numbers
        }
    | case_literal LEX_COMMA case_values
        {
            // join two case values with OR
            $$.prgsize = CONTEXT->program->size(); //save actual prog size
            CODE(OR); //generate infix code
            // generate code
            $$.val = ParserValue_t();
            $$.val.integerValue = 0;
            CODE_VAL(STACK, $$.val); //compare to stack top
            CODE_VAL(VAL, $1.val);
            if ($1.val.type == ParserValue_t::TYPE_STRING)
                CODE(STREQ); //compare as strings
            else
                CODE(NUMEQ); //compare as numbers
            // calculate OR-jump offset
            (*CONTEXT->program)[$$.prgsize].value.integerValue =
                    CONTEXT->program->size() - $$.prgsize - 1;
        }
    ;


defined:
        LEX_DEFINED LEX_L_PAREN variable_identifier LEX_R_PAREN
        {
            // following code cannot be optimized
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            // clear val and copy variable identifier
            $$.val = ParserValue_t(); //clear
            $$.id = $3.id;
            $$.val.stringValue = $3.val.stringValue;
            if ($$.id.empty()) {
                // bad identifier -- code fake value
                ERR(ERROR, $3.pos, "Invalid identifier "
                        "in 'defined()' operator");
                $$.val.setString("undefined");
                CODE_VAL(VAL, $$.val); //fake value
            } else {
                bool mustBeOpen = false;
                // check for automatic variable
                if ($$.id.back() == "_count") {
                    // remove automatic variable name
                    $$.id.pop_back();
                    // rebuild identifier
                    buildIdentifier($$);
                } else if ($$.id.back() == "_number") {
                    // remove automatic variable name
                    $$.id.pop_back();
                    // rebuild identifier
                    buildIdentifier($$);

                    // this fragment must be open!
                    mustBeOpen = true;
                }

                // resolve existence
                Identifier_t id;
                ParserContext_t::ExistResolution_t er
                    = CONTEXT->exists($3.pos, $$.id, $$.val.stringValue, id, mustBeOpen);

                // generate code
                switch (er) {
                case ParserContext_t::ER_FOUND:
                case ParserContext_t::ER_RUNTIME:
                    // object may be defined
                    CODE_VAL(DEFINED, $$.val);
                    CONTEXT->program->back().identifier = id;
                    break;

                case ParserContext_t::ER_NOT_FOUND:
                    // object cannot be present => always false
                    $$.val.setInteger(false);
                    CODE_VAL(VAL, $$.val);
                    break;
                }
            }
        }

    // defined-operator error handling
    | LEX_DEFINED LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                if (yychar == LEX_VAR)
                    ERR(ERROR, $1.pos, "Variable identifier must not "
                            "start with '$' here");
                ERR(ERROR, $1.pos, "Invalid identifier "
                        "in 'defined()' operator");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_R_PAREN
        {
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            $$.val.setInteger(0);
            CODE_VAL(VAL, $$.val); //fake value
        }
    ;

exist:
    exists_operator LEX_L_PAREN variable_identifier LEX_R_PAREN
        {
            // following code cannot be optimized
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            // clear val and copy variable identifier
            $$.val = ParserValue_t(); //clear
            $$.id = $3.id;
            $$.val.stringValue = $3.val.stringValue;
            if ($$.id.empty()) {
                // bad identifier -- code fake value
                ERR(ERROR, $3.pos, "Invalid identifier "
                        "in 'exist()' operator");
                $$.val.setString("undefined");
                CODE_VAL(VAL, $$.val); //fake value
            } else {
                bool mustBeOpen = false;
                // check for automatic variable
                if ($$.id.back() == "_count") {
                    // remove automatic variable name
                    $$.id.pop_back();
                    // rebuild identifier
                    buildIdentifier($$);
                } else if ($$.id.back() == "_number") {
                    // remove automatic variable name
                    $$.id.pop_back();
                    // rebuild identifier
                    buildIdentifier($$);

                    // this fragment must be open!
                    mustBeOpen = true;
                }

                // resolve existence
                Identifier_t id;
                ParserContext_t::ExistResolution_t er
                    = CONTEXT->exists($3.pos, $$.id, $$.val.stringValue,
                                      id, mustBeOpen);

                // generate code
                switch (er) {
                case ParserContext_t::ER_FOUND:
                    // object found in template => always true
                    $$.val.setInteger(true);
                    CODE_VAL(VAL, $$.val);

                    break;
                case ParserContext_t::ER_RUNTIME:
                    // object may be present in data => resolution postponed
                    // to runtime
                    CODE_VAL(EXIST, $$.val);
                    CONTEXT->program->back().identifier = id;
                    break;

                case ParserContext_t::ER_NOT_FOUND:
                    // object cannot be present in data => always false
                    $$.val.setInteger(false);
                    CODE_VAL(VAL, $$.val);
                    break;
                }
            }
        }

    // exist-operator error handling
    | exists_operator LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                if (yychar == LEX_VAR)
                    ERR(ERROR, $1.pos, "Variable identifier must not "
                            "start with '$' here");
                ERR(ERROR, $1.pos, "Invalid identifier "
                        "in 'exist()' operator");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_R_PAREN
        {
            $$.prgsize = CONTEXT->program->size(); //start of expr prog
            $$.val.setInteger(0);
            CODE_VAL(VAL, $$.val); //fake value
        }
    ;

exists_operator
    : LEX_EXIST {
        $$ = $1;
    }
    | LEX_EXISTS {
        $$ = $1;
    }
    ;

function_id
    : LEX_IDENT {
        $$ = $1;
    }
    | LEX_UDF_IDENT {
        $$ = $1;
    }
    ;

function:
    function_id LEX_L_PAREN function_arguments LEX_R_PAREN
        {
            // call special code generation for functions
            tengCode_generateFunctionCall(CONTEXT,
                    $1.val.stringValue, //function name
                    $3.val.integerValue); //number of args
            $$.prgsize = $2.prgsize; //start of expr prog
            tengCode_optimizeExpression(CONTEXT, $$.prgsize); //optimize
        }

    // function error handling
    | function_id LEX_L_PAREN error
        {
            if (tengSyntax_lastErrorMessage.length() > 0) {
                printUnexpectedElement(CONTEXT, yychar, yylval);
                ERR(ERROR, $1.pos, "Invalid function '"
                        + $1.val.stringValue + "()' argument(s)");
            }
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    LEX_R_PAREN
        {
            CONTEXT->program->erase(CONTEXT->program->begin() + $1.prgsize,
                    CONTEXT->program->end()); //discard program
            $$.prgsize = $1.prgsize; //start of expr prog
            $$.val.setString("undefined");
            CODE_VAL(VAL, $$.val); //fake value
        }
    ;


function_arguments:
    expression LEX_COMMA function_arguments
        {
            $$.val.integerValue = $3.val.integerValue + 1; //more than 1 args
        }
    | expression
        {
            $$.val.integerValue = 1; //single argument
        }
    | //empty
        {
            $$.val.integerValue = 0; //no args
        }
    ;


// end of grammar
%%



/** Lexical analyzer.
  * Calls preprocessor (lex level #1) and lexical analyzer
  * (lex level #2) for some level 1 tokens.
  * @return 0=EOF, >0=element identificator.
  * @param leftValue Pointer to the token's left-value.
  * @param context Pointer to the parser's control structure. */
static int yylex(YYSTYPE *leftValue, void *context)
{
    // forever loop
    for ( ; ; ) {
        // if lex2 is currently in use
        if (CONTEXT->lex2) {

            // get next L2 token (>0=tok, 0=eof, -1=err)
            ParserValue_t val;
            Error_t::Position_t pos = CONTEXT->lex2Pos;
            int tok = CONTEXT->tengLex2.getElement(
                    leftValue, val,
                    CONTEXT->lex2Pos,
                    CONTEXT->program->getErrors());

            if (tok > 0) {
                leftValue->val = val;
                leftValue->pos = pos; //elements position
                CONTEXT->position = pos; //actual position in input stream
                leftValue->prgsize = CONTEXT->program->size();
                return tok; //token
            } else if (tok < 0) {
                ERR(ERROR, pos, "Invalid lexical element");
                continue; //ignore bad element
            }
            // that was last token
            CONTEXT->tengLex2.finish();
            CONTEXT->lex2 = 0;
        }

        // test if lex1 stack is empty
        if (CONTEXT->lex1.empty()) {
            leftValue->pos = CONTEXT->lex1Pos; //last known position
            //last known position in input stream
            CONTEXT->position = CONTEXT->lex1Pos;
            leftValue->prgsize = CONTEXT->program->size();
            return 0; //EOF
        }
        // remember lex1 position
        CONTEXT->lex1Pos = CONTEXT->lex1.top()->getPosition();
        // get next L1 token
        Lex1_t::Token_t tok = CONTEXT->lex1.top()->getElement();
        if (tok.type == Lex1_t::TYPE_TENG
                || tok.type == Lex1_t::TYPE_EXPR
                || tok.type == Lex1_t::TYPE_DICT
                || (tok.type == Lex1_t::TYPE_TENG_SHORT && CONTEXT->paramDictionary->isShortTagEnabled()) ) {

            // use lex2
            CONTEXT->tengLex2.init(tok.value);
            CONTEXT->lex2Pos = CONTEXT->lex1Pos; //set starting lex2 pos
            CONTEXT->lex2 = 1;
            continue; //read first lex2-element

        } else if (tok.type == Lex1_t::TYPE_TEXT
            || (tok.type == Lex1_t::TYPE_TENG_SHORT && !CONTEXT->paramDictionary->isShortTagEnabled()) ) {

            // plain text
            leftValue->val.setString(tok.value);
            leftValue->pos = CONTEXT->lex1Pos; //element's position
            //actual position in input stream
            CONTEXT->position = CONTEXT->lex1Pos;
            leftValue->prgsize = CONTEXT->program->size();
            return LEX_TEXT;

        } else if (tok.type == Lex1_t::TYPE_ERROR) {

            // error
            ERR(ERROR, CONTEXT->lex1Pos, tok.value.c_str());
            continue; //ignore bad element
        }

        // EOF from current lex, remove it from stack and try again
        delete CONTEXT->lex1.top();
        CONTEXT->lex1.pop();
        CONTEXT->sourceIndex.pop();

    } //foreverloop
}


/** Error handling function.
  * Function is unusable, because it does not know parser context :-((
  * @return 0=ok.
  * @param msg Error message to show. */
static int yyerror(const char *msg)
{
#if YYDEBUG
    // if debug enabled
    if (yydebug) {
        fprintf(stderr, "\n*** %s ***\n", msg);
    }
#endif
    // use global variable :-((
    tengSyntax_lastErrorMessage = msg;
    return 0;
}


// if enabled debug mode
#if YYDEBUG
/** Print tokens value into file stream.
  * Just for debug purposes.
  * @param fp File to print to.
  * @param element Lexical element number.
  * @param leftValue Token's left-value to print. */
static void yyprint(FILE *fp, int element, const YYSTYPE &leftValue)
{
    fprintf(fp, " '%s', %ld, %f; addr=%d",
            leftValue.val.stringValue.c_str(),
            leftValue.val.integerValue,
            leftValue.val.realValue,
            leftValue.prgsize);
}
#endif

/** Translates token name into teng-syntax-like name
  * @return teng-syntax-like name if token is known
  * @param token Token name. */

static std::string directive(const std::string &token) {
    string directive;
    if (token == "LEX_ENDFORMAT") {
        directive = "<?teng endformat?> directive";
    } else if (token == "LEX_ENDFRAGMENT") {
        directive = "<?teng endfrag?> directive";
    } else if (token == "LEX_ENDIF") {
        directive = "<?teng endif?> directive";
    } else if (token == "LEX_ENDCTYPE") {
        directive = "<?teng endctype?> directive";
    } else {
        directive = token + " token";
    }
    return directive;
}

/** Print info about unexpected element.
  * @param context Actual parser context.
  * @param elem Look-ahead lexical element.
  * @param val Look-ahead element's semantic value. */
static void printUnexpectedElement(ParserContext_t *context,
                                   int element, const YYSTYPE &leftValue)
{
    string msg;
    switch (element) {

        // plain text
        case LEX_TEXT:
            msg = "template's plain text"; break;

        // teng directives
        case LEX_DEBUG:
            msg = "directive '<?teng debug'"; break;
        case LEX_BYTECODE:
            msg = "directive '<?teng bytecode'"; break;
        case LEX_INCLUDE:
            msg = "directive '<?teng include'"; break;
        case LEX_FORMAT:
            msg = "directive '<?teng format'"; break;
        case LEX_ENDFORMAT:
            msg = "directive '<?teng endformat'"; break;
        case LEX_FRAGMENT:
            msg = "directive '<?teng frag'"; break;
        case LEX_ENDFRAGMENT:
            msg = "directive '<?teng endfrag'"; break;
        case LEX_IF:
            msg = "directive '<?teng if'"; break;
        case LEX_ELSEIF:
            msg = "directive '<?teng elseif'"; break;
        case LEX_ELSE:
            msg = "directive '<?teng else'"; break;
        case LEX_ENDIF:
            msg = "directive '<?teng endif'"; break;
        case LEX_SET:
            msg = "directive '<?teng set'"; break;
        case LEX_EXPR:
            msg = "directive '<?teng expr'"; break;
        case LEX_TENG:
            msg = "directive '<?teng'"; break;
        case LEX_END:
            msg = "directive end '?>'"; break;
        case LEX_SHORT_EXPR:
            msg = "directive '${'"; break;
        case LEX_SHORT_DICT:
            msg = "directive '#{'"; break;
        case LEX_SHORT_END:
            msg = "directive end '}'"; break;
        case LEX_CTYPE:
            msg = "directive '<?teng ctype'"; break;
        case LEX_ENDCTYPE:
            msg = "directive '<?teng endctype'"; break;
        case LEX_REPEATFRAG:
            msg = "directive '<?teng repeat'"; break;

        // assignment
        case LEX_ASSIGN:
            msg = "character '='"; break;
        case LEX_COMMA:
            msg = "character ','"; break;

        // conditional expression
        case LEX_COND_EXPR:
            msg = "operator '?'"; break;
        case LEX_COLON:
            msg = "character ':'"; break;

        // comparison operators
        case LEX_EQ:
            msg = "operator '=='"; break;
        case LEX_NE:
            msg = "operator '!='"; break;
        case LEX_STR_EQ:
            msg = "operator '=~'"; break;
        case LEX_STR_NE:
            msg = "operator '!~'"; break;
        case LEX_GE:
            msg = "operator '>='"; break;
        case LEX_LE:
            msg = "operator '<='"; break;
        case LEX_GT:
            msg = "operator '>'"; break;
        case LEX_LT:
            msg = "operator '<'"; break;

        // logic operators
        case LEX_OR:
            msg = "operator '||'"; break;
        case LEX_AND:
            msg = "operator '&&'"; break;

        // bitwise operators
        case LEX_BITOR:
            msg = "operator '|'"; break;
        case LEX_BITXOR:
            msg = "operator '^'"; break;
        case LEX_BITAND:
            msg = "operator '&'"; break;

        // expressions
        case LEX_ADD:
            msg = "operator '+'"; break;
        case LEX_SUB:
            msg = "operator '-'"; break;
        case LEX_CONCAT:
            msg = "operator '++'"; break;
        case LEX_MUL:
            msg = "operator '*'"; break;
        case LEX_DIV:
            msg = "operator '/'"; break;
        case LEX_MOD:
            msg = "operator '%'"; break;
        case LEX_REPEAT:
            msg = "operator '**'"; break;
        case LEX_NOT:
            msg = "operator '!'"; break;
        case LEX_BITNOT:
            msg = "operator '~'"; break;
        case LEX_UNARY: //LEX_UNARY is fake terminal
            msg = "fake lexical element used just for modifying "
                    "precedence of the unary operators. Huh?!?"; break;

        // other keywords/operators
        case LEX_CASE:
            msg = "operator 'case'"; break;
        case LEX_DEFINED:
            msg = "operator 'defined'"; break;
        case LEX_EXIST:
            msg = "operator 'exist'"; break;
        case LEX_JSONIFY:
            msg = "operator 'jsonify'"; break;
        case LEX_TYPE:
            msg = "operator 'type'"; break;
        case LEX_COUNT:
            msg = "operator 'count'"; break;

        // parentheses
        case LEX_L_PAREN:
            msg = "character '('"; break;
        case LEX_R_PAREN:
            msg = "character ')'"; break;

        // identifiers and literals
        case LEX_VAR:
            msg = "character '$'"; break;
        case LEX_DICT:
            msg = "character '#'"; break;
        case LEX_DICT_INDIRECT:
            msg = "character '@'"; break;

        case LEX_SELECTOR:
            msg = "character '.'"; break;
        case LEX_UDF_IDENT:
            msg = "udf identifier '" + leftValue.val.stringValue + "'"; break;
        case LEX_IDENT:
            msg = "identifier '" + leftValue.val.stringValue + "'"; break;
        case LEX_STRING:
            msg = "string literal '" + leftValue.val.stringValue + "'"; break;
        case LEX_INT:
            msg = "integer literal '" + leftValue.val.stringValue + "'"; break;
        case LEX_REAL:
            msg = "real number literal '"
                    + leftValue.val.stringValue + "'"; break;

        // end of file
        case 0:
            msg = "end of input file while looking for " +
                directive(tengSyntax_lastErrorMessage.substr
                          (tengSyntax_lastErrorMessage.rfind(' ') + 1));
            break;

        default:
            msg = "unknown lexical element. Huh?!?"; break;
    }
    // log error
    ERR(ERROR, leftValue.pos, "Unexpected " + msg);
}

} // namespace Teng
