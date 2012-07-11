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
 *
 * HISTORY
 * 2003-09-18  (vasek)
 *             Created.
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
%option bison-bridge
%option prefix="teng_"
%option outfile="lex.yy.c"

%x qstr
%x badnumber
%x comment
%x one_line_comment

%{

#include <string>
#include <stdio.h>

#include "tengyystype.h"
#include "tengsyntax.h"
#include "tengparsercontext.h"
#include "tengerror.h"

    using namespace std;

    using namespace Teng;

#ifdef DEBUG_LEX
#define RETURN(token) \
    cout << #token << " '" << yytext << "', sval == '" << value.stringValue \
         << " nval == '" << value.integerValue << "' rval == '" \
         << value.realValue << "'" << std::endl; \
    return token;
#else
#define RETURN(token) \
    return token;
#endif

#define YY_DECL int Teng::Lex2_t::getElement(   \
            YYSTYPE *yylval_param,              \
            ParserValue_t &value,               \
            Error_t::Position_t                 \
            &bufferPos,                         \
            Error_t &err)

%}

 // integer value
INTEGER [[:digit:]]+

HEX_INTEGER "0x"[[:xdigit:]]+
BIN_INTEGER "0b"[01]+

 // real value
REAL    [[:digit:]]+"."[[:digit:]]+([eE][+-]?[[:digit:]]+)?

 // identifier
IDENT   [_[:alpha:]][_[:alnum:]]*

%%
    // temporary string value
    string tmpSval;
    // quote used to delimit string (" or ')
    char stringOpener = 0;

"<?teng"[[:space:]\0]+"debug" {
    // match '<?teng debug'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_DEBUG);
}

"<?teng"[[:space:]\0]+"bytecode" {
    // match '<?teng debug'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_BYTECODE);
}

"<?teng"[[:space:]\0]+"include" {
    // match '<?teng include'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_INCLUDE);
}

"<?teng"[[:space:]\0]+"format" {
    // match '<?teng format'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_FORMAT);
}

"<?teng"[[:space:]\0]+"endformat" {
    // match '<?teng endformat'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDFORMAT);
}

"<?teng"[[:space:]\0]+"frag" {
    // match '<?teng frag'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_FRAGMENT);
}

"<?teng"[[:space:]\0]+"endfrag" {
    // match '<?teng endfrag'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDFRAGMENT);
}

"<?teng"[[:space:]\0]+"if" {
    // match '<?teng if'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_IF);
}

"<?teng"[[:space:]\0]+"endif" {
    // match '<?teng endif'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDIF);
}

"<?teng"[[:space:]\0]+"elseif" {
    // match '<?teng elseif'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ELSEIF);
}

"<?teng"[[:space:]\0]+"else" {
    // match '<?teng else'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ELSE);
}

"<?teng"[[:space:]\0]+"set" {
    // match '<?teng set'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_SET);
}

"<?teng"[[:space:]\0]+"expr" {
    // match '<?teng expr'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_EXPR);
}

"<?teng"[[:space:]\0]+"ctype" {
    // match '<?teng ctype'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_CTYPE);
}

"<?teng"[[:space:]\0]+"endctype" {
    // match '<?teng endctype'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDCTYPE);
}

"<?teng"[[:space:]\0]+"repeatfrag" {
    // match '<?teng repeat'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_REPEATFRAG);
}

"<?teng"[[:space:]\0]*[[:alnum:]]* {
    // match '<?teng???'
    value.stringValue = string(yytext + 6, yyleng - 6);
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_TENG);
}

"<?"[[:space:]\0]*"debug" {
    // match '<?teng debug'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_DEBUG);
}

"<?"[[:space:]\0]*"bytecode" {
    // match '<?teng debug'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_BYTECODE);
}

"<?"[[:space:]\0]*"include" {
    // match '<?teng include'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_INCLUDE);
}

"<?"[[:space:]\0]*"format" {
    // match '<?teng format'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_FORMAT);
}

"<?"[[:space:]\0]*"endformat" {
    // match '<?teng endformat'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDFORMAT);
}

"<?"[[:space:]\0]*"frag" {
    // match '<?teng frag'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_FRAGMENT);
}

