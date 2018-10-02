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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
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

 // I don't know better way how to access look-ahead symbol
#define YYLA (yyla.value.as<TokenSymbol_t>())
 // Restores the YYLA, the C++ parser does not implement YYBACKUP yet :(
#define YYBACKUP(T, V) {new (&yyla) decltype(yyla)(T, V);}
 // Allow or disalow generating parser debug routines
#define YYDEBUG 1

%}

 /************************* BISON DECLARATIONS: options that modifies parser. */

// enable debugging
%define parse.trace true

// add forward declarations to the begin of tengsyntax.hh
%code requires {
    #include "tengyystype.h"
    #include "tengsemantic.h"
    namespace Teng {
    namespace Parser {
    struct Context_t;
    namespace Impl {
        using VariableSymbol_t = OptionalSymbol_t<Parser::Variable_t>;
        using OptionsSymbol_t = OptionalSymbol_t<Parser::Options_t>;
        using NAryExprSymbol_t = OptionalSymbol_t<Parser::NAryExpr_t>;
        using LiteralSymbol_t = OptionalSymbol_t<Parser::Literal_t>;
        using TokenSymbol_t = OptionalSymbol_t<Parser::Token_t>;
    } // namespace Impl
    } // namespace Parser
    } // namespace Teng
}

// add some declarations at the end of tengsyntax.hh
%code provides {
    namespace Teng {
    namespace Parser {
        using LEX2 = Impl::parser::token::yytokentype;
        using Parser_t = Impl::parser;
    } // namespace Parser
    } // namespace Teng
}

// add forward declaration to the begin of tengsyntax.cc
%code {
    namespace Teng {
    namespace Parser {
    namespace Impl {
        int yylex(parser::semantic_type *variant, Context_t *ctx);
    } // namespace Impl
    } // namespace Parser
    } // namespace Teng
}

// use C++ parser (is always pure parser)
%skeleton "lalr1.cc"

// all callbacks takes context
%param {Teng::Parser::Context_t *ctx}

// close whole parser into Teng::Parser::Impl namespace
%define api.namespace {Teng::Parser::Impl}

// we want to have all lexical symbols in tengsyntax.hh
%defines

// the type of the lexical symbol
// %define api.value.type {Teng::Parser::Symbol_t}
%define api.value.type variant

 /******************************************* BISON DECLARATIONS: lex tokens. */

// plain text
%token <TokenSymbol_t> TEXT

// teng directives
%token <TokenSymbol_t> TENG FRAGMENT ENDFRAGMENT DEBUG_FRAG BYTECODE_FRAG
%token <TokenSymbol_t> INCLUDE IF ELSEIF ELSE ENDIF SET EXPR FORMAT ENDFORMAT
%token <TokenSymbol_t> END SHORT_EXPR SHORT_DICT SHORT_END CTYPE ENDCTYPE

// assignment operator
%token <TokenSymbol_t> ASSIGN

// comma operator
%token <TokenSymbol_t> COMMA

// conditional expression
%right <TokenSymbol_t> COND_EXPR COLON

// logic operators (must be right-associative for proper code-generation)
%right <TokenSymbol_t> OR OR_DIGRAPH
%right <TokenSymbol_t> AND AND_TRIGRAPH

// bitwise operators
%left <TokenSymbol_t> BITOR
%left <TokenSymbol_t> BITXOR
%left <TokenSymbol_t> BITAND

// comparison operators (STR_* are deprecated since teng-3.0)
%left <TokenSymbol_t> EQ NE STR_EQ STR_NE EQ_DIGRAPH NE_DIGRAPH
%left <TokenSymbol_t> GE LE GT LT GE_DIGRAPH LE_DIGRAPH GT_DIGRAPH LT_DIGRAPH

// binary expressions
%left <TokenSymbol_t> PLUS MINUS CONCAT // CONCAT is deprecated since teng-3.0
%left <TokenSymbol_t> MUL DIV MOD REPEAT

