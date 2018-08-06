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
 * $Id: tenglex2.ll,v 1.7 2007-10-18 14:45:45 vasek Exp $
 *
 *
 * DESCRIPTION
 * Teng lexical analyzer.
 *
 * AUTHORS
 * Vasek Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (vasek)
 *             Created.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

/* flex generator settings */
%option noyywrap
%option nounput
%option never-interactive
%option align
%option 8bit
%option nodefault
%option full
%option reentrant
%option prefix="teng_"
%option noyylineno
%option outfile="lex.yy.c"

/* additional lexer contexes */
%x quoted_string
%x bad_number
%x multiline_comment
%x oneline_comment
%x xml_tag

%{

#include <string>

#include "tengyystype.h"
#include "tengsyntax.hh"
#include "tenglogging.h"
#include "tenglex2.h"

// hide yyinput(yyscan_t) declaration
#define YY_NO_INPUT
// singature of lexer get-next-symbol method
#define YY_DECL Teng::Parser::Symbol_t Teng::Parser::Lex2_t::next()
// at the end of input execute following stuff
#define yyterminate() return Teng::Parser::Symbol_t{symbol_pos, 0, {}}

%}

INTEGER     [[:digit:]]+
HEX_INTEGER "0x"[[:xdigit:]]+
BIN_INTEGER "0b"[01]+
REAL        [[:digit:]]+"."[[:digit:]]+([eE][+-]?[[:digit:]]+)?
IDENT       [_[:alpha:]][_[:alnum:]]*

%%
    // quote used to delimit string single or double quote
    char string_quoting_char = 0;