"<?"[[:space:]\0]*"endfrag" {
    // match '<?teng endfrag'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDFRAGMENT);
}

"<?"[[:space:]\0]*"if" {
    // match '<?teng if'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_IF);
}

"<?"[[:space:]\0]*"endif" {
    // match '<?teng endif'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDIF);
}

"<?"[[:space:]\0]*"elseif" {
    // match '<?teng elseif'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ELSEIF);
}

"<?"[[:space:]\0]*"else" {
    // match '<?teng else'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ELSE);
}

"<?"[[:space:]\0]*"set" {
    // match '<?teng set'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_SET);
}

"<?"[[:space:]\0]*"expr" {
    // match '<?teng expr'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_EXPR);
}

"<?"[[:space:]\0]*"ctype" {
    // match '<?teng ctype'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_CTYPE);
}

"<?"[[:space:]\0]*"endctype" {
    // match '<?teng endctype'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_ENDCTYPE);
}

"<?"[[:space:]\0]*"repeatfrag" {
    // match '<?teng repeat'
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_REPEATFRAG);
}

"<?"[[:space:]\0]*[[:alnum:]]* {
    // match '<?teng???'
    value.stringValue = string(yytext + 6, yyleng - 6);
    bufferPos.advance(yytext, yyleng);
    RETURN(LEX_TENG);
}

"?>" {
    // match '?>'
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_END);
}

"${" {
    // match '${' -- inline variable lookup
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_SHORT_EXPR);
}

"#{" {
    // match '#{' -- inline dictionary lookup
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_SHORT_DICT);
}

"@(" {
    // match '@(' -- fragment selector
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_FRAG_SEL);
}

"}" {
    // match '}'
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_SHORT_END);
}

"(" {
    // match left parenthesis
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_L_PAREN);
}

")" {
    // match right parenthesis
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_R_PAREN);
}

"[" {
    // match left bracket
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_L_BRACKET);
}

"]" {
    // match right bracket
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_R_BRACKET);
}

"." {
    // match selector
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_SELECTOR);
}

"#" {
    // match dictionary lookup
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_DICT);
}

"$" {
    // match variable lookup
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_VAR);
}

"@" {
    // match dictionary indirect lookup
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_DICT_INDIRECT);
}

"++" {
    // match concatenation operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_CONCAT);
}

"**" {
    // match repeat a pattern operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_REPEAT);
}

"," {
    // match comma
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_COMMA);
}

"?" {
    // match conditional expression operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_COND_EXPR);
}

":" {
    // match colon
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_COLON);
}

"=" {
    // match assignment operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_ASSIGN);
}

"==" {
    // match numeric eqaul operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_EQ);
}

"!=" {
    // match numeric not eqaul operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_NE);
}

">=" {
    // match greater or queal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_GE);
}

"<=" {
    // match less or queal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_LE);
}

">" {
    // match greater than operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_GT);
}

"<" {
    // match less than operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_LT);
}

"eq" {
    // match eqaul operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_EQ);
}

"ne" {
    // match not eqaul operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_NE);
}

"ge" {
    // match greater or queal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_GE);
}

"le" {
    // match less or queal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_LE);
}

"gt" {
    // match greater than operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_GT);
}

"lt" {
    // match less than operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_LT);
}

"=~" {
    // match string equal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_STR_EQ);
}

"!~" {
    // match string not equal operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_STR_NE);
}

"+" {
    // match numeric add operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_ADD);
}

"-" {
    // match numeric sub operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_SUB);
}

"*" {
    // match numeric mul operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_MUL);
}

"/" {
    // match numeric div operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_DIV);
}

"%" {
    // match numeric mod operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_MOD);
}

"!" {
    // match not operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_NOT);
}

"&" {
    // match bitwise and operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_BITAND);
}

"^" {
    // match bitwise xor operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_BITXOR);
}

"|" {
    // match bitwise or operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_BITOR);
}

"~" {
    // match bitwise not operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_BITNOT);
}

"&&" {
    // match and operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_AND);
}

"||" {
    // match or operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_OR);
}

"case" {
    // match case operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_CASE);
}

