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
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#include "yystype.h"
#include "syntax.hh"
#include "program.h"
#include "logging.h"
#include "instruction.h"
#include "parsercontext.h"
#include "configuration.h"
#include "semanticother.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {

void include_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts) {
    // ensure that file option exists
    auto iopt = opts.find("file");
    if (iopt == opts.end()) {
        logError(ctx, pos, "Can't include file; the 'file' option is missing");
        return;
    }

    // ensure that we not go beyond include limit
    if (ctx->lex1_stack.size() >= ctx->params->getMaxIncludeDepth()) {
        logError(ctx, pos, "Can't include file; include level is too deep");
        return;
    }

    // compile file (append compiled file at the end of current program)
    ctx->load_file(iopt->value.string(), pos);
}

void ignore_include(Context_t *ctx, const Token_t &token, bool empty) {
    if (empty) {
        logError(
            ctx,
            token.pos,
            "Missing filename to include; ignoring the include directive"
        );
    } else {
        switch (ctx->unexpected_token) {
        case LEX2_EOF:
        case LEX2::END:
            logError(
                ctx,
                token.pos,
                "Premature end of <?teng include?> directive; "
                "ignoring the include directive"
            );
            break;
        default:
            logError(
                ctx,
                token.pos,
                "Invalid or excessive tokens in <?teng include?>; "
                "ignoring the include directive"
            );
            break;
        }
        reset_error(ctx);
    }
}

void ignore_unknown_directive(Context_t *ctx, const Token_t &token) {
    logError(ctx, token.pos, "Unknown Teng directive: " + token.view() + "?>");
    reset_error(ctx);
}

void ignore_excessive_options(Context_t *ctx, const Pos_t &pos) {
    logWarning(
        ctx,
        pos,
        "This directive doesn't accept any options; ignoring them"
    );
}

void new_option(Context_t *ctx, const Token_t &name, Literal_t &&literal) {
    ctx->opts_sym.emplace(name.view(), std::move(literal.value));
}

uint32_t generate_func(Context_t *ctx, const Token_t &name, uint32_t nargs) {
    if (!ctx->params->isPrintEscapeEnabled()) {
        // be optimal for unescape($variable)
        // if last instr. is VAR and should be escaped
        // unescaping a single variable -- change escaping status of that var
        if ((nargs == 1) && (name.view() == "unescape")) {
            if (ctx->program->back().opcode() == OPCODE::VAR) {
                auto &instr = ctx->program->back().as<Var_t>();
                if (instr.escape) {
                    instr.escape = false;
                    return nargs;
                }
            }
        }
    }

    // also include diagnostic message if there any
    ctx->expr_diag.pop();
    ctx->expr_diag.unwind(ctx, ctx->unexpected_token);

    // regular function
    bool is_udf = name.token_id == LEX2::UDF_IDENT;
    generate<Func_t>(ctx, name.str(), nargs, name.pos, is_udf);
    return nargs;
}

} // namespace Parser
} // namespace Teng

