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
 * $Id: tenglex1.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Teng template level #1 lexical analyzer.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (stepan)
 *             Created.
 */

#include <cstdio>
#include <cctype>

#include "tengerror.h"
#include "tenglex1.h"
#include "tengutil.h"

namespace Teng {

/** Initialize lexical analyzer from string.
  * @param input Input string.
  * @param filename File name identifying the original input source. */
Lex1_t::Lex1_t(const std::string &input, const std::string &fname)
        : input(input), position(0), filename(fname), line(1), column(0)
{}


/** Initialize lexical analyzer from file.
  * @param input Input file to read. */
Lex1_t::Lex1_t(const std::string &fname,
        const Error_t::Position_t &position,
        Error_t &error)
        : position(0), filename(fname), line(1), column(0)
{
    // normalize filename
    tengNormalizeFilename(filename);

    // open file
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == 0) {
        // error
        error.logSyscallError(Error_t::LL_ERROR, position,
                "Cannot open input file '" + filename + "'");
    }
    else {
        // read content from file
        char buf[1024];
        int i;
        while ((i = fread(buf, 1, sizeof(buf), fp)) > 0) {
            input.append(buf, i);
        }
        // close the file
        fclose(fp);
    }
}


/** Starts possible escape sequence, used by Lex1_t::unescapeInputSubstr.
  * @param state State of finine automaton.
  * @param s Output string.
  * @param c Next input char. */
inline static void newSequence(int &state, std::string &s, char c) {
    switch (c) {
        case '$': state = 1; break;  // $\{
        case '\\': state = 3; break; // \}
        case '<': state = 4; break;  // <\?
        case '#': state = 6; break;  // #\{
        case '?': state = 8; break;  // ?\>
        default:
            state = 0;
            s.push_back(c);
            break;
    }
}


/** Unescape substring of input string variable.
 * changes only $\{, \}, <\?, #\{, ?\> to ${, }, <?, #{, ?>
 * @param begin start position in string input.
 * @param end final position in string input + 1.
 * @return unescaped substring */
std::string Lex1_t::unescapeInputSubstr(unsigned int begin, unsigned int end) {
    int state = 0;
    /*
      state
      1 $
      2 $\
      3 \ (and not $\, <\, #\, ?\ )
      4 <
      5 <\
      6 #
      7 #\
      8 ?
      9 ?\
      0 other
    */
    std::string s;
    s.reserve(end - begin);
    if (end >= input.size()) end = input.size();
    for (; begin < end; begin++) {
        switch (state) {
            case 1: // $
                if (input[begin] == '\\') {
                    state = 2;
                } else {
                    s.push_back('$');
                    newSequence(state, s, input[begin]);
                }
                break;

            case 2: /* $\ */
                if (input[begin] == '{') {
                    s.append("${");
                    state = 0;
                } else {
                    s.push_back('$');
                    state = 3;
                    goto state3;
                }
                break;

            case 3: /* \ (and not $\, <\, #\, ?\ ) */
            state3:;
                if (input[begin] == '}') {
                    s.push_back('}');
                    state = 0;
                } else {
                    s.push_back('\\');
                    newSequence(state, s, input[begin]);
                }
                break;

            case 4: // <
                if (input[begin] == '\\') {
                    state = 5;
                } else {
                    s.push_back('<');
                    newSequence(state, s, input[begin]);
                }
                break;

            case 5: /* <\ */
                if (input[begin] == '?') {
                    s.append("<?");
                    state = 0;
                } else {
                    s.push_back('<');
                    state = 3;
                    goto state3;
                }
                break;

            case 6: // #
                if (input[begin] == '\\') {
                    state = 7;
                } else {
                    s.push_back('#');
                    newSequence(state, s, input[begin]);
                }
                break;

            case 7: /* #\ */
                if (input[begin] == '{') {
                    s.append("#{");
                    state = 0;
                } else {
                    s.push_back('#');
                    state = 3;
                    goto state3;
                }
                break;

            case 8: // ?
                if (input[begin] == '\\') {
                    state = 9;
                } else {
                    s.push_back('?');
                    newSequence(state, s, input[begin]);
                }
                break;

            case 9: /* ?\ */
                if (input[begin] == '>') {
                    s.append("?>");
                    state = 0;
                } else {
                    s.push_back('?');
                    state = 3;
                    goto state3;
                }
                break;

            default:
                newSequence(state, s, input[begin]);
                break;
        }
    }

    //  do not forget flush the buffer at the end
    switch (state) {
        case 1: s.push_back('$'); break;
        case 2: s.append("$\\"); break;
        case 3: s.push_back('\\'); break;
        case 4: s.push_back('<'); break;
        case 5: s.append("<\\"); break;
        case 6: s.push_back('#'); break;
        case 7: s.append("#\\"); break;
        case 8: s.push_back('?'); break;
        case 9: s.append("?\\"); break;
    }

    return s;
}