"defined" {
    // match defined operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_DEFINED);
}

"exists" {
    // match exist operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_EXIST);
}

"exist" {
    // match exist operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_EXIST);
}

"@jsonify" {
    // match @jsonify operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_JSONIFY);
}

"@type" {
    // match @type operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_TYPE);
}

"@count" {
    // match @count operator
    bufferPos.advanceColumn(yyleng);
    RETURN(LEX_COUNT);
}

"udf."{IDENT}(\.{IDENT})* {
    // match exist operator
    bufferPos.advanceColumn(yyleng);
    value.stringValue = string(yytext, yyleng);
    value.type = ParserValue_t::TYPE_STRING;
    RETURN(LEX_UDF_IDENT);
}

[\"\'] {
    // opening quote of string
    bufferPos.advanceColumn(yyleng);
    // prepare temporary string
    tmpSval.erase();

    stringOpener = *yytext;

    // change context to parse string content
    BEGIN(qstr);
}

<qstr>{
    [\"\'] {
        if (*yytext == stringOpener) {
            // regular end-of-string
            bufferPos.advanceColumn(yyleng);
            value.stringValue = tmpSval;
            // leave this context
            BEGIN(INITIAL);
            RETURN(LEX_STRING);
        }
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back(*yytext);
    }
    "\n" {
        // newline in string => bad token
        bufferPos.advanceColumn(yyleng);
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }
    "\\\n" {
        // newline escaped => bad token
        err.logError(Error_t::LL_ERROR, bufferPos,
            "Newline cannot be escaped");
        bufferPos.newLine();
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }
    <<EOF>> {
        // end of input => bad token
        err.logError(Error_t::LL_ERROR, bufferPos, "Unterminated string");
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }
    "\\n" {
        // escape for <LF>
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back('\n');
    }
    "\\r" {
        // escape for <CR>
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back('\r');
    }
    "\\t" {
        // escape for <TAB>
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back('\t');
    }
    "\\f" {
        // escape for <FF>
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back('\f');
    }
    "\\b" {
        // escape for <BEL>
        bufferPos.advanceColumn(yyleng);
        tmpSval.push_back('\b');
    }
    "\\". {
        // escape for other escape => put escaped char verbatim into token
        bufferPos.advance(yytext);
        tmpSval.push_back(yytext[1]);
    }
    [^\"\'\\\n]+ {
        // run of regular string characters => copy verbatim into token
        bufferPos.advance(yytext, yyleng);
        tmpSval.append(yytext, yyleng);
    }
    "\\" {
        // escape itself =>
        err.logError(Error_t::LL_ERROR, bufferPos, "Invalid escape at end of input");
        bufferPos.advanceColumn();
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }
}

{INTEGER} {
    // match integral number
    bufferPos.advanceColumn(yyleng);
    value.integerValue = strtoul(yytext, 0, 10);
    value.realValue = value.integerValue;
    value.stringValue = string(yytext, yyleng);
    value.type = ParserValue_t::TYPE_INT;
    RETURN(LEX_INT);
}

{HEX_INTEGER} {
    // match integral number
    bufferPos.advanceColumn(yyleng);
    value.integerValue = strtoul(yytext + 2, 0, 16);
    value.realValue = value.integerValue;
    char buff[100];
    snprintf(buff, sizeof(buff), "%zd", size_t(value.integerValue));
    value.stringValue = string(buff);
    value.type = ParserValue_t::TYPE_INT;
    RETURN(LEX_INT);
}

{BIN_INTEGER} {
    // match integral number
    bufferPos.advanceColumn(yyleng);
    value.integerValue = strtoul(yytext + 2, 0, 2);
    value.realValue = value.integerValue;
    char buff[100];
    snprintf(buff, sizeof(buff), "%zd", size_t(value.integerValue));
    value.stringValue = string(buff);
    value.type = ParserValue_t::TYPE_INT;
    RETURN(LEX_INT);
}

{REAL} {
    // match real number
    bufferPos.advanceColumn(yyleng);
    value.integerValue = ParserValue_t::int_t(value.realValue = atof(yytext));
    value.stringValue = string(yytext, yyleng);
    value.type = ParserValue_t::TYPE_REAL;
    RETURN(LEX_REAL);
}

