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
%option outfile="lex2impl.cc"
%option header-file="lex2impl.h"

/* additional lexer contexes */
%x bad_number
%x multiline_comment
%x oneline_comment
%x xml_tag
%x regex
%x block_override

%{

#include <string>

#include "yystype.h"
#include "syntax.hh"
#include "logging.h"
#include "lex2.h"

// hide yyinput(yyscan_t) declaration
#define YY_NO_INPUT
// singature of lexer get-next-token method
#define YY_DECL Teng::Parser::Token_t Teng::Parser::Lex2_t::next()
// at the end of input execute following stuff
#define yyterminate() return Teng::Parser::Token_t{LEX2_EOF, token_pos, {}}

%}

 // basic tokens
INTEGER     [[:digit:]]+
HEX_INTEGER "0x"[[:xdigit:]]+
BIN_INTEGER "0b"[01]+
REAL        [[:digit:]]+"."[[:digit:]]+([eE][+-]?[[:digit:]]+)?
IDENT       [_[:alpha:]][_[:alnum:]]*

 // basic utf-8 support
UTF_CONT    [\x80-\xbf]
UTF_2       [\xc2-\xdf]
UTF_3       [\xe0-\xef]
UTF_4       [\xf0-\xf4]
UTF_5       [\xf8-\xfb]
UTF_6       [\xfc-\xfd]
UTF_2_CHAR  {UTF_2}{UTF_CONT}
UTF_3_CHAR  {UTF_3}{UTF_CONT}{UTF_CONT}
UTF_4_CHAR  {UTF_4}{UTF_CONT}{UTF_CONT}{UTF_CONT}
UTF_5_CHAR  {UTF_5}{UTF_CONT}{UTF_CONT}{UTF_CONT}{UTF_CONT}
UTF_6_CHAR  {UTF_6}{UTF_CONT}{UTF_CONT}{UTF_CONT}{UTF_CONT}{UTF_CONT}
UTF_CHAR    {UTF_2_CHAR}|{UTF_3_CHAR}|{UTF_4_CHAR}|{UTF_5_CHAR}|{UTF_6_CHAR}
TENG        "<?"("teng"[[:space:]\0])?[[:space:]\0]*

%%

"/*" {
    // comment start
    make_token_start(yytext);
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
        return make_token(LEX2::INV, yytext + yyleng - 1);
    }
    .|"\n" {
        // ignore
    }
}

"//" {
    // comment start
    make_token_start(yytext);
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
        return make_token(LEX2::INV, yytext + yyleng - 1);
    }
    . {
        // ignore
    }
}

<block_override>{
    {TENG}"endoverride"([[:space:]\0]+"block")? {
        // match '<?teng endoverride block'
        BEGIN(INITIAL);
        make_token_start(yytext +  yyleng);
        return make_token(LEX2::ENDBLOCK_OVERRIDE, yytext, yytext + yyleng);
    }
    "?>" {
        // match '?>'
        make_token_start(yytext +  yyleng);
        return make_nonewline_token(LEX2::END, yytext, yytext + yyleng);
    }
    {IDENT} {
        // match identifier
        make_token_start(yytext +  yyleng);
        return make_nonewline_token(LEX2::IDENT, yytext, yytext + yyleng);
    }
    <<EOF>> {
        // leave this context
        BEGIN(INITIAL);
        auto token_epos = (yytext +  yyleng - 1);
        if (token_ipos == token_epos)
            return Token_t{LEX2_EOF, token_pos, {}};
        return make_token(LEX2::TEXT, token_epos);
    }
    .|\n {
        // do nothing
    }
}

{TENG}"debug" {
    // match '<?teng debug'
    return make_token(LEX2::DEBUG_FRAG, yytext, yytext + yyleng);
}

{TENG}"bytecode" {
    // match '<?teng bytecode'
    return make_token(LEX2::BYTECODE_FRAG, yytext, yytext + yyleng);
}