%right <TokenSymbol_t> NOT BITNOT

// other keywords/operators
%token <TokenSymbol_t> CASE

// parentheses
%token <TokenSymbol_t> L_PAREN R_PAREN
%token <TokenSymbol_t> L_BRACKET R_BRACKET

// teng variables and dictionary lookups prefixes
%left <TokenSymbol_t> DICT DICT_INDIRECT SELECTOR VAR

// identifiers and literals
%token <TokenSymbol_t> BUILTIN_FIRST BUILTIN_INNER BUILTIN_LAST BUILTIN_INDEX
%token <TokenSymbol_t> BUILTIN_COUNT BUILTIN_THIS BUILTIN_PARENT
%token <TokenSymbol_t> DEFINED ISEMPTY EXISTS JSONIFY TYPE COUNT
%token <TokenSymbol_t> IDENT UDF_IDENT STRING REGEX DEC_INT HEX_INT BIN_INT REAL

// invalid lexical token
%token <TokenSymbol_t> INV

// HIGHEST_PREC is fake terminal (lower in file means higher precedence)
%precedence HIGHEST_PREC

// start symbol
%start start

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <TokenSymbol_t> identifier identifier_relative function_name
%type <TokenSymbol_t> teng_if_prepare teng_elif_prepare

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <LiteralSymbol_t> value_literal string_literal int_literal
%type <LiteralSymbol_t> signed_literal real_literal string_literal_fact

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <VariableSymbol_t> variable local_variable relative_variable set_variable

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <OptionsSymbol_t> options

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <NAryExprSymbol_t> nary_expression case_expression
%type <NAryExprSymbol_t> function_expression query_expression

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <Nil_t> start template teng_directive teng_unknown ignored_options
%type <Nil_t> teng_frag teng_frag_close
%type <Nil_t> teng_expr teng_dict option option_list
%type <Nil_t> teng_debug teng_bytecode teng_include
%type <Nil_t> unary_expression expression_impl lazy_binary_expression
%type <Nil_t> ternary_expression expression binary_expression
%type <Nil_t> absolute_variable
%type <Nil_t> absolute_variable_segment absolute_variable_prefix teng_set
%type <Nil_t> rtvar_path rtvar_segment rtvar_identifier rtvar_index
%type <Nil_t> teng_ctype teng_ctype_close block_content
%type <Nil_t> teng_else teng_elif teng_endif teng_if_stmnt
%type <Nil_t> query_argument_inner template_content teng_if
%type <Nil_t> teng_format teng_format_close teng_if_stmnt_valid
%type <Nil_t> teng_if_stmnt_invalid teng_if_stmnt_disordered case_default_option

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <bool> nullary_expression expr_up_to_end expr_up_to_end_impl

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <Pos_t> runtime_variable teng_format_open teng_frag_open teng_ctype_open

// TODO(burlog): tady je potreba popsat proc je to tento typ
%type <uint32_t> function_arguments case_particular_options case_values
%type <uint32_t> query_argument case_options deprecated_binary_expression

 /************************************************ GRAMMAR RULES: teng syntax */
%%


start
    : template {generate<Halt_t>(ctx);}
    ;


template
    : template template_content {reset_error(ctx);}
    | template error {note_error(ctx, *YYLA);}
    | %empty {reset_error(ctx);}
    ;


block_content
    : block_content template_content {}
    | %empty {}
    ;


template_content
    : TEXT {generate_print(ctx, *$1);}
    | teng_directive {}
    ;


teng_directive
    : teng_unknown
    | teng_debug
    | teng_bytecode
    | teng_include
    | teng_format
    | teng_frag
    | teng_if_stmnt
    | teng_expr
    | teng_dict
    | teng_ctype
    | teng_set
    ;

 /************************************************************* RULES: errors */


 // The error 'up to end' ensures that error recovery stops just before END
 // token.
