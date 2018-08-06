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
 * 2018-06-07  (burlog)
 *             Cleaned.
 */

 /********************************************** PROLOGUE: C/C++ declaration. */
%{

#include "tengsemantic.h"

 // I don't know better eay how to access look-ahead symbol
#define look_ahead_symbol yyla.value

%}

 /************************* BISON DECLARATIONS: options that modifies parser. */


// enable debugging
%define parse.trace true

// add forward declaration to tengsyntax.hh
%code requires {#include "tengyystype.h"}
%code requires {namespace Teng {namespace Parser {struct Context_t;}}}
%code {using I = Teng::Instruction_t;}
%code provides {namespace Teng {namespace Parser {using LEX2 = Teng::Parser::parser::token::yytokentype;}}}

// use C++ parser (is always pure parser)
%skeleton "lalr1.cc"

// all callbacks takes context
%param {Teng::Parser::Context_t *ctx}

// close whole parser into Teng::Parser namespace
%define api.namespace {Teng::Parser}

// we want to have all lexical symbols in tengsyntax.hh
%defines

// the type of the lexical symbol
// %define api.value.type {Teng::Parser::Symbol_t}
// %union {
//     Symbol_t symbol;
// }

%define api.value.type variant

%type <Symbol_t> TEXT TENG teng_fragment_open variable ENDFRAGMENT local_variable
               absolute_variable relative_variable absolute_variable_path_root
               absolute_variable_path_this absolute_variable_path_parent
               identifier BUILTIN_THIS BUILTIN_PARENT absolute_variable_middle
               SELECTOR identifier_relative TYPE COUNT JSONIFY EXISTS BUILTIN_COUNT
               BUILTIN_INDEX BUILTIN_FIRST BUILTIN_LAST BUILTIN_INNER

 /******************************************* BISON DECLARATIONS: lex tokens. */

// plain text
%token TEXT

// teng directives
%token DEBUG_FRAG BYTECODE_FRAG INCLUDE FORMAT ENDFORMAT FRAGMENT ENDFRAGMENT
       IF ELSEIF ELSE ENDIF SET EXPR TENG END SHORT_EXPR SHORT_DICT SHORT_END
       CTYPE ENDCTYPE

// assignment
%token ASSIGN COMMA

// conditional expression
%right COND_EXPR COLON

// logic operators (must be right-associative for proper code-generation)
%right OR AND

// bitwise operators
%left BITOR BITXOR BITAND

// comparison operators
%left EQ NE STR_EQ STR_NE GE LE GT LT

// expressions
%left ADD SUB CONCAT
%left MUL DIV MOD REPEAT
%precedence NOT BITNOT

// UNARY is fake terminal
%precedence UNARY

// other keywords/operators
%token CASE DEFINED ISEMPTY EXISTS JSONIFY TYPE COUNT

// parentheses
%token L_PAREN R_PAREN
%token L_BRACKET R_BRACKET

// identifiers and literals
%token BUILTIN_FIRST BUILTIN_INNER BUILTIN_LAST BUILTIN_INDEX BUILTIN_COUNT
       BUILTIN_THIS BUILTIN_PARENT VAR DICT DICT_INDIRECT
       SELECTOR UDF_IDENT IDENT STRING INT REAL

// invalid lexical token
%token INVALID

// start symbol
%start start

// There is one shift/reduce conflict:
//
// start -> error
// teng_format -> error lex_endformat lex_end
// ...
//
// which I don't know how to resolve.
//
// This declaration mutes warning caused by this conflict.
//%expect 1

 /*********************************************** GRAMMAR RULES: teng syntax. */
%%


start
    : template {generateHalt(ctx);}
    | error {replaceCode(ctx, I::HALT, look_ahead_symbol);}
    ;


template
    : template TEXT {generatePrint(ctx, $TEXT);}
    | template teng_directive
    | %empty
    ;


teng_directive
    : teng_unknown  // done
//     | teng_debug    // skip
//     | teng_bytecode // skip
//     | teng_include  // done
//     | teng_format   // done
    | teng_fragment // done
//     | teng_if
//     | teng_set
//     | teng_expr
//     | teng_dict
//     | teng_ctype
    ;


 /********************************************************* RULES: directives */


no_options
//     : options {if (!$options.opt.empty()) logWarning(ctx, $options.pos, "This directive does not accept any option(s)");}
    : error {syntaxError(ctx, look_ahead_symbol, "Syntax error inside <?teng ...?> directive");}
    ;


// options
//     : identifier ASSIGN STRING options[parsed_options] {$$.opt = std::move($parsed_options.opt); $$.opt.emplace($identifier.val.str(), $STRING.val.str()); $$.pos = $identifier.pos;}
//     | %empty {$$.opt.clear();}
//     ;


teng_unknown
    : TENG error END {logError(ctx, $TENG.pos, "Unknown directive: " + $TENG.str());}
    ;


// teng_debug
//     : DEBUG_FRAG no_options END {generateCode(ctx, I::DEBUG_FRAG);}
//     ;
//
//
// teng_bytecode
//     : BYTECODE_FRAG no_options END {generateCode(ctx, I::BYTECODE_FRAG);}
//     ;
//
//
// teng_include
//     : INCLUDE options END {includeFile(ctx, $INCLUDE, $options);}
//     | INCLUDE error {syntaxError(ctx, $error, "Invalid <?teng include ...?> directive");} END
//     ;
//
//
// teng_format
//     : FORMAT options END {openFormat(ctx, $$, $FORMAT, $options);}
//       template ENDFORMAT no_options END {closeFormat(ctx, $ENDFORMAT);}
//     | FORMAT error {syntaxError(ctx, $error, "Invalid <?teng format ...?> directive");}
//       END template ENDFORMAT no_options END
//     | FORMAT error {syntaxError(ctx, $error, "Syntax error in teng format block; discarding it");}
//       ENDFORMAT no_options END {eraseCodeFrom(ctx, $FORMAT.prgsize);}
//     | error {if (yyn) syntaxError(ctx, $error, "Misplaced <?teng endformat?> directive");}
//       ENDFORMAT no_options END {eraseCodeFrom(ctx, $ENDFORMAT.prgsize);}
//     ;


teng_fragment_open
    : FRAGMENT variable END {openFrag(ctx, $$, $variable);
   {
        std::cerr << ">>> " << $variable.pos << std::endl;
        std::cerr << ">>> " << $variable.name() << std::endl;
    } }
    ;


teng_fragment_close
    : ENDFRAGMENT no_options END {closeFrag(ctx, $ENDFRAGMENT);}
    ;


teng_fragment
    : teng_fragment_open template teng_fragment_close {
        std::cerr << ">>> " << $teng_fragment_open.pos << std::endl;
        std::cerr << ">>> " << $teng_fragment_open.name() << std::endl;
    }
//    | FRAGMENT error {syntaxError(ctx, $error, "Invalid <?teng frag ...?> directive; discarding it");}
//      END template ENDFRAGMENT no_options END {eraseCodeFrom(ctx, $FRAGMENT.prgsize);}
//    | FRAGMENT variable END error {syntaxError(ctx, $error, "Syntax error in teng fragment block; discarding it");}
//      ENDFRAGMENT no_options END {eraseCodeFrom(ctx, $FRAGMENT.prgsize);}
//    | error {syntaxError(ctx, $error, "Misplaced <?teng endfrag?> directive");}
//      ENDFRAGMENT no_options END {eraseCodeFrom(ctx, $ENDFRAGMENT.prgsize);}
    ;


// teng_set
//     : SET setting_variable ASSIGN expression END {setVariable(ctx, $$, $SET, $setting_variable);}
//     | SET invalid_set_expression END {eraseCodeFrom(ctx, $SET.prgsize);}
//     ;
//
//
// invalid_set_expression
//     : setting_variable ASSIGN error {syntaxError(ctx, $error, "Invalid expression in <?teng set ...?> directive; variable '" + $setting_variable.val.str() + "' will not be set");}
//     | error {syntaxError(ctx, $error, "Invalid variable identifier in <?teng set ...?> directive");} ASSIGN expression
//     | error {syntaxError(ctx, $error, "Invalid <?teng set ...?> directive");}
//     ;
//
//
// teng_if
//     : IF expression END {$$.prgsize = generateCode(ctx, I::JMPIFNOT);}
//       template {$$.prgsize = ctx->program->size();}
//       teng_else ENDIF no_options END {buildIfEndJump(ctx, $4, $6, $7);}
//     | IF error {syntaxError(ctx, $error, "Error in condition expression " "in <?teng if ...?> directive");}
//       END {eraseCodeFrom(ctx, $IF.prgsize);}
//       template {$$.prgsize = ctx->program->size();}
//       teng_else ENDIF no_options END {eraseCodeFrom(ctx, $7.prgsize);}
//     | IF error {syntaxError(ctx, $error, "Syntax error in teng conditional block; discarding it");}
//       ENDIF no_options END {eraseCodeFrom(ctx, $IF.prgsize);}
//     | error[first_error] {syntaxError(ctx, $first_error, "Misplaced <?teng elseif ...?> directive");}
//       ELSEIF error[second_error] END {eraseCodeFrom(ctx, $second_error.prgsize);}
//     | error {syntaxError(ctx, $error, "Misplaced <?teng else?> directive");}
//       ELSE no_options END {eraseCodeFrom(ctx, $error.prgsize);}
//     | error {syntaxError(ctx, $error, "Misplaced <?teng endif?> directive");}
//       ENDIF no_options END {eraseCodeFrom(ctx, $error.prgsize);}
//     ;
//
//
// teng_else
//     : ELSE no_options END {$$.addr = {generateCode(ctx, I::JMP)};}
//       template {$$.addr = $END.addr;}
//     | ELSEIF {$$.prgsize = generateCode(ctx, I::JMP);}
//       expression END {$$.prgsize = generateCode(ctx, I::JMPIFNOT);}
//       template {$$.prgsize = ctx->program->size();}
//       teng_else {buildElseEndJump(ctx, $$, $2, $5, $7, $8);}
//     | ELSEIF
//       error {syntaxError(ctx, $error, "Error in condition expression in <?teng elseif ...?> directive");}
//       END {eraseCodeFrom(ctx, $ELSEIF.prgsize);}
//       template {$$.prgsize = ctx->program->size();}
//       teng_else {eraseCodeFrom(ctx, $7.prgsize);}
//     | %empty {$$.addr.clear();}
//     ;
//
//
// teng_expr
//     : EXPR expression END {generatePrint(ctx);}
//     | SHORT_EXPR expression SHORT_END {generatePrint(ctx);}
//     | EXPR error {syntaxError(ctx, $error, "Invalid expression in <?teng expr ...?> directive");}
//       END {generateUndefined(ctx, $$, $EXPR); generatePrint(ctx);}
//     | SHORT_EXPR error {syntaxError(ctx, $error, "Invalid expression in ${...} statement");}
//       SHORT_END {generateUndefined(ctx, $$, $SHORT_EXPR); generatePrint(ctx);}
//     ;
//
//
// teng_dict
//     : SHORT_DICT dictionary_item SHORT_END {generatePrint(ctx);}
//     | SHORT_DICT error {syntaxError(ctx, $error, "Invalid dictionary item in #{...} statement");}
//       SHORT_END {generateUndefined(ctx, $$, $SHORT_DICT); generatePrint(ctx);}
//     ;
//
//
// teng_ctype
//     : CTYPE STRING END {$$.val = openCType(ctx, $STRING);}
//       template ENDCTYPE no_options END {closeCType(ctx, $ENDCTYPE);}
//     | CTYPE error {syntaxError(ctx, $error, "Invalid <?teng ctype ...?> directive");}
//       END template ENDCTYPE no_options END
//     | CTYPE error {syntaxError(ctx, $error, "Syntax error in teng ctype block; discarding it");}
//       ENDCTYPE no_options END {eraseCodeFrom(ctx, $CTYPE.prgsize);}
//     | error {syntaxError(ctx, $error, "Misplaced <?teng endctype?> directive");}
//       ENDCTYPE no_options END {eraseCodeFrom(ctx, $ENDCTYPE.prgsize);}
//     ;
//
//
//  /********************************************************* RULES: expression */
//
//
// expression
//     : unary_expression {$$ = $unary_expression;}
//     | binary_expression {$$ = $binary_expression;}
//     | ternary_expression {$$ = $ternary_expression;}
//     ;
//
//
// unary_expression
//     : L_PAREN expression R_PAREN {$$.prgsize = $unary_expression.prgsize;}
//     | L_PAREN error {syntaxError(ctx, $error, "Invalid sub-expression (in parentheses)");} R_PAREN {generateUndefined(ctx, $$, $L_PAREN);}
//     | NOT expression {$$.prgsize = generateExpression(ctx, $NOT.prgsize, I::NOT);}
//     | BITNOT expression {$$.prgsize = generateExpression(ctx, $BITNOT.prgsize, I::BITNOT);}
//     | SUB %prec UNARY {$$.val = 0; $$.prgsize = generateCode(ctx, I::VAL, $$.val);}
//       expression {$$.prgsize = generateExpression(ctx, $expression.prgsize, I::SUB);}
//     | ADD %prec UNARY {$$.val = 0; $$.prgsize = generateCode(ctx, I::VAL, $$.val);}
//       expression {$$.prgsize = generateExpression(ctx, $expression.prgsize, I::ADD);}
//     | DICT dictionary_item {$$.prgsize = $dictionary_item.prgsize;}
//     | DICT_INDIRECT unary_expression[expression] {$$.prgsize = generateExpression(ctx, $expression.prgsize, I::DICT);}
//     | VAR variable {$$.prgsize = ctx->program->size(); codeForVariable(ctx, $$, $variable);}
//     | variable {$$.prgsize = ctx->program->size(); codeForVariable(ctx, $$, $variable);}
//     | runtime_variable {generateRepr(ctx); $$.prgsize = ctx->program->size();}
//     | value_literal {$$.prgsize = generateCode(ctx, I::VAL, $value_literal.val);}
//     | function_expression {$$.prgsize = $function_expression.prgsize;}
//     | query_expression {$$.prgsize = $query_expression.prgsize;}
//     | case_expression {$$.prgsize = $case_expression.prgsize;}
//     ;
//
//
// binary_expression
//     : expression[lhs] EQ expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMEQ);}
//     | expression[lhs] NE expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMEQ, true);}
//     | expression[lhs] GE expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMGE);}
//     | expression[lhs] LE expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMGT, true);}
//     | expression[lhs] GT expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMGT);}
//     | expression[lhs] LT expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::NUMGE, true);}
//     | expression[lhs] STR_EQ expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::STREQ);}
//     | expression[lhs] STR_NE expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::STREQ, true);}
//     | expression[lhs] OR {$$.prgsize = generateCode(ctx, I::OR);}
//       expression[rhs] {$$.prgsize = finalizeBinOp(ctx, $lhs, $rhs);}
//     | expression[lhs] AND {$$.prgsize = generateCode(ctx, I::AND);}
//       expression[rhs] {$$.prgsize = finalizeBinOp(ctx, $lhs, $rhs);}
//     | expression[lhs] BITOR expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::BITOR);}
//     | expression[lhs] BITXOR expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::BITXOR);}
//     | expression[lhs] BITAND expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::BITAND);}
//     | expression[lhs] ADD expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::ADD);}
//     | expression[lhs] SUB expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::SUB);}
//     | expression[lhs] CONCAT expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::CONCAT);}
//     | expression[lhs] MUL expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::MUL);}
//     | expression[lhs] DIV expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::DIV);}
//     | expression[lhs] MOD expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::MOD);}
//     | expression[lhs] REPEAT expression {$$.prgsize = generateExpression(ctx, $lhs.prgsize, I::REPEAT);}
//     ;
//
//
// ternary_expression
//     : expression[cond] COND_EXPR {$$.prgsize = generateCode(ctx, I::JMPIFNOT);}
//       expression[true] COLON {$$.prgsize = buildTernOp(ctx, $[true]);}
//       expression[false] {$$.prgsize = finalizeTernOp(ctx, $[cond], $[false]);}
//     ;
// 
// 
//  /*********************************************** RULES: identifier, literals */
// 
// 
// dictionary_item
//     : identifier {generateDictLookup(ctx, $$, $identifier);}
//     ;
// 
// 
// value_literal
//     : string_literal {$$.val = $string_literal.val;}
//     | INT {$$.val = $INT.val;}
//     | REAL {$$.val = $REAL.val;}
//     ;
// 
// 
// string_literal
//     : STRING {$$.val = $STRING.val;}
//     | STRING string_literal[tail] {$$.val = $STRING.val.str() + $tail.val.str();}
//     ;


identifier_relative
    : IDENT {}
    | BUILTIN_FIRST
    | BUILTIN_INNER
    | BUILTIN_LAST
    | BUILTIN_INDEX
    | BUILTIN_COUNT
    | TYPE
    | COUNT
    | JSONIFY
    | EXISTS
    ;


identifier
    : identifier_relative {}
    | BUILTIN_THIS
    | BUILTIN_PARENT
    ;


 /****************************************************** RULES: teng variable */


variable
    : absolute_variable // {buildAbsVar(ctx, $$, $absolute_variable);}
    | relative_variable // {buildRelVar(ctx, $$, $relative_variable);}
    | local_variable
   {
        std::cerr << ">>> " << $local_variable.pos << std::endl;
        std::cerr << ">>> " << $local_variable.name() << std::endl;
    } // {buildLocVar(ctx, $$, $local_variable);}
    ;


absolute_variable
    : absolute_variable_path_root
    | absolute_variable_path_this
    | absolute_variable_path_parent
    ;


absolute_variable_path_root
    : /* {ctx->ident = {};} */
      absolute_variable_middle // identifier {pushVarName(ctx, $$, $2, $3);}
    ;


absolute_variable_path_this
    : BUILTIN_THIS // {ctx->ident = ctx->frag_ctxs.top().ident;}
      absolute_variable_middle identifier // {pushVarName(ctx, $$, $1, $4);}
    ;


absolute_variable_path_parent
    : BUILTIN_PARENT // {ctx->ident = ctx->frag_ctxs.top().ident; popAbsVarSegment(ctx, $1);}
      absolute_variable_middle identifier // {pushVarName(ctx, $$, $1, $4);}
    ;


absolute_variable_middle
    : absolute_variable_middle identifier_relative SELECTOR // {pushAbsVarSegment(ctx, $2); $$.pos = $1.pos;}
    | absolute_variable_middle BUILTIN_THIS SELECTOR // {logWarning(ctx, $2.pos, "Ignoring useless _this variable path segment"); $$.pos = $1.pos;}
    | absolute_variable_middle BUILTIN_PARENT SELECTOR // {popAbsVarSegment(ctx, $2); $$.pos = $1.pos;}
    | SELECTOR // {$$.pos = $1.pos;}
    ;


relative_variable
    : identifier_relative // {ctx->ident = {std::move($1.val.as_str())};}
      SELECTOR relative_variable_middle identifier // {pushVarName(ctx, $$, $1, $5);}
    ;


relative_variable_middle
    : relative_variable_middle identifier SELECTOR // {pushRelVarSegment(ctx, $2);}
    | %empty
    ;


local_variable
    : identifier
    ;


// setting_variable
//     : VAR variable {$$.val = std::move($2.val); $$.id = std::move($2.id);}
//     | variable {$$.val = std::move($1.val); $$.id = std::move($1.id);}
//     ;
//
//
// runtime_variable
//     : VAR VAR runtime_variable_path
//     ;
//
//
// runtime_variable_path
//     : SELECTOR {generateCode(ctx, I::PUSH_ROOT_FRAG);} runtime_variable_segment
//     | {generateCode(ctx, I::PUSH_THIS_FRAG);} runtime_variable_segment
//     ;
//
//
// runtime_variable_segment
//     : runtime_variable_segment SELECTOR runtime_variable_identifier runtime_variable_subscript
//     | runtime_variable_identifier runtime_variable_subscript
//     ;
//
//
// runtime_variable_subscript
//     : runtime_variable_subscript L_BRACKET expression R_BRACKET {generateCode(ctx, I::PUSH_ATTR_AT);}
//     | %empty
//     ;
//
//
// runtime_variable_identifier
//     : identifier {if ($1.token_id != token::BUILTIN_THIS) generateCode(ctx, I::PUSH_ATTR, $1.val);}
//     ;
//
//
//  /**************************************************** RULES: case expression */
//
//
// case_options
//     : case_values COLON {$$.prgsize = generateCode(ctx, I::JMPIFNOT);}
//       expression {$$.prgsize = ctx->program->size();}
//       case_more_options {finalizeCaseOptionsOp(ctx, $$, $3, $5, $6);}
//     | MUL COLON expression {$$.addr.clear();}
//     ;
//
//
// case_more_options
//     : COMMA {$$.prgsize = generateCode(ctx, I::JMP);}
//       case_options {$$.addr = $3.addr; $$.addr.push_back($2.prgsize);}
//     | %empty {$$.addr = {generateCode(ctx, I::JMP, I::VAL)};}
//     ;
//
//
// // Following code is for parsing literals used in case label. It must
// // be literal for fast evaluation but we must allow thing like +number
// // and -number.
// case_literal
//     : value_literal {$$.val = $1.val;}
//     | ADD INT {$$.val = $2.val;}
//     | ADD REAL {$$.val = $2.val;}
//     | SUB INT {$$.val = -$2.val;}
//     | SUB REAL {$$.val = -$2.val;}
//     ;
//
//
// case_values
//     : case_literal {generateCaseLiteral(ctx, $$, $1);}
//     | case_literal COMMA case_values {generateCaseValues(ctx, $$, $1);}
//     ;
//
//
// case_expression
//     : CASE L_PAREN expression COMMA {generateCode(ctx, I::PUSH);}
//       case_options R_PAREN {finalizeCaseOp(ctx, $$, $1, $6);}
//     | CASE L_PAREN error {syntaxError(ctx, $error, "Invalid condition expression in 'case()' operator");}
//       COMMA case_options R_PAREN {$$.prgsize = generateUndefined(ctx, $$, $1);}
//     | CASE L_PAREN error {syntaxError(ctx, $error, "Invalid 'case()' operator arguments");}
//       R_PAREN {$$.prgsize = generateUndefined(ctx, $$, $1);}
//     ;
//
//
//  /************************************************ RULES: function expression */
//
//
// function_name
//     : IDENT {$$.val = $1.val;}
//     | UDF_IDENT {$$.val = $1.val;}
//     ;
//
//
// function_arguments
//     : expression COMMA function_arguments {$$.val = $3.val.integral() + 1;}
//     | expression {$$.val = 1;}
//     | %empty {$$.val = 0;}
//     ;
//
//
//  // {syntaxError(ctx, yyla.value, "Invalid function '" + $1.val.str() + "()' arg(s)");}
// function_expression
//     : function_name L_PAREN function_arguments R_PAREN {$$.prgsize = generateFunction(ctx, $1, $2, $3);}
//     | function_name L_PAREN error R_PAREN {$$.prgsize = generateUndefined(ctx, $$, $1);}
//     ;
//
//
//  /*************************************************** RULES: query expression */
//
//
// query_argument
//     : variable {$$.val = std::move($1.val); $$.id = std::move($1.id);}
//     | VAR variable {$$.val = "$";}
//     | %empty
//     ;
//
//
// query_name
//     : DEFINED {prepareQueryInstr(ctx, $$, $1, "_defined");}
//     | ISEMPTY {prepareQueryInstr(ctx, $$, $1, "_isempty");}
//     | EXISTS {prepareQueryInstr(ctx, $$, $1, "_exists");}
//     ;
//
//
//  // {syntaxError(ctx, yyla.value, "Invalid query '" + $1.val.str() + "()' arg");}
// query_expression
//     : query_name L_PAREN query_argument R_PAREN {generateQueryInstr(ctx, $$, $1, $3);}
//     | query_name L_PAREN runtime_variable R_PAREN {/* warn */generateRepr(ctx, $1);}
//     | query_name L_PAREN error {generateQueryInstr(ctx, $$, $1, $error);} R_PAREN
//     /* | EXISTS L_PAREN runtime_variable R_PAREN { */
//     /*     generateFunctionCall(ctx, "_exists", 1); */
//     /* } */
//     ;


 /************************************ EPILOGUE: glue between bison and flex. */
%%

