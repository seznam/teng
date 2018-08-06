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
#include <fstream>

#include "tenglogging.h"
#include "tengerror.h"
#include "tenglex1.h"
#include "tengutil.h"

namespace Teng {
namespace Parser {
namespace {

/** Unescape substring of input string variable.
 *
 * Transformations:
 *     $\{ -> ${
 *     #\{ -> #{
 *     ?\> -> ?>
 *     \}  -> {
 *     <\? -> <?
 *
 * It never allocates any memory because it writes unescaped string into given
 * interval.
 */
string_view_t
unescape(flex_string_value_t &str, std::size_t start, std::size_t end) {
    enum {
        initial,
        dollar_expected_backslash_lcurly,
        dollar_backslash_expected_lcurly,
        backslash_expected_rcurly,
        lt_expected_backslash_question,
        lt_backslash_expected_question,
        hash_expected_backslash_lcurly,
        hash_backslash_expected_lcurly,
        question_expected_backslash_gt,
        question_backslash_expected_gt,
    } state = initial;

    char *istr = str.data() + start;
    char *estr = str.data() + end;
    const char *ires = istr;
    char *eres = estr;
    const char *elast_seq = istr;
    const char *icur_seq = istr;

    auto do_unescape = [&] (const char *ipos, string_view_t seq) {
        eres = eres == estr
            ? istr + std::distance(elast_seq, icur_seq)
            : std::copy(elast_seq, icur_seq, eres);
        eres = std::copy(seq.begin(), seq.end(), eres);
        elast_seq = ++ipos;
    };

    for (auto ipos = istr; ipos != estr; ++ipos) {
        switch (*ipos) {
        case '$':
            //!< .$\{ -> $.\{
            state = dollar_expected_backslash_lcurly;
            icur_seq = ipos;
            break;

        case '#':
            //!< .#<\{ -> #.\{
            state = hash_expected_backslash_lcurly;
            icur_seq = ipos;
            break;

        case '<':
            //!< .<\? -> <.\?
            state = lt_expected_backslash_question;
            icur_seq = ipos;
            break;

        case '\\':
            switch (state) {
            // $.\{ -> $\.{
            case dollar_expected_backslash_lcurly:
                state = dollar_backslash_expected_lcurly;
                break;
            // <.\? -> <\.?
            case lt_expected_backslash_question:
                state = lt_backslash_expected_question;
                break;
            // #.\{ -> #\.{
            case hash_expected_backslash_lcurly:
                state = hash_backslash_expected_lcurly;
                break;
            // ?.\> -> ?\.>
            case question_expected_backslash_gt:
                state = question_backslash_expected_gt;
                break;
            // .\{ -> \.{
            default:
                state = backslash_expected_rcurly;
                icur_seq = ipos;
                break;
            }
            break;

        case '?':
            switch (state) {
            // <\.? -> <\?.
            case lt_backslash_expected_question:
                do_unescape(ipos, "<?");
                state = initial;
                break;
            // .?\> -> ?.\>
            default:
                state = question_expected_backslash_gt;
                icur_seq = ipos;
                break;
            }
            break;

        case '>':
            switch (state) {
            // ?\.> -> ?\>.
            case question_backslash_expected_gt:
                do_unescape(ipos, "?>");
                break;
            // X.> -> X>.
            default:
                break;
            }
            state = initial;
            break;

        case '{':
            switch (state) {
            //!< $\.{ -> $\{.
            case dollar_backslash_expected_lcurly:
                do_unescape(ipos, "${");
                break;
            //!< #\.{ -> #\{.
            case hash_backslash_expected_lcurly:
                do_unescape(ipos, "#{");
                break;
            // X.{ -> X{.
            default:
                break;
            }
            state = initial;
            break;

        case '}':
            switch (state) {
            // X\.} -> X\}.
            case dollar_backslash_expected_lcurly:
            case lt_backslash_expected_question:
            case hash_backslash_expected_lcurly:
            case question_backslash_expected_gt:
                // remove first char from escape sequence
                ++icur_seq;
                [[fallthrough]];
            // \.} -> \}.
            case backslash_expected_rcurly:
                do_unescape(ipos, "}");
                break;
            // \.X -> \X.
            default:
                break;
            }
            state = initial;
            break;
        default:
            state = initial;
            break;
        };
    }
    if (eres != estr)
        eres = std::copy<const char *>(elast_seq, estr, eres);
    return {ires, eres};
}

} // namespace

Lex1_t::Token_t Lex1_t::next(bool accept_short_directive) {
    // backup start values
    std::size_t start_pos = offset;
    int32_t start_line = pos.lineno;
    int32_t start_column = pos.colno;

    auto incrementPosition = [&] (int num) {
        while (num-- > 0) {
            if (offset < source_code.size()) {
                pos.advance(source_code[offset]);
                ++offset;
            }
        }
    };

    // go up to element start
    while (offset < source_code.size()) {

        // test for elements
        if (offset + 4 < source_code.size()
                && source_code[offset] == '<'
                && source_code[offset + 1] == '!'
                && source_code[offset + 2] == '-'
                && source_code[offset + 3] == '-'
                && source_code[offset + 4] == '-') {
            // comment
            // test for end of text token
            if (offset > start_pos) {
                return {
                    LEX1::TEXT,
                    {pos.filename, start_line, start_column},
                    unescape(source_code, start_pos, offset)
                };
            }
            // skip "<!---"
            incrementPosition(5);

            // skip input until "--->"
            while (offset < source_code.size()) {
                // test for comment end
                if (offset + 3 < source_code.size()
                        && source_code[offset] == '-'
                        && source_code[offset + 1] == '-'
                        && source_code[offset + 2] == '-'
                        && source_code[offset + 3] == '>') {
                    break; //comment end
                }
                incrementPosition(1); //next char
            }
            // if comment end not found
            if (offset >= source_code.size()) {
                return {
                    LEX1::ERROR,
                    {pos.filename, start_line, start_column},
                    "Unterminated comment"
                };
            }
            // skip "--->"
            incrementPosition(4);
            // reset start pointers
            start_pos = offset;
            start_line = pos.lineno;
            start_column = pos.colno;
            // continue with next token
            continue;

        }
        else if (offset + 6 < source_code.size()
                && source_code[offset] == '<'
                && source_code[offset + 1] == '?'
                && source_code[offset + 2] == 't'
                && source_code[offset + 3] == 'e'
                && source_code[offset + 4] == 'n'
                && source_code[offset + 5] == 'g'
                && (isspace(source_code[offset + 6]) //accept "<?teng "
                || (offset + 7 < source_code.size()
                && source_code[offset + 6] == '?'
                && source_code[offset + 7] == '>'))) { //accept "<?teng?>"
            // "<?teng" directive
            // test for end of text token
            if (offset > start_pos)
                return {
                    LEX1::TEXT,
                    {pos.filename, start_line, start_column},
                    unescape(source_code, start_pos, offset)
                };
            // skip "<?teng"
            incrementPosition(6);

            // skip input until "?>"
            int escape = 0, inString = 0;
            while (offset < source_code.size()) {
                if (inString) {
                    if (source_code[offset] == '"')
                       if (!escape) inString = 0;
                    if (source_code[offset] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (offset + 1 < source_code.size()
                        && source_code[offset] == '?'
                        && source_code[offset + 1] == '>') break;
                    if (source_code[offset] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (offset >= source_code.size()) {
                return {
                    LEX1::ERROR,
                    {pos.filename, start_line, start_column},
                    "Unterminated <?teng ...?> directive"
                };
            }
            // skip "?>"
            incrementPosition(2);
            // return teng token
            return {
                LEX1::TENG,
                {pos.filename, start_line, start_column},
                {source_code, start_pos, offset - start_pos}
            };

        }
        else if (accept_short_directive == true
                && offset + 1 < source_code.size()
                && source_code[offset] == '<'
                && source_code[offset + 1] == '?') {
            // "<?teng" directive
            // test for end of text token
            if (offset > start_pos)
                return {
                    LEX1::TEXT,
                    {pos.filename, start_line, start_column},
                    unescape(source_code, start_pos, offset)
                };
            // skip "<?teng"
            incrementPosition(2);

            // skip input until "?>"
            int escape = 0, inString = 0;
            while (offset < source_code.size()) {
                if (inString) {
                    if (source_code[offset] == '"')
                       if (!escape) inString = 0;
                    if (source_code[offset] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (offset + 1 < source_code.size()
                        && source_code[offset] == '?'
                        && source_code[offset + 1] == '>') break;
                    if (source_code[offset] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (offset >= source_code.size()) {
                return {
                    LEX1::ERROR,
                    {pos.filename, start_line, start_column},
                    "Unterminated <? ...?> directive"
                };
            }
            // skip "?>"
            incrementPosition(2);
            // return teng token
            return {
                LEX1::TENG_SHORT,
                {pos.filename, start_line, start_column},
                {source_code, start_pos, offset - start_pos}
            };

        }
        else if (offset + 1 < source_code.size()
                && source_code[offset] == '$'
                && source_code[offset + 1] == '{') {
            // shorted expression form
            // test for end of text token
            if (offset > start_pos)
                return {
                    LEX1::TEXT,
                    {pos.filename, start_line, start_column},
                    unescape(source_code, start_pos, offset),
                };
            // skip "${"
            incrementPosition(2);

            // skip input until "}", except escaped "\}"
            int escape = 0, inString = 0;
            while (offset < source_code.size()) {
                if (inString) {
                    if (source_code[offset] == '"')
                       if (!escape) inString = 0;
                    if (source_code[offset] == '\\') escape = !escape;
                    else escape = 0;
                }
                else {
                    if (source_code[offset] == '}') break;
                    if (source_code[offset] == '"') inString = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (offset >= source_code.size()) {
                return {
                    LEX1::ERROR,
                    {pos.filename, start_line, start_column},
                    "Unterminated ${...} directive"
                };
            }
            // skip "}"
            incrementPosition(1);
            // return expression token
            return {
                LEX1::EXPR,
                {pos.filename, start_line, start_column},
                {source_code, start_pos, offset - start_pos}
            };

        }
        else if (offset + 1 < source_code.size()
                && source_code[offset] == '#'
                && source_code[offset + 1] == '{') {
            // shorted dictionary item form
            // test for end of text token
            if (offset > start_pos)
                return {
                    LEX1::TEXT,
                    {pos.filename, start_line, start_column},
                    unescape(source_code, start_pos, offset),
                };
            // skip "#{"
            incrementPosition(2);

            // skip input until "}", except escaped \}
            int escape = 0;
            while (offset < source_code.size()) {
                // test for directive end
                if (escape) {
                    escape = 0;
                    incrementPosition(1); //skip next char
                    continue;
                } else if (source_code[offset] == '}') {
                    break; //directive end
                } else if (source_code[offset] == '\\') {
                    escape = 1;
                }
                incrementPosition(1); //next char
            }
            // if directive end not found
            if (offset >= source_code.size()) {
                return {
                    LEX1::ERROR,
                    {pos.filename, start_line, start_column},
                    "Unterminated #{...} directive"
                };
            }
            // skip "}"
            incrementPosition(1);
            // return dict item token
            return {
                LEX1::DICT,
                {pos.filename, start_line, start_column},
                {source_code, start_pos, offset - start_pos}
            };

        }
        else {
            // text
            incrementPosition(1);
        }

    } //end while

    // test for end of text token
    if (offset > start_pos) {
        return {
            LEX1::TEXT,
            {pos.filename, start_line, start_column},
            unescape(source_code, start_pos, offset)
        };
    }

    // return end token
    return {
        LEX1::END_OF_INPUT,
        {pos.filename, start_line, start_column},
        "End of input stream"
    };
}

} // namespace Parser
} // namespace Teng