error_up_to_end
    : error {note_error(ctx, *YYLA);} END {YYBACKUP(LEX2::END, $3);}
    | error {note_error(ctx, *YYLA);} INV {YYBACKUP(LEX2::END, $3);}
    ;


 // The error 'up to end' ensures that error recovery stops just before SHORT_END
 // token.
error_up_to_short_end
    : error {note_error(ctx, *YYLA);} SHORT_END {YYBACKUP(LEX2::SHORT_END, $3);}
    | error {note_error(ctx, *YYLA);} INV {YYBACKUP(LEX2::SHORT_END, $3);}
    ;


 // The error 'up to end' ensures that error recovery stops just before R_PAREN
 // token.
error_up_to_r_paren
    : error {note_error(ctx, *YYLA);} R_PAREN {YYBACKUP(LEX2::R_PAREN, $3);}
    | error {note_error(ctx, *YYLA);} INV {YYBACKUP(LEX2::R_PAREN, $3);}
    ;


 // The error 'up to end' ensures that error recovery stops just before ENDIF
 // token.
error_up_to_end_if
    : error ENDIF {YYBACKUP(LEX2::ENDIF, $2);}
    ;


 /************************************************************ RULES: options */


options
    : {ctx->opts_sym = Options_t(*YYLA);}
      option_list {$$.emplace(std::move(ctx->opts_sym));}
    ;


ignored_options
    : options {ignore_excessive_options(ctx, $1->pos);}
    | %empty {}
    ;


option_list
    : option_list option
    | option
    ;


option
    : identifier ASSIGN value_literal {new_option(ctx, *$1, std::move(*$3));}
    ;


 /************************************************* RULES: general directives */


teng_unknown
    : TENG error_up_to_end END {ignore_unknown_directive(ctx, *$1);}
    ;


teng_debug
    : DEBUG_FRAG ignored_options END {debug_frag(ctx, $1->pos);}
    | DEBUG_FRAG error_up_to_end END {debug_frag(ctx, $1->pos, true);}
    ;


teng_bytecode
    : BYTECODE_FRAG ignored_options END {bytecode_frag(ctx, $1->pos);}
    | BYTECODE_FRAG error_up_to_end END {bytecode_frag(ctx, $1->pos, true);}
    ;


teng_include
    : INCLUDE options END {include_file(ctx, $1->pos, *$2);}
    | INCLUDE END {ignore_include(ctx, *$1, true);}
    | INCLUDE error_up_to_end END {ignore_include(ctx, *$1);}
    ;


 /************************************************** RULES: format directives */


teng_format
    : teng_format_open block_content teng_format_close {}
    | teng_format_open error {close_unclosed_format(ctx, $1, *YYLA); yyerrok;}
    ;


teng_format_open
    : FORMAT options END {open_format(ctx, $1->pos, *$2); $$ = $1->pos;}
    | FORMAT error_up_to_end END {open_inv_format(ctx, $1->pos); $$ = $1->pos;}
    ;


teng_format_close
    : ENDFORMAT ignored_options END {close_format(ctx, $1->pos);}
    | ENDFORMAT error_up_to_end END {close_inv_format(ctx, $1->pos);}
    ;


 /**************************************************** RULES: frag directives */


teng_frag
    : teng_frag_open block_content teng_frag_close {}
    | teng_frag_open error {close_unclosed_frag(ctx, $1, *YYLA); yyerrok;}
    ;


teng_frag_open
    : FRAGMENT variable END {open_frag(ctx, $1->pos, *$2); $$ = $1->pos;}
    | FRAGMENT error_up_to_end END {open_inv_frag(ctx, $1->pos); $$ = $1->pos;}
    ;


teng_frag_close
    : ENDFRAGMENT ignored_options END {close_frag(ctx, $1->pos);}
    | ENDFRAGMENT error_up_to_end END {close_inv_frag(ctx, $1->pos);}
    ;


 /*************************************************** RULES: ctype directives */


teng_ctype
    : teng_ctype_open block_content teng_ctype_close {}
    | teng_ctype_open error {close_unclosed_ctype(ctx, $1, *YYLA); yyerrok;}
    ;


