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
 * $Id: tenglex1.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
 *
 * DESCRIPTION
 * Teng level #1 lexical analyzer.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (stepan)
 *             Created.
 */

#ifndef TENGLEX1_H
#define TENGLEX1_H

#include <string>

#include "tengerror.h"

using namespace std;

namespace Teng {

class Lex1_t {

public:
        
    /** Possible token types. */
    enum Type_t {
        TYPE_EOF, /**< End of input, last token. */
        TYPE_ERROR, /**< Parse error, token.value contains error message. */
        TYPE_TEXT, /**< General text. */
        TYPE_TENG, /**< Teng directive. */
        TYPE_EXPR, /**< Shorted expression form. */
        TYPE_DICT, /**< Shorted dictionary item form. */
    };
    
    /** Token type, returned by the get() method. */
    struct Token_t {
        Type_t type; /**< Token Type. */
        string value; /**< Value of the token. */
        int line; /**< Line number of the token start. */
        int column; /**< Column number of the token start. */
        
        /** Construct structure with initialized members. */
        inline Token_t(Type_t t, const string &v, int l, int c)
            : type(t), value(v), line(l), column(c)
        {}
    };
    
    /** Initialize lexical analyzer from string.
      * @param input Input string.
      * @param filename File from which the string was taken. */
    Lex1_t(const string &input, const string &filename);
    
    /** Initialize lexical analyzer from file.
      * @param filename Input file to read.
      * @param position Position in source file.
      * @param error Error log class for dumping possible errors. */
    Lex1_t(const string &filename,
           const Error_t::Position_t &position,
           Error_t &error);
    
     /** Unescape substring of input string variable.
      * @param begin start position in string input.
      * @param end final position in string input + 1.
      * @return unescaped substring */   
    string Lex1_t::unescapeInputSubstr(unsigned int begin, unsigned int end);
    
    /** Get next token.
      * @return Token struct of next token. */
    Token_t getElement();

    /** Get error position.
      * @return Error position. */
    Error_t::Position_t getPosition() const;
        
    
private:

    /** Advance actual position by one char
      * and also update line and column pointers.
      * @param num Increment position by this number of chars. */
    void incrementPosition(int num);

    /** Get text token from current post */
    Token_t getText(int end);

    /** Input string passed on init. */
    string input;
    /** Position within the input string. */
    size_t position;
    /** File name identifying the original source. */
    string filename;
    /** Actual line number. */
    size_t line;
    /** Actual column number. */
    size_t column;
};

} // namespace Teng

#endif // TENGLEX1_H