{TENG}"include" {
    // match '<?teng include'
    return make_token(LEX2::INCLUDE, yytext, yytext + yyleng);
}

{TENG}"format" {
    // match '<?teng format'
    return make_token(LEX2::FORMAT, yytext, yytext + yyleng);
}

{TENG}"endformat" {
    // match '<?teng endformat'
    return make_token(LEX2::ENDFORMAT, yytext, yytext + yyleng);
}

{TENG}"frag" {
    // match '<?teng frag'
    return make_token(LEX2::FRAGMENT, yytext, yytext + yyleng);
}

{TENG}"endfrag" {
    // match '<?teng endfrag'
    return make_token(LEX2::ENDFRAGMENT, yytext, yytext + yyleng);
}

{TENG}"if" {
    // match '<?teng if'
    return make_token(LEX2::IF, yytext, yytext + yyleng);
}

{TENG}"endif" {
    // match '<?teng endif'
    return make_token(LEX2::ENDIF, yytext, yytext + yyleng);
}

{TENG}"elseif" {
    // match '<?teng elseif'
    return make_token(LEX2::ELSEIF, yytext, yytext + yyleng);
}

{TENG}"elif" {
    // match '<?teng elif'
    return make_token(LEX2::ELSEIF, yytext, yytext + yyleng);
}

{TENG}"else" {
    // match '<?teng else'
    return make_token(LEX2::ELSE, yytext, yytext + yyleng);
}

{TENG}"set" {
    // match '<?teng set'
    return make_token(LEX2::SET, yytext, yytext + yyleng);
}

{TENG}"expr" {
    // match '<?teng expr'
    return make_token(LEX2::ESC_EXPR, yytext, yytext + yyleng);
}

{TENG}"ctype" {
    // match '<?teng ctype'
    return make_token(LEX2::CTYPE, yytext, yytext + yyleng);
}

{TENG}"endctype" {
    // match '<?teng endctype'
    return make_token(LEX2::ENDCTYPE, yytext, yytext + yyleng);
}

{TENG}"extends" {
    // match '<?teng extends'
    return make_token(LEX2::EXTENDS, yytext, yytext + yyleng);
}

{TENG}"endextends" {
    // match '<?teng endextends'
    return make_token(LEX2::ENDEXTENDS, yytext, yytext + yyleng);
}

{TENG}"define"([[:space:]\0]+"block")? {
    // match '<?teng define block'
    return make_token(LEX2::DEFINE_BLOCK, yytext, yytext + yyleng);
}

<INITIAL,block_override>
{TENG}"override"([[:space:]\0]+"block")? {
    // match '<?teng override block'
    return make_token(LEX2::OVERRIDE_BLOCK, yytext, yytext + yyleng);
}

{TENG}"enddefine"([[:space:]\0]+"block")? {
    // match '<?teng enddefine block'
    return make_token(LEX2::ENDBLOCK_DEFINE, yytext, yytext + yyleng);
}

{TENG}"endoverride"([[:space:]\0]+"block")? {
    // match '<?teng endoverride block'
    return make_token(LEX2::ENDBLOCK_OVERRIDE, yytext, yytext + yyleng);
}

{TENG}"super"([[:space:]\0]+"block")? {
    // match '<?teng super block'
    return make_token(LEX2::SUPER_BLOCK, yytext, yytext + yyleng);
}

"<?teng"[[:space:]\0]+[[:alnum:]]* {
    // match '<?teng???'
    return make_token(LEX2::TENG, yytext, yytext + yyleng);
}

"<?" {
    // xml tag start
    make_token_start(yytext);
    // change context to parse (ehm, ignore) xml tag's content
    BEGIN(xml_tag);
}

<xml_tag>{
    "?>" {
        // end of xml tag
        pos.advanceColumn(yyleng);
        // leave this context
        BEGIN(INITIAL);
        return make_token(LEX2::TEXT, yytext + yyleng);
    }
    <<EOF>> {
        // end of input => bad token
        logError(err, pos, "Unterminated xml tag");
        // leave this context
        BEGIN(INITIAL);
        return make_token(LEX2::INV, yytext + yyleng - 1);
    }
    .|"\n" {
        // do nothing
    }
}