teng_ctype_open
    : CTYPE string_literal END {open_ctype(ctx, $1->pos, *$2); $$ = $1->pos;}
    | CTYPE error_up_to_end END {open_inv_ctype(ctx, $1->pos); $$ = $1->pos;}
    ;


teng_ctype_close
    : ENDCTYPE ignored_options END {close_ctype(ctx, $1->pos);}
    | ENDCTYPE error_up_to_end END {close_inv_ctype(ctx, $1->pos);}
    ;


 /***************************************************** RULES: set directives */


teng_set
    : SET set_variable ASSIGN expr_up_to_end END {set_var(ctx, std::move(*$2));}
    | SET error_up_to_end END {ignore_inv_set(ctx, $1->pos);}
    ;


set_variable
    : variable {$$ = std::move($1);}
    | VAR variable {$$ = std::move($2); obsolete_dollar(ctx, $1->pos);}
    ;


 /**************************************************** RULES: cond directives */


teng_if_stmnt
    : teng_if_stmnt_valid {finalize_if_stmnt(ctx);}
    | teng_if_stmnt_invalid {}
    | teng_if_stmnt_disordered {}
    ;


teng_if_stmnt_valid
    : teng_if block_content teng_endif
    | teng_if block_content teng_else block_content teng_endif
    | teng_if block_content teng_elif_stmnt teng_endif
    | teng_if block_content teng_elif_stmnt teng_else block_content teng_endif
    ;


teng_if_stmnt_invalid
    : teng_if
      error {finalize_inv_if_stmnt(ctx, *YYLA);}
    | teng_if block_content teng_else
      error {finalize_inv_if_stmnt(ctx, *YYLA);}
    | teng_if block_content teng_elif_stmnt teng_else
      error {finalize_inv_if_stmnt(ctx, *YYLA);}
    ;


teng_if_stmnt_disordered
    : teng_if block_content teng_else block_content teng_elif
      error_up_to_end_if teng_endif {discard_if(ctx);}
    | teng_if block_content teng_elif_stmnt teng_else block_content teng_elif
      error_up_to_end_if teng_endif {discard_if(ctx);}
    ;


teng_elif_stmnt
    : teng_elif block_content
    | teng_elif_stmnt teng_elif block_content
    ;


teng_if_prepare
    : IF {prepare_if(ctx, $1->pos); $$ = std::move($1);}
    ;


teng_if
    : teng_if_prepare expr_up_to_end END {generate_if(ctx, *$1, $2);}
    | teng_if_prepare expr_up_to_end INV {generate_if(ctx, *$1, *$3);}
    ;


teng_else
    : ELSE ignored_options END {generate_else(ctx, *$1);}
    | ELSE error_up_to_end END {generate_inv_else(ctx, *$1);}
    ;


teng_elif_prepare
    : ELSEIF {generate_elif(ctx, *$1); $$ = std::move($1);}


teng_elif
    : teng_elif_prepare expr_up_to_end END {generate_if(ctx, *$1, $2);}
    | teng_elif_prepare expr_up_to_end INV {generate_if(ctx, *$1, *$3);}
    ;


teng_endif
    : ENDIF ignored_options END {finalize_if(ctx);}
    | ENDIF error_up_to_end END {finalize_inv_if(ctx, $1->pos);}
    ;


 /**************************************************** RULES: expr directives */


teng_expr
    : EXPR expr_up_to_end END {generate_print(ctx);}
    | EXPR expr_up_to_end INV {generate_inv_print(ctx, *$3);}
    | SHORT_EXPR expr_up_to_short_end SHORT_END {generate_print(ctx);}
    | SHORT_EXPR expr_up_to_short_end INV {generate_inv_print(ctx, *$3);}
    ;


