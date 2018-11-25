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
 * $Id: tengprocessor.cc,v 1.15 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor executors.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Extracted from tengprocessor.cc.
 */

#ifndef TENGPROCESSORDEBUG_H
#define TENGPROCESSORDEBUG_H

#include <iomanip>
#include <iostream>

#include "util.h"
#include "function.h"
#include "instructionpointer.h"
#include "processorcontext.h"
#include "teng/udf.h"

namespace Teng {
namespace exec {
namespace {

/** Function that writes fragment value.
 */
void
write_frag_val(RunCtx_t *ctx, const FragmentValue_t &val, std::string indent);

/** Function that writes fragment variables.
 */
bool has_vars_and_frags(const Fragment_t &frag) {
    bool vars = false;
    bool frags = false;
    for (auto &var: frag) {
        switch (var.second.type()) {
        case FragmentValue_t::tag::frag:
        case FragmentValue_t::tag::frags:
        case FragmentValue_t::tag::frag_ptr:
            if (vars) return true;
            frags = true;
            break;
        case FragmentValue_t::tag::integral:
        case FragmentValue_t::tag::real:
        case FragmentValue_t::tag::string:
            if (frags) return true;
            vars = true;
            break;
        }
    }
    return false;
}

/** Function that writes fragment variables.
 */
void write_vars(RunCtx_t *ctx, const Fragment_t &frag, std::string indent) {
    // applies escaping on given string
    auto write_escaped = [&] (const string_view_t &what) {
        ctx->output.write(ctx->escaper.escape(what));
    };

    // shortcuts
    auto max_val_len = ctx->params.getMaxDebugValLength();

    // write vars
    for (auto &var: frag) {
        switch (var.second.type()) {
        case FragmentValue_t::tag::frag:
        case FragmentValue_t::tag::frags:
        case FragmentValue_t::tag::frag_ptr:
            // skip frags, they will be written after variables
            break;
        case FragmentValue_t::tag::integral:
            write_escaped(indent);
            write_escaped(var.first);
            write_escaped(": ");
            write_escaped(std::to_string(*var.second.integral()));
            write_escaped("\n");
            break;
        case FragmentValue_t::tag::real:
            write_escaped(indent);
            write_escaped(var.first);
            write_escaped(": ");
            write_escaped(std::to_string(*var.second.real()));
            write_escaped("\n");
            break;
        case FragmentValue_t::tag::string:
            write_escaped(indent);
            write_escaped(var.first);
            write_escaped(": '");
            write_escaped(clip(*var.second.string(), max_val_len));
            write_escaped("'\n");
            break;
        }
    }
}

/** Function that recursively writes nested fragments.
 */
void write_frags(RunCtx_t *ctx, const Fragment_t &frag, std::string indent) {
    // applies escaping on given string
    auto write_escaped = [&] (const string_view_t &what) {
        ctx->output.write(ctx->escaper.escape(what));
    };

    // write frags
    for (auto &var: frag) {
        switch (var.second.type()) {
        case FragmentValue_t::tag::frags:
            for (auto i = 0u; i < var.second.list()->size(); ++i) {
                write_escaped(indent);
                write_escaped(var.first);
                write_escaped("[" + std::to_string(i) + "]:\n");
                write_frag_val(ctx, (*var.second.list())[i], indent + "    ");
                if (i != (var.second.list()->size() - 1))
                    write_escaped("\n");
            }
            break;
        case FragmentValue_t::tag::frag:
        case FragmentValue_t::tag::frag_ptr:
            write_escaped(indent);
            write_escaped(var.first);
            write_escaped("[0]:\n");
            write_vars(ctx, *var.second.fragment(), indent + "    ");
            if (has_vars_and_frags(*var.second.fragment()))
                write_escaped("\n");
            write_frags(ctx, *var.second.fragment(), indent + "    ");
            write_escaped("\n");
            break;
        case FragmentValue_t::tag::integral:
        case FragmentValue_t::tag::real:
        case FragmentValue_t::tag::string:
            // skip scalar values, they have been written before variables
            break;
        }
    }
}

/** Function that writes fragment value.
 */
void
write_frag_val(RunCtx_t *ctx, const FragmentValue_t &val, std::string indent) {
    // applies escaping on given string
    auto write_escaped = [&] (const string_view_t &what) {
        ctx->output.write(ctx->escaper.escape(what));
    };

    // shortcuts
    auto max_val_len = ctx->params.getMaxDebugValLength();

    // write fragment value
    switch (val.type()) {
    case FragmentValue_t::tag::frags:
        for (auto i = 0u; i < val.list()->size(); ++i) {
            write_escaped(indent);
            write_escaped("[" + std::to_string(i) + "]:\n");
            write_frag_val(ctx, (*val.list())[i], indent + "    ");
            if (i != (val.list()->size() - 1))
                write_escaped("\n");
        }
        break;
    case FragmentValue_t::tag::frag:
    case FragmentValue_t::tag::frag_ptr:
        write_vars(&*ctx, *val.fragment(), indent);
        if (has_vars_and_frags(*val.fragment()))
            write_escaped("\n");
        write_frags(&*ctx, *val.fragment(), indent);
        break;
    case FragmentValue_t::tag::integral:
        write_escaped(std::to_string(*val.integral()));
        break;
    case FragmentValue_t::tag::string:
        write_escaped(clip(*val.string(), max_val_len));
        break;
    case FragmentValue_t::tag::real:
        write_escaped(std::to_string(*val.real()));
        break;
    };
}

} // namespace

/** Writes debug fragment into template if it is enabled.
 */
void debug_frag(RunCtxPtr_t ctx) {
    if (!ctx->params.isDebugEnabled())
        return;

    // applies escaping on given string
    auto write_escaped = [&] (const string_view_t &what) {
        ctx->output.write(ctx->escaper.escape(what));
    };

    // templates
    write_escaped("Template sources:\n");
    for (auto &source: ctx->program.getSources())
        write_escaped("    " + source->filename + "\n");

    // lang dicts
    write_escaped("\nLanguage dictionary sources:\n");
    for (auto &source: ctx->dict.getSources())
        write_escaped("    " + source->filename + "\n");

    // params files
    write_escaped("\nConfiguration dictionary sources:\n");
    for (auto &source: ctx->params.getSources())
        write_escaped("    " + source->filename + "\n");

    // params
    std::ostringstream os;
    os << ctx->params;
    write_escaped("\n" + os.str());

    // app data
    write_escaped("\nApplication data:\n");
    write_frag_val(&*ctx, *ctx->frames.root_frag(), "    ");
}

/** Writes bytecode fragment into template if it is enabled.
 */
void bytecode_frag(RunCtxPtr_t ctx) {
    if (!ctx->params.isBytecodeEnabled())
        return;

    std::ostringstream out;
    for (std::size_t i = 0; i < ctx->program.size(); ++i)
        out << std::setw(3) << std::setfill('0')
            << std::noshowpos << i << " " << ctx->program[i]
            << std::endl;
    ctx->output.write(ctx->escaper.escape(out.str()));
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSORDEBUG_H */