"?>" {
    // match '?>'
    return make_nonewline_token(LEX2::END, yytext, yytext + yyleng);
}

"${" {
    // match '${' -- inline variable lookup escaped
    return make_nonewline_token(LEX2::SHORT_ESC_EXPR, yytext, yytext + yyleng);
}

"#{" {
    // match '#{' -- inline dictionary lookup
    return make_nonewline_token(LEX2::SHORT_DICT, yytext, yytext + yyleng);
}

"%{" {
    // match '%{' -- inline variable lookup unescaped
    return make_nonewline_token(LEX2::SHORT_RAW_EXPR, yytext, yytext + yyleng);
}

"}" {
    // match '}'
    return make_nonewline_token(LEX2::SHORT_END, yytext, yytext + yyleng);
}

"(" {
    // match left parenthesis
    BEGIN(regex);
    return make_nonewline_token(LEX2::L_PAREN, yytext, yytext + yyleng);
}

")" {
    // match right parenthesis
    return make_nonewline_token(LEX2::R_PAREN, yytext, yytext + yyleng);
}

"[" {
    // match left bracket
    BEGIN(regex);
    return make_nonewline_token(LEX2::L_BRACKET, yytext, yytext + yyleng);
}

"]" {
    // match right bracket
    return make_nonewline_token(LEX2::R_BRACKET, yytext, yytext + yyleng);
}

"." {
    // match selector
    return make_nonewline_token(LEX2::SELECTOR, yytext, yytext + yyleng);
}

"#" {
    // match dictionary lookup
    return make_nonewline_token(LEX2::DICT, yytext, yytext + yyleng);
}

"$" {
    // match variable lookup
    return make_nonewline_token(LEX2::VAR, yytext, yytext + yyleng);
}

"@" {
    // match dictionary indirect lookup
    return make_nonewline_token(LEX2::DICT_INDIRECT, yytext, yytext + yyleng);
}

"++" {
    // match concatenation operator
    return make_nonewline_token(LEX2::CONCAT, yytext, yytext + yyleng);
}

"**" {
    // match repeat a pattern operator
    return make_nonewline_token(LEX2::REPEAT, yytext, yytext + yyleng);
}

"," {
    // match comma
    BEGIN(regex);
    return make_nonewline_token(LEX2::COMMA, yytext, yytext + yyleng);
}

"?" {
    // match conditional expression operator
    BEGIN(regex);
    return make_nonewline_token(LEX2::COND_EXPR, yytext, yytext + yyleng);
}

":" {
    // match colon
    BEGIN(regex);
    return make_nonewline_token(LEX2::COLON, yytext, yytext + yyleng);
}

"=" {
    // match assignment operator
    BEGIN(regex);
    return make_nonewline_token(LEX2::ASSIGN, yytext, yytext + yyleng);
}

"==" {
    // match equal operator
    return make_nonewline_token(LEX2::EQ, yytext, yytext + yyleng);
}

"!=" {
    // match not equal operator
    return make_nonewline_token(LEX2::NE, yytext, yytext + yyleng);
}

">=" {
    // match greater or queal operator
    return make_nonewline_token(LEX2::GE, yytext, yytext + yyleng);
}

"<=" {
    // match less or queal operator
    return make_nonewline_token(LEX2::LE, yytext, yytext + yyleng);
}

">" {
    // match greater than operator
    return make_nonewline_token(LEX2::GT, yytext, yytext + yyleng);
}

"<" {
    // match less than operator
    return make_nonewline_token(LEX2::LT, yytext, yytext + yyleng);
}

"eq" {
    // match equal operator
    return make_nonewline_token(LEX2::EQ_DIGRAPH, yytext, yytext + yyleng);
}

"ne" {
    // match not equal operator
    return make_nonewline_token(LEX2::NE_DIGRAPH, yytext, yytext + yyleng);
}