teng_dict
    : SHORT_DICT identifier SHORT_END {print_dict_lookup(ctx, *$2);}
    | SHORT_DICT error_up_to_short_end SHORT_END {print_dict_undef(ctx, *$1);}
    ;


 /********************************************************* RULES: expression */


 // The purpose of expr_up_to_end is pushing new frame to branch
 // addresses stack.
expr_up_to_end
    : {prepare_expr(ctx, YYLA->pos);} expr_up_to_end_impl {$$ = $2;}
    ;


 // The purpose of expr_up_to_end_impl is finish/discard valid/invalid
 // expressions. The error 'up to end' ensures that error recovery stops just
 // before END token.
expr_up_to_end_impl
    : expression_impl {finish_expr(ctx); $$ = true;}
    | error_up_to_end {discard_expr(ctx); $$ = false;}
    ;


 // The purpose of expr_up_to_end is pushing new frame to branch
 // addresses stack.
expr_up_to_short_end
    : {prepare_expr(ctx, YYLA->pos);} expr_up_to_short_end_impl
    ;


 // The purpose of expr_up_to_end_impl is finish/discard valid/invalid
 // expressions. The error 'up to end' ensures that error recovery stops just
 // before SHORT_END token.
expr_up_to_short_end_impl
    : expression_impl {finish_expr(ctx);}
    | error_up_to_short_end {discard_expr(ctx);}
    ;


 // The purpuse of is to note expression's start address in program and mainly
 // note position of the expression's first token. This has to be here because
 // in expr_up_to_end state the YYLA contains token of previous syntactic
 // element.
expression_impl
    : {note_expr_start_point(ctx, YYLA->pos);} expression {}
    ;


expression
    : nullary_expression {note_optimization_point(ctx, $1);}
    | unary_expression {optimize_expr(ctx, 1);}
    | binary_expression {optimize_expr(ctx, 2);}
    | lazy_binary_expression {optimize_expr(ctx, 2, true);}
    | deprecated_binary_expression {optimize_expr(ctx, $1);}
    | ternary_expression {optimize_expr(ctx, 3, true);}
    | nary_expression {optimize_expr(ctx, $1->arity, $1->lazy_evaluated);}
    ;


nullary_expression
    : VAR variable {generate_var(ctx, std::move(*$2)); $$ = false;}
    | variable {generate_var(ctx, std::move(*$1)); $$ = false;}
    | value_literal {generate_val(ctx, $1->pos, $1->value); $$ = true;}
    | DICT identifier {generate_dict_lookup(ctx, *$2); $$ = true;}
    | REGEX {generate_val(ctx, $1->pos, Value_t(generate_regex(ctx, *$1)));}
    ;


unary_expression
    : L_PAREN expression R_PAREN {}
    | NOT expression {generate_expr<Not_t>(ctx, *$1);}
    | BITNOT expression {generate_expr<BitNot_t>(ctx, *$1);}
    | DICT_INDIRECT expression {generate_expr<Dict_t>(ctx, *$1);}
    | PLUS expression {generate_expr<UnaryPlus_t>(ctx, *$1);} %prec HIGHEST_PREC
    | MINUS expression {generate_expr<UnaryMinus_t>(ctx, *$1);} %prec HIGHEST_PREC
    /* | expression STR_EQ REGEX {generate_match(ctx, *$2, *$3);} */
    /* | expression STR_NE REGEX {generate_match(ctx, *$2, *$3);} */
    | runtime_variable {generate<Repr_t>(ctx, $1);}
    ;


