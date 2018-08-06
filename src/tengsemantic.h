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
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

// includes
#include <cstdio>
#include <string>

#include "tengyystype.h"
#include "tengerror.h"
#include "tenginstruction.h"
#include "tengparsercontext.h"
#include "tengparservalue.h"
#include "tengformatter.h"
#include "tengcode.h"
#include "tenglex2.h"
#include "tengaux.h"
#include "tengyylex.h"

// namespace Teng {
//
// #define YYPARSE_PARAM context
// #define YYLEX_PARAM context
//
// // Cause the `yydebug' variable to be defined
// // #define YYDEBUG 1
//
// // verbose errors
// #define YYERROR_VERBOSE
//
// // define size of the parser stack
// // (now, 200 is enough - grammar was fixed)
// //#define YYINITDEPTH 200
// // (default value 200 is too small for complex templates)
// #define YYINITDEPTH 10000
//
// // local function prototypes
// static int yylex(YYSTYPE *leftValue, void *context);
// static int yyerror(void *context, const char *msg);
//
// // errlog function
// static void printUnexpectedElement(ParserContext_t *context,
//         int element, const YYSTYPE &leftValue);
//
//
// // if enabled debug mode
// #if YYDEBUG
// // print element's semantic value
// #define YYPRINT(FILE,YYCHAR,YYLVAL) yyprint(FILE,YYCHAR,YYLVAL)
// static void yyprint(FILE *fp, int element, const YYSTYPE &leftValue);
// #endif
//
//
// // last error message (syntax or parse error)
// // global variable -- ugly hack :-((
// std::string tengSyntax_lastErrorMessage;
//
//
// // max depth for templates included templates (looping protection)
// #define MAX_INCLUDE_DEPTH 10
//
// // macro for casting 'context' to the proper type
// #define CONTEXT            (reinterpret_cast<ParserContext_t *> (context))
//
// // macro for appending error class in actual context
// // error levels are: DEBUG, WARNING, ERROR, FATAL
// #define ERR(level, pos, msg)    do { \
//                         (reinterpret_cast<ParserContext_t *> \
//                         (context))->program->getErrors().logError( \
//                         (Error_t::LL_##level), (pos), (msg)); \
//                         } while (0)
//
// // macros for byte-code generation
// #define CODE(code)        do { \
//                         tengCode_generate( \
//                         (reinterpret_cast<ParserContext_t *> (context)), \
//                         Instruction_t::code); \
//                         } while (0)
//
// #define CODE_VAL(code, val) \
//                         do { \
//                         tengCode_generate( \
//                         (reinterpret_cast<ParserContext_t *> (context)), \
//                         Instruction_t::code, val); \
//                         } while (0)
//
// #define CODE_VAL_VAR(opcode, val) \
//                         do { \
//                         tengCode_generate( \
//                         (reinterpret_cast<ParserContext_t *> (context)), \
//                         opcode, val); \
//                         } while (0)
//
// static inline void buildIdentifier(LeftValue_t &v) {
//     v.val = ParserValue_t(); //clear
//     for (LeftValue_t::Identifier_t::const_iterator i
//              = v.id.begin(); i != v.id.end(); ++i)
//         v.val.stringValue += "." + *i;
// }
//
// namespace {
//     enum VariableName_t {
//         VN_REGULAR,
//         VN_COUNT,
//         VN_NUMBER,
//         VN_FIRST,
//         VN_INNER,
//         VN_LAST
//     };
//
//     struct VariableMapping_t {
//         const char *name;
//         VariableName_t variable;
//         Instruction_t::OpCode_t opcode;
//     };
//
//     VariableMapping_t variableMapping[] = {
//         {
//             "_count",
//             VN_COUNT,
//             Instruction_t::FRAGCNT
//         }, {
//             "_number",
//             VN_NUMBER,
//             Instruction_t::FRAGITR
//         }, {
//             "_first",
//             VN_FIRST,
//             Instruction_t::FRAGFIRST
//         }, {
//             "_inner",
//             VN_INNER,
//             Instruction_t::FRAGINNER
//         }, {
//             "_last",
//             VN_LAST,
//             Instruction_t::FRAGLAST
//         }, {
//             0,
//             VN_REGULAR,
//             Instruction_t::VAR
//         }
//     };
//
//     const VariableMapping_t& findVariableMapping(const std::string &name) {
//         // ignore names not starting with underscore
//         if (name.empty() || (name[0] != '_')) {
//             return *(variableMapping +
//                      (sizeof(variableMapping) / sizeof(VariableMapping_t))
//                      - 1);
//         }
//
//         VariableMapping_t *sv = variableMapping;
//         for ( ; sv->name; ++sv)
//             if (!name.compare(sv->name)) return *sv;
//
//         // not found
//         return *sv;
//     }
// }
//
// #<{(|* @short Generate code for variable.
//  * @param result use $$ as this parameter
//  * @param val value with variable name ($x).
//  |)}>#
// static inline void
// codeForVariable(void *context, LeftValue_t &result, LeftValue_t &val) {
//     // clear val and copy variable identifier
//     result.val = ParserValue_t(); //clear
//     result.id = val.id;
//     result.val.stringValue = val.val.stringValue;
//
//     // error indicator
//     bool found = false;
//
//     if (result.id.size() == 0) {
//         // bad variable identifier -- code fake value
//         ERR(ERROR, val.pos, "Invalid variable identifier "
//             "in ${...} directive");
//     } else {
//         // identifier of variable/fragment
//         Identifier_t id;
//
//         const VariableMapping_t
//             &varMapping(findVariableMapping(result.id.back()));
//
//         switch (varMapping.variable) {
//         case VN_COUNT:
//             {
//                 // remove automatic variable name
//                 result.id.pop_back();
//
//                 // rebuild identifier
//                 buildIdentifier(result);
//
//                 // try to find fragment
//                 ParserContext_t::FragmentResolution_t fr =
//                     CONTEXT->findFragment(&val.pos, result.id,
//                                           result.val.stringValue, id, true);
//                 switch (fr) {
//                 case ParserContext_t::FR_NOT_FOUND:
//                     // no-op -- we have nothing found
//                     break;
//                 case ParserContext_t::FR_FOUND:
//                     // fragment found
//                     found = true;
//
//                     CODE_VAL(FRAGCNT, result.val);
//                     CONTEXT->program->back().identifier = id;
//                     break;
//                 case ParserContext_t::FR_PARENT_FOUND:
//                     // parent fragment found -- this fragment is not open
//                     // and we must have some runtime overhead to find count
//                     // of it
//                     found = true;
//
//                     CODE_VAL(XFRAGCNT, result.val);
//                     CONTEXT->program->back().identifier = id;
//                     break;
//                 }
//             }
//             break;
//
//         case VN_NUMBER:
//         case VN_FIRST:
//         case VN_INNER:
//         case VN_LAST:
//             {
//                 // remove automatic variable name
//                 result.id.pop_back();
//
//                 // rebuild identifier
//                 buildIdentifier(result);
//
//                 // try to find fragment
//                 if (CONTEXT->findFragment(&val.pos, result.id,
//                                           result.val.stringValue, id)) {
//                     // we have found fragment
//                     found = true;
//
//                     // add instruction
//                     CODE_VAL_VAR(varMapping.opcode, result.val);
//                     CONTEXT->program->back().identifier = id;
//                 }
//             }
//             break;
//
//         default:
//             // find fragment for this variable
//             if (CONTEXT->findFragmentForVariable(val.pos, result.id,
//                     result.val.stringValue, id)) {
//                 // variable found
//                 found = true;
//
//                 // generate code for variable
//                 result.val.integerValue = 1; //escape
//                 CODE_VAL(VAR, result.val);
//
//                 // add identifier
//                 CONTEXT->program->back().identifier = id;
//             }
//         }
//     }
//
//     if (!found) {
//         // no such varibale or fragment => generate code for undefined value
//         result.val.setString("undefined");
//         CODE_VAL(VAL, result.val);
//     }
// }