{INTEGER}|{REAL}/[._[:alpha:]] {
    // match number (integer or real) followed by possible indentifier
    // or dot

    // copy number into tmp value
    tmpSval = string(yytext, yyleng);
    // start parsing of trailing characters
    BEGIN(badnumber);
}

{HEX_INTEGER}/[._] {
    // match hexa number followed by possible indentifier or dot

    // copy number into tmp value
    tmpSval = string(yytext, yyleng);
    // start parsing of trailing characters
    BEGIN(badnumber);
}

{BIN_INTEGER}/[2-9._[:alpha:]] {
    // match bin number followed by possible indentifier or dot

    // copy number into tmp value
    tmpSval = string(yytext, yyleng);
    // start parsing of trailing characters
    BEGIN(badnumber);
}

<badnumber>{
    [._[:alnum:]]+ {
        // run of number, dot or identifier characters composing
        // bad token
        value.stringValue = tmpSval + string(yytext, yyleng);
        err.logError(Error_t::LL_ERROR, bufferPos, "Invalid token '" +
            value.stringValue + "'");
        bufferPos.advance(value.stringValue);
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }

    [^._[:alnum:]]+ {
        // match any other sequence of characters
        // return them back to the input
        yyless(0);
        BEGIN(INITIAL);
    }
}

{IDENT} {
    // match identifier
    bufferPos.advanceColumn(yyleng);
    value.stringValue = string(yytext, yyleng);
    value.type = ParserValue_t::TYPE_STRING;
    RETURN(LEX_IDENT);
}

[[:space:]\0] {
    // match spaces -- spaces are ignored
    bufferPos.advance(*yytext);
}

. {
    // default rule
    value.stringValue = string(yytext, yyleng);
    err.logError(Error_t::LL_ERROR, bufferPos, "Unexpected character '"
                 + value.stringValue + "'");
    bufferPos.advance(yytext, yyleng);
    value.type = ParserValue_t::TYPE_STRING;
    RETURN(-1); //ERROR
}

"/*" {
    // comment start
    bufferPos.advanceColumn(yyleng);

    // change context to parse (ehm, ignore) comment's content
    BEGIN(comment);
}

<comment>{
    "*/" {
        // end of comment
        bufferPos.advanceColumn(yyleng);
        BEGIN(INITIAL);
    }

    <<EOF>> {
        // end of input => bad token
        err.logError(Error_t::LL_ERROR, bufferPos, "Unterminated comment");
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }

    .|"\n" {
        // ignore
        bufferPos.advanceColumn(yyleng);
    }
}

"//" {
    // comment start
    bufferPos.advanceColumn(yyleng);

    // change context to parse (ehm, ignore) comment's content
    BEGIN(one_line_comment);
}

<one_line_comment>{
    "\n" {
        // end of comment
        bufferPos.advanceColumn(yyleng);
        BEGIN(INITIAL);
    }

    <<EOF>> {
        // end of input => bad token
        err.logError(Error_t::LL_ERROR, bufferPos, "Unterminated comment");
        // leave this context
        BEGIN(INITIAL);
        RETURN(-1);
    }

    . {
        // ignore
        bufferPos.advanceColumn(yyleng);
    }
}

%%

namespace Teng {

Lex2_t::Lex2_t() : stackTop(0), yyscanner(0) {
    yylex_init((yyscan_t*)&yyscanner);
}

Lex2_t::~Lex2_t() {
    yylex_destroy((yyscan_t)yyscanner);
}

int Lex2_t::init(const string &src) {
    // test for stack overflow
    if (stackTop >= MAX_LEX_STACK_DEPTH)
        return -1;
    // create new flex buffer and push onto the stack
    bufferStack[stackTop++] = yy_scan_bytes(src.data(),
                                            src.length(),
                                            (yyscan_t*)yyscanner);

    // OK
    return 0;
}

int  Lex2_t::finish() {
    // destroy top buffer
    if (stackTop > 0)
        yy_delete_buffer((YY_BUFFER_STATE)bufferStack[--stackTop], yyscanner);
    // OK
    return 0;
}

} // namespace Teng