binary_expression
    : expression EQ expression {generate_expr<EQ_t>(ctx, *$2);}
    | expression EQ_DIGRAPH expression {generate_expr<EQ_t>(ctx, *$2);}
    | expression NE expression {generate_expr<NE_t>(ctx, *$2);}
    | expression NE_DIGRAPH expression {generate_expr<NE_t>(ctx, *$2);}
    | expression GE expression {generate_expr<GE_t>(ctx, *$2);}
    | expression GE_DIGRAPH expression {generate_expr<GE_t>(ctx, *$2);}
    | expression GT expression {generate_expr<GT_t>(ctx, *$2);}
    | expression GT_DIGRAPH expression {generate_expr<GT_t>(ctx, *$2);}
    | expression LE expression {generate_expr<LE_t>(ctx, *$2);}
    | expression LE_DIGRAPH expression {generate_expr<LE_t>(ctx, *$2);}
    | expression LT expression {generate_expr<LT_t>(ctx, *$2);}
    | expression LT_DIGRAPH expression {generate_expr<LT_t>(ctx, *$2);}
    | expression BITOR expression {generate_expr<BitOr_t>(ctx, *$2);}
    | expression BITXOR expression {generate_expr<BitXor_t>(ctx, *$2);}
    | expression BITAND expression {generate_expr<BitAnd_t>(ctx, *$2);}
    | expression PLUS expression {generate_expr<Plus_t>(ctx, *$2);}
    | expression MINUS expression {generate_expr<Minus_t>(ctx, *$2);}
    | expression MUL expression {generate_expr<Mul_t>(ctx, *$2);}
    | expression DIV expression {generate_expr<Div_t>(ctx, *$2);}
    | expression MOD expression {generate_expr<Mod_t>(ctx, *$2);}
    | expression REPEAT expression {generate_expr<Repeat_t>(ctx, *$2);}
    ;


lazy_binary_expression
    : expression OR {generate_bin_op<Or_t>(ctx, *$2);}
      expression {finalize_bin_op<Or_t>(ctx);}
    | expression OR_DIGRAPH {generate_bin_op<Or_t>(ctx, *$2);}
      expression {finalize_bin_op<Or_t>(ctx);}
    | expression AND {generate_bin_op<And_t>(ctx, *$2);}
      expression {finalize_bin_op<And_t>(ctx);}
    | expression AND_TRIGRAPH {generate_bin_op<And_t>(ctx, *$2);}
      expression {finalize_bin_op<And_t>(ctx);}
    ;


 // TODO(burlog): uncommnet 'expression STR_EQ REGEX' rule, after removed
deprecated_binary_expression
    : expression STR_EQ expression {$$ = generate_str_expr<StrEQ_t>(ctx, *$2);}
    | expression STR_NE expression {$$ = generate_str_expr<StrNE_t>(ctx, *$2);}
    | expression CONCAT expression {generate_expr<Concat_t>(ctx, *$2); $$ = 2;}
    ;


ternary_expression
    : expression
      COND_EXPR {generate_tern_op(ctx, *$2);}
      expression {expr_diag(ctx, diag_code::tern_colon);}
      COLON {finalize_tern_op_true_branch(ctx, *$6);}
      expression {finalize_tern_op_false_branch(ctx);}
    ;


nary_expression
    : function_expression {$$ = std::move($1);}
    | case_expression {$$ = std::move($1);}
    | query_expression {$$ = std::move($1);}
    ;


 /*********************************************** RULES: identifier, literals */


value_literal
    : string_literal {$$ = std::move($1);}
    | int_literal {$$ = std::move($1);}
    | real_literal {$$ = std::move($1);}
    ;


int_literal
    : DEC_INT {$$.emplace(*$1, Literal_t::extract_int($1->view(), 10));}
    | HEX_INT {$$.emplace(*$1, Literal_t::extract_int($1->view(), 16));}
    | BIN_INT {$$.emplace(*$1, Literal_t::extract_int($1->view(), 2));}
    ;


real_literal
    : REAL {$$.emplace(*$1, Literal_t::extract_real($1->view()));}
    ;


string_literal_fact
    : STRING {$$.emplace(*$1, Literal_t::extract_str($1->view()));}
    ;


string_literal
    : string_literal string_literal_fact
      {$$.emplace(*$1, $1->value.as_string() + $2->value.as_string());}
    | string_literal_fact {$$ = std::move($1);}
    ;


