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
 * $Id: tengerror.h,v 1.6 2011-01-19 06:39:45 burlog Exp $
 *
 * DESCRIPTION
 * Teng error logging.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-14  (burlog)
 *             Created.
 */

#include <string>
#include <vector>
#include <cerrno>
#include <cstring>

#include "tengprogram.h"
#include "tengparsercontext.h"
#include "tengyystype.h"
#include "tenglogging.h"

namespace Teng {

void
logError(
    Error_t &err,
    Error_t::Level_t level,
    const Pos_t &pos,
    const string_view_t &msg
) {
    Error_t::Entry_t new_entry = {
        level, {
            pos.filename == pos.no_filename()? "": *pos.filename,
            pos.lineno,
            pos.colno
        }, msg.str()
    };
    for (auto &entry: err.getEntries())
        if (entry == new_entry)
            return;
    err.append(std::move(new_entry));
}

void
logDebug(Error_t &err, const Pos_t &pos, const string_view_t &msg) {
    logError(err, Error_t::DEBUGING, pos, msg);
}

void
logWarning(Error_t &err, const Pos_t &pos, const string_view_t &msg) {
    logError(err, Error_t::WARNING, pos, msg);
}

void
logError(Error_t &err, const Pos_t &pos, const string_view_t &msg) {
    logError(err, Error_t::ERROR, pos, msg);
}

void
logFatal(Error_t &err, const Pos_t &pos, const string_view_t &msg) {
    logError(err, Error_t::FATAL, pos, msg);
}

void
logDebug(Error_t &err, const string_view_t &msg) {
    logError(err, Error_t::DEBUGING, msg);
}

void
logWarning(Error_t &err, const string_view_t &msg) {
    logError(err, Error_t::WARNING, msg);
}

void
logError(Error_t &err, const string_view_t &msg) {
    logError(err, Error_t::ERROR, msg);
}

void
logFatal(Error_t &err, const string_view_t &msg) {
    logError(err, Error_t::FATAL, msg);
}

namespace Parser {

void
logWarning(Context_t *ctx, const Pos_t &pos, const string_view_t &msg) {
    logError(ctx->program->getErrors(), Error_t::WARNING, pos, msg);
}

void
logDiag(Context_t *ctx, const Pos_t &pos, const string_view_t &msg) {
    logError(ctx->program->getErrors(), Error_t::DIAG, pos, msg);
}

void
logError(Context_t *ctx, const Pos_t &pos, const string_view_t &msg) {
    logError(ctx->program->getErrors(), Error_t::ERROR, pos, msg);
}

void
logFatal(Context_t *ctx, const Pos_t &pos, const string_view_t &msg) {
    logError(ctx->program->getErrors(), Error_t::FATAL, pos, msg);
}

} // namespace Parser
} // namespace Teng