/** Get next token.
  * @return Token struct of next token. */
Lex1_t::Token_t Lex1_t::getElement(bool shortTag) {
    // backup start values
    size_t start_pos = position;
    size_t start_line = line;
    size_t start_column = column;

    // go up to element start
    while (position < input.length()) {

        // test for elements
        if (position + 4 < input.length()
                && input[position] == '<'
                && input[position + 1] == '!'
                && input[position + 2] == '-'
                && input[position + 3] == '-'
                && input[position + 4] == '-') {
            // comment
            // test for end of text token
            if (position > start_pos)
                return Token_t(TYPE_TEXT,
                        unescapeInputSubstr(start_pos, position),
                        start_line, start_column);
            // skip "<!---"
            incrementPosition(5);

            // skip input until "--->"
            while (position < input.length()) {
                // test for comment end
                if (position + 3 < input.length()
                        && input[position] == '-'
                        && input[position + 1] == '-'
                        && input[position + 2] == '-'
                        && input[position + 3] == '>') {
                    break; //comment end
                }
                incrementPosition(1); //next char
            }
            // if comment end not found
            if (position >= input.length()) {
                return Token_t(TYPE_ERROR,
                        "Unterminated comment",
                        start_line, start_column);
            }
            // skip "--->"
            incrementPosition(4);
            // reset start pointers
            start_pos = position;
            start_line = line;
            start_column = column;
            // continue with next token
            continue;

        }
        else if (position + 6 < input.length()
                && input[position] == '<'
                && input[position + 1] == '?'
                && input[position + 2] == 't'
                && input[position + 3] == 'e'
                && input[position + 4] == 'n'
                && input[position + 5] == 'g'
                && (isspace(input[position + 6]) //accept "<?teng "
                || (position + 7 < input.length()
                && input[position + 6] == '?'
                && input[position + 7] == '>'))) { //accept "<?teng?>"
            // "<?teng" directive
            // test for end of text token
            if (position > start_pos)
                return Token_t(TYPE_TEXT,
                        unescapeInputSubstr(start_pos, position),
                        start_line, start_column);
            // skip "<?teng"
            incrementPosition(6);

            // skip input until "?>"
            int escape = 0, inString = 0;
            while (position < input.length()) {
                if (inString) {
                    if (input[position] == '"')
                       if (!escape) inString = 0;
                    if (input[position] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (position + 1 < input.length()
                        && input[position] == '?'
                        && input[position + 1] == '>') break;
                    if (input[position] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (position >= input.length()) {
                return Token_t(TYPE_ERROR,
                        "Unterminated <?teng ...?> directive",
                        start_line, start_column);
            }
            // skip "?>"
            incrementPosition(2);
            // return teng token
            return Token_t(TYPE_TENG,
                    input.substr(start_pos, position - start_pos),
                    start_line, start_column);

        }
        else if ( shortTag == true
                && position + 1 < input.length()
                && input[position] == '<'
                && input[position + 1] == '?') {
            // "<?teng" directive
            // test for end of text token
            if (position > start_pos)
                return Token_t(TYPE_TEXT,
                        unescapeInputSubstr(start_pos, position),
                        start_line, start_column);
            // skip "<?teng"
            incrementPosition(2);

            // skip input until "?>"
            int escape = 0, inString = 0;
            while (position < input.length()) {
                if (inString) {
                    if (input[position] == '"')
                       if (!escape) inString = 0;
                    if (input[position] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (position + 1 < input.length()
                        && input[position] == '?'
                        && input[position + 1] == '>') break;
                    if (input[position] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (position >= input.length()) {
                return Token_t(TYPE_ERROR,
                        "Unterminated <? ...?> directive",
                        start_line, start_column);
            }
            // skip "?>"
            incrementPosition(2);
            // return teng token
            return Token_t(TYPE_TENG_SHORT,
                    input.substr(start_pos, position - start_pos),
                    start_line, start_column);

        }
        else if (position + 1 < input.length()
                && input[position] == '$'
                && input[position + 1] == '{') {
            // shorted expression form
            // test for end of text token
            if (position > start_pos)
                return Token_t(TYPE_TEXT,
                        unescapeInputSubstr(start_pos, position),
                        start_line, start_column);
            // skip "${"
            incrementPosition(2);

            // skip input until "}", except escaped "\}"
            int escape = 0, inString = 0;
            while (position < input.length()) {
                if (inString) {
                    if (input[position] == '"')
                       if (!escape) inString = 0;
                    if (input[position] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (input[position] == '}') break;
                    if (input[position] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (position >= input.length()) {
                return Token_t(TYPE_ERROR,
                        "Unterminated ${...} directive",
                        start_line, start_column);
            }
            // skip "}"
            incrementPosition(1);
            // return expression token
            return Token_t(TYPE_EXPR,
                    input.substr(start_pos, position - start_pos),
                    start_line, start_column);

        }
        else if (position + 1 < input.length()
                && input[position] == '#'
                && input[position + 1] == '{') {
            // shorted dictionary item form
            // test for end of text token
            if (position > start_pos)
                return Token_t(TYPE_TEXT,
                        unescapeInputSubstr(start_pos, position),
                        start_line, start_column);
            // skip "#{"
            incrementPosition(2);

            // skip input until "}", except escaped \}
            int escape = 0;
            while (position < input.length()) {
                // test for directive end
                if (escape) {
                    escape = 0;
                    incrementPosition(1); //skip next char
                    continue;
                } else if (input[position] == '}') {
                    break; //directive end
                } else if (input[position] == '\\') {
                    escape = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (position >= input.length()) {
                return Token_t(TYPE_ERROR,
                        "Unterminated #{...} directive",
                        start_line, start_column);
            }
            // skip "}"
            incrementPosition(1);
            // return dict item token
            return Token_t(TYPE_DICT,
                    input.substr(start_pos, position - start_pos),
                    start_line, start_column);

        }
        else {
            // text
            incrementPosition(1);
        }

    } //end while

    // test for end of text token
    if (position > start_pos)
        return Token_t(TYPE_TEXT,
                unescapeInputSubstr(start_pos, position),
                start_line, start_column);

    // return end token
    return Token_t(TYPE_EOF,
            "End of input stream",
            start_line, start_column);
}


/** Get error position.
  * @return Error position. */
Error_t::Position_t Lex1_t::getPosition() const {
    return Error_t::Position_t(filename, line, column);
}


/** Advance actual position by one char
  * and also update line and column pointers.
  * @param num Increment position by this number of chars. */
void Lex1_t::incrementPosition(int num) {
    // repeat num-times
    while (num -- > 0) {

        // check end
        if (position < input.length()) {

            // advance line & column pointers
            if (input[position] == '\n') {
                ++ line; //inc line
                column = 0; //reset column
            } else if (input[position] == '\t') {
                column = ((column / 8) + 1) * 8; //skip to next tab
            } else
                ++ column;

            // next char in input
            ++ position;
        }
    }
}

} // namespace Teng

