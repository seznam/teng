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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser diagnostic codes.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#include "lex2.h"
#include "syntax.hh"
#include "logging.h"
#include "parsercontext.h"
#include "parserdiag.h"

namespace Teng {
namespace Parser {

void ExprDiagEntry_t::log_case(Context_t *ctx, const Token_t &token) {
    switch (token) {
    case LEX2_EOF:
    case LEX2::END:
    case LEX2::SHORT_END:
        logDiag(ctx, token.pos, "Missing closing ')' in case expression");
        break;
    default:
        switch (code) {
        case diag_code::case_cond:
            logDiag(ctx, pos, "Invalid condition in case expression");
            break;
        case diag_code::case_option:
            logDiag(ctx, pos, "Invalid case option literal");
            break;
        case diag_code::case_option_branch:
            logDiag(ctx, pos, "Invalid case branch expression");
            break;
        case diag_code::case_default_branch:
            logDiag(ctx, pos, "Invalid case default branch expression");
            break;
        default:
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        break;
    }
}

void ExprDiagEntry_t::log_tern(Context_t *ctx, const Token_t &token) {
    switch (code) {
    case diag_code::tern_colon:
        logDiag(
            ctx,
            token.pos,
            "Missing colon token of ternary operator"
        );
        break;
    case diag_code::tern_true_branch:
        switch (token) {
        case LEX2_EOF:
        case LEX2::SHORT_END:
        case LEX2::COLON:
            logDiag(
                ctx,
                token.pos,
                "Missing expression in true branch of ternary operator"
            );
            break;
        default:
            logDiag(
                ctx,
                token.pos,
                "Invalid expression in true branch of ternary operator"
            );
            break;
        }
        break;
    case diag_code::tern_false_branch:
        switch (token) {
        case LEX2_EOF:
        case LEX2::SHORT_END:
            logDiag(
                ctx,
                token.pos,
                "Missing expression in false branch of ternary operator"
            );
            break;
        default:
            logDiag(
                ctx,
                token.pos,
                "Invalid expression in false branch of ternary operator"
            );
            break;
        }
        break;
    default:
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }
}

void ExprDiagEntry_t::log(Context_t *ctx, const Token_t &token) {
    switch (code) {
    case diag_code::sentinel:
        logDiag(ctx, pos, "Unmatched sentinel diagnostic code");
        break;
    case diag_code::fun_args:
        switch (token) {
        case LEX2_EOF:
        case LEX2::END:
        case LEX2::SHORT_END:
            logDiag(ctx, token.pos, "Missing closing ')' in function call");
            break;
        default:
            logDiag(ctx, pos, "Invalid function arguments");
            break;
        }
        break;
    case diag_code::case_cond:
    case diag_code::case_option:
    case diag_code::case_option_branch:
    case diag_code::case_default_branch:
        log_case(ctx, token);
        break;
    case diag_code::tern_colon:
    case diag_code::tern_true_branch:
    case diag_code::tern_false_branch:
        log_tern(ctx, token);
        break;
    }
}

void ExprDiag_t::log_unexpected_token(Context_t *ctx) {
    auto pos = ctx->unexpected_token.pos;
    switch (ctx->unexpected_token) {
    case LEX2::ENDIF:
        logWarning(
            ctx,
            pos,
            "The <?teng endif?> directive closes unopened if block"
        );
        break;
    case LEX2::ENDCTYPE:
        logWarning(
            ctx,
            pos,
            "The <?teng endctype?> directive closes unopened ctype block"
        );
        break;
    case LEX2::ENDFORMAT:
        logWarning(
            ctx,
            pos,
            "The <?teng endformat?> directive closes unopened format block"
        );
        break;
    case LEX2::ENDFRAGMENT:
        logWarning(
            ctx,
            pos,
            "The <?teng endfrag?> directive closes unopened fragment block"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Unexpected token: name="
            + ctx->unexpected_token.token_name()
            + ", view=" + ctx->unexpected_token.str()
        );
        break;
    }
}

void ExprDiag_t::unwind(Context_t *ctx, const Token_t &token) {
    while (!entries.empty()) {
        if (entries.back().is_sentinel()) {
            entries.pop_back();
            break;
        } else {
            entries.back().log(ctx, token);
            entries.pop_back();
        }
    }
}

} // namespace Parser
} // namespace Teng