"ge" {
    // match greater or queal operator
    return make_nonewline_token(LEX2::GE_DIGRAPH, yytext, yytext + yyleng);
}

"le" {
    // match less or queal operator
    return make_nonewline_token(LEX2::LE_DIGRAPH, yytext, yytext + yyleng);
}

"gt" {
    // match greater than operator
    return make_nonewline_token(LEX2::GT_DIGRAPH, yytext, yytext + yyleng);
}

"lt" {
    // match less than operator
    return make_nonewline_token(LEX2::LT_DIGRAPH, yytext, yytext + yyleng);
}

"=~" {
    // match string equal operator
    BEGIN(regex);
    return make_nonewline_token(LEX2::STR_EQ, yytext, yytext + yyleng);
}

"!~" {
    // match string not equal operator
    BEGIN(regex);
    return make_nonewline_token(LEX2::STR_NE, yytext, yytext + yyleng);
}

"+" {
    // match plus operator
    return make_nonewline_token(LEX2::PLUS, yytext, yytext + yyleng);
}

"-" {
    // match minus operator
    return make_nonewline_token(LEX2::MINUS, yytext, yytext + yyleng);
}

"*" {
    // match numeric mul operator
    return make_nonewline_token(LEX2::MUL, yytext, yytext + yyleng);
}

"/" {
    // match numeric div operator
    return make_nonewline_token(LEX2::DIV, yytext, yytext + yyleng);
}

"%" {
    // match numeric mod operator
    return make_nonewline_token(LEX2::MOD, yytext, yytext + yyleng);
}

"!" {
    // match not operator
    return make_nonewline_token(LEX2::NOT, yytext, yytext + yyleng);
}

"&" {
    // match bitwise and operator
    return make_nonewline_token(LEX2::BITAND, yytext, yytext + yyleng);
}

"^" {
    // match bitwise xor operator
    return make_nonewline_token(LEX2::BITXOR, yytext, yytext + yyleng);
}

"|" {
    // match bitwise or operator
    return make_nonewline_token(LEX2::BITOR, yytext, yytext + yyleng);
}

"~" {
    // match bitwise not operator
    return make_nonewline_token(LEX2::BITNOT, yytext, yytext + yyleng);
}

"&&" {
    // match and operator
    return make_nonewline_token(LEX2::AND, yytext, yytext + yyleng);
}

"||" {
    // match or operator
    return make_nonewline_token(LEX2::OR, yytext, yytext + yyleng);
}

"and" {
    // match and operator
    return make_nonewline_token(LEX2::AND_TRIGRAPH, yytext, yytext + yyleng);
}

"or" {
    // match or operator
    return make_nonewline_token(LEX2::OR_DIGRAPH, yytext, yytext + yyleng);
}

"case" {
    // match case operator
    return make_nonewline_token(LEX2::CASE, yytext, yytext + yyleng);
}

"defined" {
    // match defined query
    return make_nonewline_token(LEX2::DEFINED, yytext, yytext + yyleng);
}

"repr" {
    // match repr query
    return make_nonewline_token(LEX2::REPR, yytext, yytext + yyleng);
}

"isempty" {
    // match exist query
    return make_nonewline_token(LEX2::ISEMPTY, yytext, yytext + yyleng);
}

"exists" {
    // match exist query
    return make_nonewline_token(LEX2::EXISTS, yytext, yytext + yyleng);
}

"type" {
    // match type query
    return make_nonewline_token(LEX2::TYPE, yytext, yytext + yyleng);
}

"count" {
    // match count query
    return make_nonewline_token(LEX2::COUNT, yytext, yytext + yyleng);
}

"_first" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_FIRST, yytext, yytext + yyleng);
}

"_inner" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_INNER, yytext, yytext + yyleng);
}

"_last" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_LAST, yytext, yytext + yyleng);
}

"_number" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_INDEX, yytext, yytext + yyleng);
}

"_index" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_INDEX, yytext, yytext + yyleng);
}