// Following code is for parsing literals used in case label. It must
// be literal for fast evaluation but we must allow thing like +number
// and -number.
signed_literal
    : value_literal {$$ = std::move($1);}
    | PLUS int_literal {$$ = std::move($2);}
    | PLUS real_literal {$$ = std::move($2);}
    | MINUS int_literal {$$.emplace(*$1, -$2->value.as_int());}
    | MINUS real_literal {$$.emplace(*$1, -$2->value.as_real());}
    ;


identifier_relative
    : IDENT {$$ = std::move($1);}
    | BUILTIN_FIRST {$$ = std::move($1);}
    | BUILTIN_INNER {$$ = std::move($1);}
    | BUILTIN_LAST {$$ = std::move($1);}
    | BUILTIN_INDEX {$$ = std::move($1);}
    | BUILTIN_COUNT {$$ = std::move($1);}
    | TYPE {$$ = std::move($1);}
    | COUNT {$$ = std::move($1);}
    | JSONIFY {$$ = std::move($1);}
    | EXISTS {$$ = std::move($1);}
    | LT_DIGRAPH {$$ = std::move($1);}
    | LE_DIGRAPH {$$ = std::move($1);}
    | GT_DIGRAPH {$$ = std::move($1);}
    | GE_DIGRAPH {$$ = std::move($1);}
    | EQ_DIGRAPH {$$ = std::move($1);}
    | NE_DIGRAPH {$$ = std::move($1);}
    | AND_TRIGRAPH {$$ = std::move($1);}
    | OR_DIGRAPH {$$ = std::move($1);}
    | CASE {$$ = std::move($1);}
    ;


identifier
    : identifier_relative {$$ = std::move($1);}
    | BUILTIN_THIS {$$ = std::move($1);}
    | BUILTIN_PARENT {$$ = std::move($1);}
    ;


 /********************************************** RULES: teng regular variable */


variable
    : absolute_variable {$$.emplace(std::move(ctx->var_sym));}
    | relative_variable {$$.emplace(std::move(ctx->var_sym));}
    | local_variable {$$.emplace(std::move(ctx->var_sym));}
    ;


absolute_variable_prefix
    : SELECTOR {prepare_root_variable(ctx, *$1);}
    | BUILTIN_THIS SELECTOR {prepare_this_variable(ctx, *$1);}
    | BUILTIN_PARENT SELECTOR {prepare_parent_variable(ctx, *$1);}
    ;


absolute_variable
    : absolute_variable_prefix absolute_variable_segment identifier
      {ctx->var_sym.push_back(*$3);}
    ;


absolute_variable_segment
    : absolute_variable_segment identifier_relative SELECTOR
      {ctx->var_sym.push_back(*$2);}
    | absolute_variable_segment BUILTIN_THIS SELECTOR
      {ignoring_this(ctx, $2->pos);}
    | absolute_variable_segment BUILTIN_PARENT SELECTOR
      {ctx->var_sym.pop_back(ctx, $2->pos);}
    | %empty
      {}
    ;


relative_variable
    : identifier_relative
      {ctx->var_sym = Variable_t(*$1);}
      relative_variable_segment identifier
      {ctx->var_sym.push_back(*$4);}
    ;


relative_variable_segment
    : relative_variable_segment identifier_relative SELECTOR
      {ctx->var_sym.push_back(*$2);}
    | relative_variable_segment BUILTIN_THIS SELECTOR
      {ignoring_this(ctx, $2->pos);}
    | relative_variable_segment BUILTIN_PARENT SELECTOR
      {ctx->var_sym.pop_back(ctx, $2->pos);}
    | SELECTOR
      {}
    ;


local_variable
    : identifier {ctx->var_sym = Variable_t(*$1);}
    ;


 /********************************************** RULES: teng runtime variable */


runtime_variable
    : VAR VAR rtvar_path {$$ = $1->pos;}
    ;


rtvar_path
    : SELECTOR {generate_rtvar<PushRootFrag_t>(ctx, *$1);} rtvar_segment {}
    | {generate_rtvar<PushThisFrag_t>(ctx, *YYLA);} rtvar_segment {}
    ;


