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

#include "syntax.hh"
#include "logging.h"
#include "program.h"
#include "formatter.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semanticblock.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {

void
open_format(Context_t *ctx, const Pos_t &pos, const Options_t &opts) {
    auto resolve_mode_id = [&] (auto iopt) {
        // ensure that space option exists
        if (iopt == opts.end()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is missing"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space is string
        if (!iopt->value.is_string_like()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is not string"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space has value
        if (iopt->value.string().empty()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is empty"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // check space option
        Formatter_t::Mode_t mode_id = resolveFormat(iopt->value.string());
        if (mode_id == Formatter_t::MODE_INVALID) {
            logError(
                ctx,
                pos,
                "Unsupported value '" + iopt->value.string() + "' "
                "of 'space' formatting option"
            );
            return Formatter_t::MODE_COPY_PREV;
        }
        return mode_id;
    };

    // generate format instruction
    auto iopt = opts.find("space");
    generate<OpenFormat_t>(ctx, resolve_mode_id(iopt), pos);
}

void open_inv_format(Context_t *ctx, const Pos_t &pos) {
    switch (ctx->unexpected_token) {
    case LEX2::END:
        logError(
            ctx,
            pos,
            "The <?teng format?> directive must contain at least one option in "
            "name=value format (e.g. space='nospace'); ignoring this directive"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Invalid excessive tokens in <?teng format?> directive found; "
            "ignoring this directive"
        );
        break;
    }
    reset_error(ctx);
    generate<OpenFormat_t>(ctx, Formatter_t::MODE_COPY_PREV, pos);
}

void close_format(Context_t *ctx, const Pos_t &pos) {
    generate<CloseFormat_t>(ctx, pos);
}

void close_inv_format(Context_t *ctx, const Pos_t &pos) {
    close_format(ctx, pos);
    logWarning(
        ctx,
        pos,
        "Ignoring invalid excessive tokens in <?teng endformat?> directive"
    );
    reset_error(ctx);
}

void
close_unclosed_format(Context_t *ctx, const Pos_t &pos, const Token_t &token) {
    close_format(ctx, token.pos);
    logWarning(
        ctx,
        pos,
        "The closing directive of this <?teng format?> directive is missing"
    );
    note_error(ctx, token);
    reset_error(ctx);
}

void open_ctype(Context_t *ctx, const Pos_t &pos, const Literal_t &type) {
    // push new ctype instruction
    if (auto *desc = ContentType_t::find(type.value.string())) {
        generate<OpenCType_t>(ctx, desc, pos);
        ctx->escaper.push(desc->contentType.get());
        return;
    }

    // push invalid ctype
    generate<OpenCType_t>(ctx, nullptr, pos);
    ctx->escaper.push(nullptr);

    // log invalid conent type name
    logError(
        ctx,
        pos,
        "Invalid content type '" + type.value.string() + "'"
        "; using top instead"
    );
}

void open_inv_ctype(Context_t *ctx, const Pos_t &pos) {
    switch (ctx->unexpected_token) {
    case LEX2::END:
        logError(
            ctx,
            pos,
            "The <?teng ctype?> directive must contain content type name "
            "(e.g. <?teng ctype 'text/html'?>; using top content type instead"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Invalid or excessive tokens in <?teng ctype?> directive"
            "; using top content type instead"
        );
        break;
    }
    generate<OpenCType_t>(ctx, nullptr, pos);
    ctx->escaper.push(nullptr);
    reset_error(ctx);
}

void close_ctype(Context_t *ctx, const Pos_t &pos) {
    ctx->escaper.pop();
    generate<CloseCType_t>(ctx, pos);
}

void close_inv_ctype(Context_t *ctx, const Pos_t &pos) {
    close_ctype(ctx, pos);
    logWarning(
        ctx,
        pos,
        "Ignoring invalid excessive tokens in <?teng endctype?> directive"
    );
    reset_error(ctx);
}

void
close_unclosed_ctype(Context_t *ctx, const Pos_t &pos, const Token_t &token) {
    close_ctype(ctx, token.pos);
    logWarning(
        ctx,
        pos,
        "The closing directive of this <?teng ctype?> directive is missing"
    );
    note_error(ctx, token);
    reset_error(ctx);
}

} // namespace Parser
} // namespace Teng