"<?teng"[[:space:]\0]+"debug" {
    // match '<?teng debug'
    return make_symbol(LEX2::DEBUG_FRAG, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"bytecode" {
    // match '<?teng bytecode'
    return make_symbol(LEX2::BYTECODE_FRAG, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"include" {
    // match '<?teng include'
    return make_symbol(LEX2::INCLUDE, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"format" {
    // match '<?teng format'
    return make_symbol(LEX2::FORMAT, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"endformat" {
    // match '<?teng endformat'
    return make_symbol(LEX2::ENDFORMAT, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"frag" {
    // match '<?teng frag'
    return make_symbol(LEX2::FRAGMENT, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"endfrag" {
    // match '<?teng endfrag'
    return make_symbol(LEX2::ENDFRAGMENT, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"if" {
    // match '<?teng if'
    return make_symbol(LEX2::IF, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"endif" {
    // match '<?teng endif'
    return make_symbol(LEX2::ENDIF, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"elseif" {
    // match '<?teng elseif'
    return make_symbol(LEX2::ELSEIF, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"elif" {
    // match '<?teng elif'
    return make_symbol(LEX2::ELSEIF, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"else" {
    // match '<?teng else'
    return make_symbol(LEX2::ELSE, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"set" {
    // match '<?teng set'
    return make_symbol(LEX2::SET, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"expr" {
    // match '<?teng expr'
    return make_symbol(LEX2::EXPR, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"ctype" {
    // match '<?teng ctype'
    return make_symbol(LEX2::CTYPE, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+"endctype" {
    // match '<?teng endctype'
    return make_symbol(LEX2::ENDCTYPE, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]*[[:alnum:]]* {
    // match '<?teng???'
    return make_symbol(LEX2::TENG, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"debug" {
    // match '<?debug'
    return make_symbol(LEX2::DEBUG_FRAG, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"bytecode" {
    // match '<?bytecode'
    return make_symbol(LEX2::BYTECODE_FRAG, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"include" {
    // match '<?include'
    return make_symbol(LEX2::INCLUDE, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"format" {
    // match '<?format'
    return make_symbol(LEX2::FORMAT, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"endformat" {
    // match '<?endformat'
    return make_symbol(LEX2::ENDFORMAT, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"frag" {
    // match '<?frag'
    return make_symbol(LEX2::FRAGMENT, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"endfrag" {
    // match '<?endfrag'
    return make_symbol(LEX2::ENDFRAGMENT, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"if" {
    // match '<?if'
    return make_symbol(LEX2::IF, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"endif" {
    // match '<?endif'
    return make_symbol(LEX2::ENDIF, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"elseif" {
    // match '<?elseif'
    return make_symbol(LEX2::ELSEIF, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"elif" {
    // match '<?elif'
    return make_symbol(LEX2::ELSEIF, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"else" {
    // match '<?else'
    return make_symbol(LEX2::ELSE, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"set" {
    // match '<?set'
    return make_symbol(LEX2::SET, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"expr" {
    // match '<?expr'
    return make_symbol(LEX2::EXPR, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"ctype" {
    // match '<?ctype'
    return make_symbol(LEX2::CTYPE, yytext, yytext + yyleng);
}

"<?"[[:space:]\0]*"endctype" {
    // match '<?endctype'
    return make_symbol(LEX2::ENDCTYPE, yytext, yytext + yyleng);
}

"<?" {
    // xml tag start
    make_symbol_start(yytext);
    // change context to parse (ehm, ignore) xml tag's content
    BEGIN(xml_tag);
}

<xml_tag>{
    "?>" {
        // end of xml tag
        pos.advanceColumn(yyleng);
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::TEXT, yytext + yyleng);
    }
    <<EOF>> {
        // end of input => bad token
        logError(err, pos, "Unterminated xml tag");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    .|"\n" {
        // do nothing
    }
}

"?>" {
    // match '?>'
    return make_oneline_symbol(LEX2::END, yytext, yytext + yyleng);
}

"${" {
    // match '${' -- inline variable lookup
    return make_oneline_symbol(LEX2::SHORT_EXPR, yytext, yytext + yyleng);
}

"#{" {
    // match '#{' -- inline dictionary lookup
    return make_oneline_symbol(LEX2::SHORT_DICT, yytext, yytext + yyleng);
}

"}" {
    // match '}'
    return make_oneline_symbol(LEX2::SHORT_END, yytext, yytext + yyleng);
}

"(" {
    // match left parenthesis
    return make_oneline_symbol(LEX2::L_PAREN, yytext, yytext + yyleng);
}

")" {
    // match right parenthesis
    return make_oneline_symbol(LEX2::R_PAREN, yytext, yytext + yyleng);
}

"[" {
    // match left bracket
    return make_oneline_symbol(LEX2::L_BRACKET, yytext, yytext + yyleng);
}

"]" {
    // match right bracket
    return make_oneline_symbol(LEX2::R_BRACKET, yytext, yytext + yyleng);
}

"." {
    // match selector
    return make_oneline_symbol(LEX2::SELECTOR, yytext, yytext + yyleng);
}

"#" {
    // match dictionary lookup
    return make_oneline_symbol(LEX2::DICT, yytext, yytext + yyleng);
}

"$" {
    // match variable lookup
    return make_oneline_symbol(LEX2::VAR, yytext, yytext + yyleng);
}

"@" {
    // match dictionary indirect lookup
    return make_oneline_symbol(LEX2::DICT_INDIRECT, yytext, yytext + yyleng);
}

"++" {
    // match concatenation operator
    return make_oneline_symbol(LEX2::CONCAT, yytext, yytext + yyleng);
}

"**" {
    // match repeat a pattern operator
    return make_oneline_symbol(LEX2::REPEAT, yytext, yytext + yyleng);
}

"," {
    // match comma
    return make_oneline_symbol(LEX2::COMMA, yytext, yytext + yyleng);
}

"?" {
    // match conditional expression operator
    return make_oneline_symbol(LEX2::COND_EXPR, yytext, yytext + yyleng);
}

":" {
    // match colon
    return make_oneline_symbol(LEX2::COLON, yytext, yytext + yyleng);
}

"=" {
    // match assignment operator
    return make_oneline_symbol(LEX2::ASSIGN, yytext, yytext + yyleng);
}

"==" {
    // match numeric eqaul operator
    return make_oneline_symbol(LEX2::EQ, yytext, yytext + yyleng);
}

"!=" {
    // match numeric not eqaul operator
    return make_oneline_symbol(LEX2::NE, yytext, yytext + yyleng);
}

">=" {
    // match greater or queal operator
    return make_oneline_symbol(LEX2::GE, yytext, yytext + yyleng);
}

"<=" {
    // match less or queal operator
    return make_oneline_symbol(LEX2::LE, yytext, yytext + yyleng);
}

">" {
    // match greater than operator
    return make_oneline_symbol(LEX2::GT, yytext, yytext + yyleng);
}

"<" {
    // match less than operator
    return make_oneline_symbol(LEX2::LT, yytext, yytext + yyleng);
}

"eq" {
    // match eqaul operator
    return make_oneline_symbol(LEX2::EQ, yytext, yytext + yyleng);
}

"ne" {
    // match not eqaul operator
    return make_oneline_symbol(LEX2::NE, yytext, yytext + yyleng);
}

"ge" {
    // match greater or queal operator
    return make_oneline_symbol(LEX2::GE, yytext, yytext + yyleng);
}

"le" {
    // match less or queal operator
    return make_oneline_symbol(LEX2::LE, yytext, yytext + yyleng);
}

"gt" {
    // match greater than operator
    return make_oneline_symbol(LEX2::GT, yytext, yytext + yyleng);
}

"lt" {
    // match less than operator
    return make_oneline_symbol(LEX2::LT, yytext, yytext + yyleng);
}

"=~" {
    // match string equal operator
    return make_oneline_symbol(LEX2::STR_EQ, yytext, yytext + yyleng);
}

"!~" {
    // match string not equal operator
    return make_oneline_symbol(LEX2::STR_NE, yytext, yytext + yyleng);
}

"+" {
    // match numeric add operator
    return make_oneline_symbol(LEX2::ADD, yytext, yytext + yyleng);
}

"-" {
    // match numeric sub operator
    return make_oneline_symbol(LEX2::SUB, yytext, yytext + yyleng);
}

"*" {
    // match numeric mul operator
    return make_oneline_symbol(LEX2::MUL, yytext, yytext + yyleng);
}

"/" {
    // match numeric div operator
    return make_oneline_symbol(LEX2::DIV, yytext, yytext + yyleng);
}

"%" {
    // match numeric mod operator
    return make_oneline_symbol(LEX2::MOD, yytext, yytext + yyleng);
}

"!" {
    // match not operator
    return make_oneline_symbol(LEX2::NOT, yytext, yytext + yyleng);
}

"&" {
    // match bitwise and operator
    return make_oneline_symbol(LEX2::BITAND, yytext, yytext + yyleng);
}

"^" {
    // match bitwise xor operator
    return make_oneline_symbol(LEX2::BITXOR, yytext, yytext + yyleng);
}

"|" {
    // match bitwise or operator
    return make_oneline_symbol(LEX2::BITOR, yytext, yytext + yyleng);
}

"~" {
    // match bitwise not operator
    return make_oneline_symbol(LEX2::BITNOT, yytext, yytext + yyleng);
}

"&&" {
    // match and operator
    return make_oneline_symbol(LEX2::AND, yytext, yytext + yyleng);
}

"||" {
    // match or operator
    return make_oneline_symbol(LEX2::OR, yytext, yytext + yyleng);
}

"case" {
    // match case operator
    return make_oneline_symbol(LEX2::CASE, yytext, yytext + yyleng);
}

"defined" {
    // match defined operator
    return make_oneline_symbol(LEX2::DEFINED, yytext, yytext + yyleng);
}

"isempty" {
    // match exist operator
    return make_oneline_symbol(LEX2::ISEMPTY, yytext, yytext + yyleng);
}

"exists" {
    // match exist operator
    return make_oneline_symbol(LEX2::EXISTS, yytext, yytext + yyleng);
}

"_first" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_FIRST, yytext, yytext + yyleng);
}

"_inner" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_INNER, yytext, yytext + yyleng);
}

"_last" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_LAST, yytext, yytext + yyleng);
}

"_number" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_INDEX, yytext, yytext + yyleng);
}

"_index" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_INDEX, yytext, yytext + yyleng);
}

"_count" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_COUNT, yytext, yytext + yyleng);
}

"_this" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_THIS, yytext, yytext + yyleng);
}

"_parent" {
    // match builtin variable name
    return make_oneline_symbol(LEX2::BUILTIN_PARENT, yytext, yytext + yyleng);
}

[\"\'] {
    // opening quote of string
    string_quoting_char = *yytext;
    make_symbol_start(yytext);
    // change context to parse string content
    BEGIN(quoted_string);
}

<quoted_string>{
    [\"\'] {
        if (*yytext == string_quoting_char) {
            // regular end-of-string
            // leave this context
            BEGIN(INITIAL);
            return make_symbol(LEX2::STRING, yytext + yyleng);
        }
    }
    "\n" {
        // newline in string => bad token
        logError(err, pos, "Bare newline cannot be part of string");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    "\\\n" {
        // newline escaped => bad token
        logError(err, pos, "Newline cannot be escaped");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    <<EOF>> {
        // end of input => bad token
        logError(err, pos, "Unterminated string");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    "\\n" {
        // escape for <LF>
    }
    "\\r" {
        // escape for <CR>
    }
    "\\t" {
        // escape for <TAB>
    }
    "\\f" {
        // escape for <FF>
    }
    "\\b" {
        // escape for <BEL>
    }
    "\\". {
        // escape for other escape => put escaped char verbatim into token
    }
    [^\"\'\\\n]+ {
        // run of regular string characters => copy verbatim into token
    }
    "\\" {
        // escape itself =>
        logError(err, pos, "Invalid escape at end of string");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
}

{INTEGER} {
    // match integral number
    return make_oneline_symbol(LEX2::INT, yytext, yytext + yyleng);
}

{HEX_INTEGER} {
    // match integral number
    return make_oneline_symbol(LEX2::INT, yytext, yytext + yyleng);
}

{BIN_INTEGER} {
    // match integral number
    return make_oneline_symbol(LEX2::INT, yytext, yytext + yyleng);
}

{REAL} {
    // match real number
    return make_oneline_symbol(LEX2::REAL, yytext, yytext + yyleng);
}

{INTEGER}|{REAL}/[._[:alpha:]] {
    // match number (integer or real) followed by possible indentifier or dot
    make_symbol_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

{HEX_INTEGER}/[._] {
    // match hexa number followed by possible indentifier or dot
    make_symbol_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

{BIN_INTEGER}/[2-9._[:alpha:]] {
    // match bin number followed by possible indentifier or dot
    make_symbol_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

<bad_number>{
    [._[:alnum:]]+ {
        // run of number, dot or identifier characters composing bad token
        auto symbol_len = (yytext - symbol_ipos) + yyleng;
        std::string tmp(symbol_ipos, symbol_ipos + symbol_len);
        logError(err, pos, "Invalid token '" + tmp + "' after number");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    [^._[:alnum:]]+ {
        // match any other sequence of characters
        // return them back to the input
        yyless(0);
        BEGIN(INITIAL);
    }
}

"udf."{IDENT}(\.{IDENT})* {
    // match identifier of the user defined functions
    return make_oneline_symbol(LEX2::UDF_IDENT, yytext, yytext + yyleng);
}

{IDENT} {
    // match identifier
    return make_oneline_symbol(LEX2::IDENT, yytext, yytext + yyleng);
}

[[:space:]\0] {
    // match spaces -- spaces are ignored
}

. {
    // default rule
    std::string tmp(yytext, yyleng);
    logError(err, pos, "Unexpected character '" + tmp + "'");
    return make_symbol(LEX2::INVALID, yytext, yytext + yyleng);
}

"/*" {
    // comment start
    make_symbol_start(yytext);
    // change context to parse (ehm, ignore) comment's content
    BEGIN(multiline_comment);
}

<multiline_comment>{
    "*/" {
        // end of comment
        pos.advanceColumn(yyleng);
        BEGIN(INITIAL);
    }
    <<EOF>> {
        // end of input => bad token
        logError(err, pos, "Unterminated comment");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    .|"\n" {
        // ignore
    }
}

"//" {
    // comment start
    make_symbol_start(yytext);
    // change context to parse (ehm, ignore) comment's content
    BEGIN(oneline_comment);
}

<oneline_comment>{
    "\n" {
        // end of comment
        pos.advanceColumn(yyleng);
        BEGIN(INITIAL);
    }
    <<EOF>> {
        // end of input => bad token
        logError(err, pos, "Unterminated comment");
        // leave this context
        BEGIN(INITIAL);
        return make_symbol(LEX2::INVALID, yytext + yyleng);
    }
    . {
        // ignore
    }
}

%%

namespace Teng {
namespace Parser {
namespace {flex_string_value_t empty_directive(0);}

Lex2_t::Lex2_t(Error_t &err)
  : yyscanner(nullptr), buffer(nullptr),
    err(err), pos(), symbol_pos(), symbol_ipos(nullptr), directive(empty_directive)
{
    if (yylex_init(&yyscanner)) {
        char error[1024];
        error[0] = '\0';
        strerror_r(errno, error, sizeof(error));
        throw std::runtime_error("can't initalize lex2: " + std::string(error));
    }
}

Lex2_t::~Lex2_t() {
    if (buffer) finish_scanning();
    yylex_destroy((yyscan_t)yyscanner);
}

void Lex2_t::start_scanning(flex_string_view_t &&new_directive, const Pos_t &init_pos) {
    if (buffer != nullptr)
        throw std::runtime_error("another buffer in lex2 not finished yet");
    pos = init_pos;
    directive = std::move(new_directive);
    // std::cerr << "LEX2::DIRECTIVE ptr=" << (void *)directive.data()
    //           << ", #" << directive.size()
    //           << ", [" << (int)directive[directive.size() + 0] << "/" << (int)directive.saved_eob()[0] << "]"
    //           << ", [" << (int)directive[directive.size() + 1]  << "/" << (int)directive.saved_eob()[1] << "]"
    //           << std::endl
    //           << "LEX2::DATA " << directive.str()
    //           << std::endl;
    if (!(buffer = yy_scan_buffer(directive.data(), directive.flex_size(), yyscanner)))
        throw std::runtime_error("can't allocate flex buffer in lex2");
}

void Lex2_t::finish_scanning() {
    directive.reset();
    yy_delete_buffer((YY_BUFFER_STATE)buffer, yyscanner);
    buffer = nullptr;
}

} // namespace Parser
} // namespace Teng