rtvar_segment
    : rtvar_segment SELECTOR rtvar_identifier rtvar_index {}
    | rtvar_identifier rtvar_index {}
    ;


rtvar_index
    : rtvar_index L_BRACKET expression R_BRACKET
      {generate_rtvar_index(ctx, *$2);}
    | %empty {}
    ;


rtvar_identifier
    : identifier_relative {generate<PushAttr_t>(ctx, $1->str(), $1->pos);}
    | BUILTIN_THIS {/*ignore this*/}
    | BUILTIN_PARENT {generate<PopAttr_t>(ctx, $1->pos);}
    ;


 /**************************************************** RULES: case expression */


case_expression
    : CASE L_PAREN {prepare_case(ctx);}
      expression {prepare_case_cond(ctx, *$1);}
      COMMA case_options R_PAREN {$$.emplace(finalize_case(ctx, *$8, $7));}
    ;


case_options
    : case_particular_options COMMA case_default_option {$$ = $1 + 1;}
    | case_particular_options {$$ = $1; generate<Val_t>(ctx, YYLA->pos);}
    | case_default_option {$$ = 1;}
    ;


case_default_option
    : MUL COLON {expr_diag(ctx, diag_code::case_default_branch);} expression {}
    ;


case_particular_options
    : case_particular_options
      COMMA case_values COLON {update_case_jmp(ctx, *$2, $3);}
      expression {finalize_case_branch(ctx, *$2); $$ = $1 + 1;}
    | case_values COLON {update_case_jmp(ctx, *$2, $1);}
      expression {finalize_case_branch(ctx, *YYLA); $$ = 1;}
    ;


case_values
    : case_values COMMA signed_literal {$$ = generate_case_next(ctx, *$3, $1);}
    | signed_literal {$$ = generate_case_cmp(ctx, *$1);}
    ;


 /************************************************ RULES: function expression */


function_expression
    : function_name L_PAREN {expr_diag_sentinel(ctx, diag_code::fun_args);}
      function_arguments R_PAREN {$$.emplace(*$1, generate_func(ctx, *$1, $4));}
    ;


function_name
    : IDENT {$$ = std::move($1);}
    | UDF_IDENT {$$ = std::move($1);}


function_arguments
    : function_arguments COMMA expression {$$ = $1 + 1;}
    | expression {$$ = 1;}
    | %empty {$$ = 0;}
    ;


 /*************************************************** RULES: query expression */


query_expression
    : DEFINED query_argument
      {$$.emplace(query_expr<ReprDefined_t>(ctx, *$1, $2));}
    | EXISTS query_argument
      {$$.emplace(query_expr<ReprExists_t>(ctx, *$1, $2));}
    | ISEMPTY query_argument
      {$$.emplace(query_expr<ReprIsEmpty_t>(ctx, *$1, $2));}
    | TYPE query_argument
      {$$.emplace(query_expr<ReprType_t>(ctx, *$1, $2));}
    | COUNT query_argument
      {$$.emplace(query_expr<ReprCount_t>(ctx, *$1, $2));}
    | JSONIFY query_argument
      {$$.emplace(query_expr<ReprJsonify_t>(ctx, *$1, $2));}
    ;


query_argument
    : L_PAREN query_argument_inner R_PAREN {$$ = 1;}
    | L_PAREN error_up_to_r_paren R_PAREN {$$ = 0;}
    ;


query_argument_inner
    : variable {generate_query(ctx, *$1, false);}
    | VAR variable {generate_query(ctx, *$2, true);}
    | runtime_variable {/*generates instructions itself*/}
    ;


 /************************************ EPILOGUE: glue between bison and flex. */
%%

namespace Teng {
namespace Parser {
namespace Impl {

int yylex(parser::semantic_type *variant, Context_t *ctx) {
    return *(variant->build<TokenSymbol_t>().emplace(ctx->next_token()));
}

} // namespace Impl
} // namespace Parser
} // namespace Teng

