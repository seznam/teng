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
 * $Id$
 *
 * DESCRIPTION
 * Level 2 lexer.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             First version.
 */

#include "tengutil.h"
#include "tengsyntax.hh"
#include "tenglex2impl.h"
#include "tenglex2.h"

namespace Teng {
namespace Parser {

string_view_t l2_token_name(int token_id) {
    switch (LEX2(token_id)) {
    case LEX2::TEXT: return "TEXT";
    case LEX2::DEBUG_FRAG: return "DEBUG_FRAG";
    case LEX2::BYTECODE_FRAG: return "BYTECODE_FRAG";
    case LEX2::INCLUDE: return "INCLUDE";
    case LEX2::FORMAT: return "FORMAT";
    case LEX2::ENDFORMAT: return "ENDFORMAT";
    case LEX2::FRAGMENT: return "FRAGMENT";
    case LEX2::ENDFRAGMENT: return "ENDFRAGMENT";
    case LEX2::IF: return "IF";
    case LEX2::ELSEIF: return "ELSEIF";
    case LEX2::ELSE: return "ELSE";
    case LEX2::ENDIF: return "ENDIF";
    case LEX2::SET: return "SET";
    case LEX2::EXPR: return "EXPR";
    case LEX2::TENG: return "TENG";
    case LEX2::END: return "END";
    case LEX2::SHORT_EXPR: return "SHORT_EXPR";
    case LEX2::SHORT_DICT: return "SHORT_DICT";
    case LEX2::SHORT_END: return "SHORT_END";
    case LEX2::CTYPE: return "CTYPE";
    case LEX2::ENDCTYPE: return "ENDCTYPE";
    case LEX2::ASSIGN: return "ASSIGN";
    case LEX2::COMMA: return "COMMA";
    case LEX2::COND_EXPR: return "COND_EXPR";
    case LEX2::COLON: return "COLON";
    case LEX2::OR: return "OR";
    case LEX2::OR_DIGRAPH: return "OR_DIGRAPH";
    case LEX2::AND: return "AND";
    case LEX2::AND_TRIGRAPH: return "AND_TRIGRAPH";
    case LEX2::BITOR: return "BITOR";
    case LEX2::BITXOR: return "BITXOR";
    case LEX2::BITAND: return "BITAND";
    case LEX2::EQ: return "EQ";
    case LEX2::EQ_DIGRAPH: return "EQ_DIGRAPH";
    case LEX2::NE: return "NE";
    case LEX2::NE_DIGRAPH: return "NE_DIGRAPH";
    case LEX2::STR_EQ: return "STR_EQ";
    case LEX2::STR_NE: return "STR_NE";
    case LEX2::GE: return "GE";
    case LEX2::GE_DIGRAPH: return "GE_DIGRAPH";
    case LEX2::LE: return "LE";
    case LEX2::LE_DIGRAPH: return "LE_DIGRAPH";
    case LEX2::GT: return "GT";
    case LEX2::GT_DIGRAPH: return "GT_DIGRAPH";
    case LEX2::LT: return "LT";
    case LEX2::LT_DIGRAPH: return "LT_DIGRAPH";
    case LEX2::PLUS: return "PLUS";
    case LEX2::MINUS: return "MINUS";
    case LEX2::CONCAT: return "CONCAT";
    case LEX2::MUL: return "MUL";
    case LEX2::DIV: return "DIV";
    case LEX2::MOD: return "MOD";
    case LEX2::REPEAT: return "REPEAT";
    case LEX2::NOT: return "NOT";
    case LEX2::BITNOT: return "BITNOT";
    case LEX2::HIGHEST_PREC: return "HIGHEST_PREC";
    case LEX2::CASE: return "CASE";
    case LEX2::DEFINED: return "DEFINED";
    case LEX2::ISEMPTY: return "ISEMPTY";
    case LEX2::EXISTS: return "EXISTS";
    case LEX2::JSONIFY: return "JSONIFY";
    case LEX2::TYPE: return "TYPE";
    case LEX2::COUNT: return "COUNT";
    case LEX2::L_PAREN: return "L_PAREN";
    case LEX2::R_PAREN: return "R_PAREN";
    case LEX2::L_BRACKET: return "L_BRACKET";
    case LEX2::R_BRACKET: return "R_BRACKET";
    case LEX2::VAR: return "VAR";
    case LEX2::DICT: return "DICT";
    case LEX2::DICT_INDIRECT: return "DICT_INDIRECT";
    case LEX2::SELECTOR: return "SELECTOR";
    case LEX2::UDF_IDENT: return "UDF_IDENT";
    case LEX2::IDENT: return "IDENT";
    case LEX2::STRING: return "STRING";
    case LEX2::REGEX: return "REGEX";
    case LEX2::DEC_INT: return "DEC_INT";
    case LEX2::HEX_INT: return "HEX_INT";
    case LEX2::BIN_INT: return "BIN_INT";
    case LEX2::REAL: return "REAL";
    case LEX2::INVALID: return "INVALID";
    case LEX2::BUILTIN_FIRST: return "BUILTIN_FIRST";
    case LEX2::BUILTIN_INNER: return "BUILTIN_INNER";
    case LEX2::BUILTIN_LAST: return "BUILTIN_LAST";
    case LEX2::BUILTIN_INDEX: return "BUILTIN_INDEX";
    case LEX2::BUILTIN_COUNT: return "BUILTIN_COUNT";
    case LEX2::BUILTIN_THIS: return "BUILTIN_THIS";
    case LEX2::BUILTIN_PARENT: return "BUILTIN_PARENT";
    }
    return token_id == LEX2_EOF? "<EOF>": "<UNEXPECTED>";
}

std::ostream &operator<<(std::ostream &os, const Token_t &token) {
    os << "(level=2"
       << ", id=" << static_cast<int>(token.token_id)
       << ", name=" << token.token_name()
       << ", view='" << token.view() << "'"
       << ", at=" << token.pos << ")";
    return os;
}

namespace {

flex_string_value_t empty_directive(0);

auto scan(flex_string_view_t &d, void *yyscanner) {
    return teng__scan_buffer(d.data(), d.flex_size(), yyscanner);
}

} // namespace

Lex2_t::Lex2_t(Error_t &err)
  : yyscanner(nullptr), buffer(nullptr),
    err(err), pos(), token_pos(), token_ipos(nullptr),
    directive(empty_directive)
{
    if (teng_lex_init(&yyscanner))
        throw std::runtime_error("can't initalize lex2: " + strerr(errno));
}

Lex2_t::~Lex2_t() {
    if (buffer) finish_scanning();
    teng_lex_destroy((yyscan_t)yyscanner);
}

void
Lex2_t::start_scanning(
    flex_string_view_t &&new_directive,
    const Pos_t &init_pos
) {
    if (buffer != nullptr)
        throw std::runtime_error("another buffer in lex2 not finished yet");
    pos = init_pos;
    directive = std::move(new_directive);
    if (!(buffer = scan(directive, yyscanner)))
        throw std::runtime_error("can't allocate flex buffer in lex2");
}

void Lex2_t::finish_scanning() {
    directive.reset();
    teng__delete_buffer((YY_BUFFER_STATE)buffer, yyscanner);
    buffer = nullptr;
}

} // namespace Parser
} // namespace Teng