// /** Error handling function.
//   * Function is unusable, because it does not know parser context :-((
//   * @return 0=ok.
//   * @param msg Error message to show. */
// static int yyerror(void *context, const char *msg) {
// #if YYDEBUG
//     // if debug enabled
//     if (yydebug) {
//         fprintf(stderr, "\n*** %s ***\n", msg);
//     }
// #endif
//     // use global variable :-((
//     tengSyntax_lastErrorMessage = msg;
//     return 0;
// }
// 
// 
// // if enabled debug mode
// #if YYDEBUG
// /** Print tokens value into file stream.
//   * Just for debug purposes.
//   * @param fp File to print to.
//   * @param element Lexical element number.
//   * @param leftValue Token's left-value to print. */
// static void yyprint(FILE *fp, int element, const YYSTYPE &leftValue) {
//     fprintf(fp, " '%s', %ld, %f; addr=%d",
//             leftValue.val.stringValue.c_str(),
//             leftValue.val.integerValue,
//             leftValue.val.realValue,
//             leftValue.prgsize);
// }
// #endif
// 
// /** Translates token name into teng-syntax-like name
//   * @return teng-syntax-like name if token is known
//   * @param token Token name. */
// 
// static std::string directive(const std::string &token) {
//     std::string directive;
//     if (token == "LEX_ENDFORMAT") {
//         directive = "<?teng endformat?> directive";
//     } else if (token == "LEX_ENDFRAGMENT") {
//         directive = "<?teng endfrag?> directive";
//     } else if (token == "LEX_ENDIF") {
//         directive = "<?teng endif?> directive";
//     } else if (token == "LEX_ENDCTYPE") {
//         directive = "<?teng endctype?> directive";
//     } else {
//         directive = token + " token";
//     }
//     return directive;
// }
// 
// /** Print info about unexpected element.
//   * @param context Actual parser context.
//   * @param elem Look-ahead lexical element.
//   * @param val Look-ahead element's semantic value. */
// static void printUnexpectedElement(ParserContext_t *context,
//                                    int element, const YYSTYPE &leftValue)
// {
//     std::string msg;
//     switch (element) {
// 
//         // plain text
//         case LEX_TEXT:
//             msg = "template's plain text"; break;
// 
//         // teng directives
//         case LEX_DEBUG:
//             msg = "directive '<?teng debug'"; break;
//         case LEX_BYTECODE:
//             msg = "directive '<?teng bytecode'"; break;
//         case LEX_INCLUDE:
//             msg = "directive '<?teng include'"; break;
//         case LEX_FORMAT:
//             msg = "directive '<?teng format'"; break;
//         case LEX_ENDFORMAT:
//             msg = "directive '<?teng endformat'"; break;
//         case LEX_FRAGMENT:
//             msg = "directive '<?teng frag'"; break;
//         case LEX_ENDFRAGMENT:
//             msg = "directive '<?teng endfrag'"; break;
//         case LEX_IF:
//             msg = "directive '<?teng if'"; break;
//         case LEX_ELSEIF:
//             msg = "directive '<?teng elseif'"; break;
//         case LEX_ELSE:
//             msg = "directive '<?teng else'"; break;
//         case LEX_ENDIF:
//             msg = "directive '<?teng endif'"; break;
//         case LEX_SET:
//             msg = "directive '<?teng set'"; break;
//         case LEX_EXPR:
//             msg = "directive '<?teng expr'"; break;
//         case LEX_TENG:
//             msg = "directive '<?teng'"; break;
//         case LEX_END:
//             msg = "directive end '?>'"; break;
//         case LEX_SHORT_EXPR:
//             msg = "directive '${'"; break;
//         case LEX_SHORT_DICT:
//             msg = "directive '#{'"; break;
//         case LEX_SHORT_END:
//             msg = "directive end '}'"; break;
//         case LEX_CTYPE:
//             msg = "directive '<?teng ctype'"; break;
//         case LEX_ENDCTYPE:
//             msg = "directive '<?teng endctype'"; break;
//         case LEX_REPEATFRAG:
//             msg = "directive '<?teng repeat'"; break;
// 
//         // assignment
//         case LEX_ASSIGN:
//             msg = "character '='"; break;
//         case LEX_COMMA:
//             msg = "character ','"; break;
// 
//         // conditional expression
//         case LEX_COND_EXPR:
//             msg = "operator '?'"; break;
//         case LEX_COLON:
//             msg = "character ':'"; break;
// 
//         // comparison operators
//         case LEX_EQ:
//             msg = "operator '=='"; break;
//         case LEX_NE:
//             msg = "operator '!='"; break;
//         case LEX_STR_EQ:
//             msg = "operator '=~'"; break;
//         case LEX_STR_NE:
//             msg = "operator '!~'"; break;
//         case LEX_GE:
//             msg = "operator '>='"; break;
//         case LEX_LE:
//             msg = "operator '<='"; break;
//         case LEX_GT:
//             msg = "operator '>'"; break;
//         case LEX_LT:
//             msg = "operator '<'"; break;
// 
//         // logic operators
//         case LEX_OR:
//             msg = "operator '||'"; break;
//         case LEX_AND:
//             msg = "operator '&&'"; break;
// 
//         // bitwise operators
//         case LEX_BITOR:
//             msg = "operator '|'"; break;
//         case LEX_BITXOR:
//             msg = "operator '^'"; break;
//         case LEX_BITAND:
//             msg = "operator '&'"; break;
// 
//         // expressions
//         case LEX_ADD:
//             msg = "operator '+'"; break;
//         case LEX_SUB:
//             msg = "operator '-'"; break;
//         case LEX_CONCAT:
//             msg = "operator '++'"; break;
//         case LEX_MUL:
//             msg = "operator '*'"; break;
//         case LEX_DIV:
//             msg = "operator '/'"; break;
//         case LEX_MOD:
//             msg = "operator '%'"; break;
//         case LEX_REPEAT:
//             msg = "operator '**'"; break;
//         case LEX_NOT:
//             msg = "operator '!'"; break;
//         case LEX_BITNOT:
//             msg = "operator '~'"; break;
//         case LEX_UNARY: //LEX_UNARY is fake terminal
//             msg = "fake lexical element used just for modifying "
//                     "precedence of the unary operators. Huh?!?"; break;
// 
//         // other keywords/operators
//         case LEX_CASE:
//             msg = "operator 'case'"; break;
//         case LEX_DEFINED:
//             msg = "operator 'defined'"; break;
//         case LEX_ISEMPTY:
//             msg = "operator 'isempty'"; break;
//         case LEX_EXISTS:
//             msg = "operator 'exists'"; break;
//         case LEX_JSONIFY:
//             msg = "operator 'jsonify'"; break;
//         case LEX_TYPE:
//             msg = "operator 'type'"; break;
//         case LEX_COUNT:
//             msg = "operator 'count'"; break;
// 
//         // parentheses
//         case LEX_L_PAREN:
//             msg = "character '('"; break;
//         case LEX_R_PAREN:
//             msg = "character ')'"; break;
// 
//         // identifiers and literals
//         case LEX_VAR:
//             msg = "character '$'"; break;
//         case LEX_DICT:
//             msg = "character '#'"; break;
//         case LEX_DICT_INDIRECT:
//             msg = "character '@'"; break;
// 
//         case LEX_SELECTOR:
//             msg = "character '.'"; break;
//         case LEX_UDF_IDENT:
//             msg = "udf identifier '" + leftValue.val.stringValue + "'"; break;
//         case LEX_IDENT:
//             msg = "identifier '" + leftValue.val.stringValue + "'"; break;
//         case LEX_STRING:
//             msg = "string literal '" + leftValue.val.stringValue + "'"; break;
//         case LEX_INT:
//             msg = "integer literal '" + leftValue.val.stringValue + "'"; break;
//         case LEX_REAL:
//             msg = "real number literal '"
//                     + leftValue.val.stringValue + "'"; break;
// 
//         // end of file
//         case 0:
//             msg = "end of input file while looking for " +
//                 directive(tengSyntax_lastErrorMessage.substr
//                           (tengSyntax_lastErrorMessage.rfind(' ') + 1));
//             break;
// 
//         default:
//             msg = "unknown lexical element. Huh?!?"; break;
//     }
//     // log error
//     ERR(ERROR, leftValue.pos, "Unexpected " + msg);
// }
// 