"_count" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_COUNT, yytext, yytext + yyleng);
}

"_error" {
    // match builtin fragment name
    return make_nonewline_token(LEX2::BUILTIN_ERROR, yytext, yytext + yyleng);
}

"_this" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_THIS, yytext, yytext + yyleng);
}

"_parent" {
    // match builtin variable name
    return make_nonewline_token(LEX2::BUILTIN_PARENT, yytext, yytext + yyleng);
}

\"([^\\\"]|\\.|\\\n)*\" {
    // match string literal in double quotes
    return make_token(LEX2::STRING, yytext, yytext + yyleng);
}

'([^\\']|\\.|\\\n)*' {
    // match string literal in single quotes
    return make_token(LEX2::STRING, yytext, yytext + yyleng);
}

<regex>{
    "/"([^\\/]|\\.)+"/"([gimGIM])* {
        // match regex pattern in slahes
        BEGIN(INITIAL);
        return make_nonewline_token(LEX2::REGEX, yytext, yytext + yyleng);
    }
    [[:space:]\0]* {
        // match spaces -- spaces are ignored
        pos.advance(yytext, yytext + yyleng);
    }
    .|\n {
        // match any other sequence of characters
        // return them back to the input
        yyless(0);
        BEGIN(INITIAL);
    }
}

{INTEGER} {
    // match integral number
    return make_nonewline_token(LEX2::DEC_INT, yytext, yytext + yyleng);
}

{HEX_INTEGER} {
    // match integral number
    return make_nonewline_token(LEX2::HEX_INT, yytext, yytext + yyleng);
}

{BIN_INTEGER} {
    // match integral number
    return make_nonewline_token(LEX2::BIN_INT, yytext, yytext + yyleng);
}

{REAL} {
    // match real number
    return make_nonewline_token(LEX2::REAL, yytext, yytext + yyleng);
}

{INTEGER}|{REAL}/[._[:alpha:]] {
    // match number (integer or real) followed by possible indentifier or dot
    make_token_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

{HEX_INTEGER}/[._] {
    // match hexa number followed by possible indentifier or dot
    make_token_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

{BIN_INTEGER}/[2-9._[:alpha:]] {
    // match bin number followed by possible indentifier or dot
    make_token_start(yytext);
    // start parsing of trailing characters
    BEGIN(bad_number);
}

<bad_number>{
    [._[:alnum:]]+ {
        // run of number, dot or identifier characters composing bad token
        auto token_len = (yytext - token_ipos) + yyleng;
        std::string tmp(token_ipos, token_ipos + token_len);
        logError(err, pos, "Invalid token '" + tmp + "' after number");
        // leave this context
        BEGIN(INITIAL);
        return make_token(LEX2::INV, yytext + yyleng);
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
    return make_nonewline_token(LEX2::UDF_IDENT, yytext, yytext + yyleng);
}

{IDENT} {
    // match identifier
    return make_nonewline_token(LEX2::IDENT, yytext, yytext + yyleng);
}

[[:space:]\0] {
    // match spaces -- spaces are ignored
    pos.advance(yytext, yytext + yyleng);
}

{UTF_CHAR} {
    // utf-8 character rule
    std::string tmp(yytext, yyleng);
    if (utf8) logError(err, pos, "Unexpected utf-8 encoded character '" + tmp + "'");
    else logError(err, pos, "Unexpected binary character '" + tmp + "'");
    return make_token(LEX2::INV, yytext, yytext + yyleng);
}

. {
    // default rule
    std::string tmp(yytext, yyleng);
    logError(err, pos, "Unexpected character '" + tmp + "'");
    return make_token(LEX2::INV, yytext, yytext + yyleng);
}

%%

void Teng::Parser::Lex2_t::set_start_condition(int start_condition) {
    auto *yyg = static_cast<yyguts_t *>(yyscanner);
    BEGIN(start_condition);
    if (start_condition == block_override)
        make_token_start(yytext);
}

