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
 * Teng processor context.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Created.
 */

#ifndef TENGPROCESSORCONTEXT_H
#define TENGPROCESSORCONTEXT_H

#include <stack>

#include "logging.h"
#include "program.h"
#include "formatter.h"
#include "configuration.h"
#include "openframes.h"
#include "teng/error.h"
#include "teng/value.h"

namespace Teng {

// types
namespace exec {using Result_t = Value_t;}

/** Processor context variables that does not depend on runtime data and can be
 * used for evaluation during compile time.
 */
struct EvalCtx_t {
    Error_t &err;                           //!< error log
    const Program_t &program;               //!< program (translated template)
    const Dictionary_t &dict;               //!< language specific dictionary
    const Configuration_t &params;          //!< param dictionary
    const string_view_t &encoding;          //!< the template charset
    const OFFApi_t *frames_ptr = nullptr;   //!< open fragments frames accessor
    const Escaper_t *escaper_ptr = nullptr; //!< current string escaping machine
    const Instruction_t *instr = nullptr;   //!< current instruction or nullptr
    uint32_t log_suppressed = 0;            //!< enables errors log
};

/** Processor context variables that depends on runtime data and can't be
 * used for evaluation during compile time.
 */
struct RunCtx_t: public EvalCtx_t {
    /** C'tor.
     */
    RunCtx_t(
        Error_t &err,
        const Program_t &program,
        const Dictionary_t &dict,
        const Configuration_t &params,
        const string_view_t &encoding,
        const ContentType_t *contentType,
        const FragmentValue_t &root,
        Formatter_t &output
    ): EvalCtx_t{err, program, dict, params, encoding},
       output(output), frames(&root), escaper(contentType)
    {EvalCtx_t::frames_ptr = &frames; EvalCtx_t::escaper_ptr = &escaper;}

    /** D'tor.
     */
    ~RunCtx_t() {output.flush();}

    Formatter_t &output; //!< where write processor output
    OpenFrames_t frames; //!< list of frames of open fragments
    Escaper_t escaper;   //!< stack of escapers
};

/** It's supposed to use as default argument of function that needs RunCtx_t.
 */
struct RunCtxPtr_t {
    RunCtxPtr_t(RunCtx_t *ptr): ptr(ptr) {}
    RunCtxPtr_t(EvalCtx_t *) {throw runtime_ctx_needed_t{};}
    RunCtx_t *operator->() const {return ptr;}
    RunCtx_t &operator*() const {return *ptr;}
    operator EvalCtx_t *() const {return ptr;}
    RunCtx_t *ptr;
};

/** Because of undefined order of function arguments evaluation, you can't use
 * somehing like: some_function(pop(stack), pop(stack)).
 * (Supposing that pop returns a value.)
 *
 * This class solves the issue by postponing poping of args to function body.
 */
struct GetArg_t {
public:
    /** C'tor.
     */
    GetArg_t(std::vector<Value_t> &stack): stack(stack) {}

    /** Returns the most recent arg.
     */
    Value_t operator()() const {
        if (stack.empty())
            throw std::runtime_error("program stack underflow");
        Value_t value = std::move(stack.back());
        stack.pop_back();
        return value;
    }

protected:
    std::vector<Value_t> &stack; //!< where are arguments stored
};

/** Returns position of instruction in template source.
 */
inline Pos_t position(const Instruction_t *instr) {
    return instr? instr->pos(): Pos_t();
}

/** Writes fatal message to log.
 */
inline void logFatal(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logFatal(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

/** Writes error message to log.
 */
inline void logError(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logError(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

/** Writes warning message to log.
 */
inline void logWarning(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logWarning(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

} // namespace Teng

#endif /* TENGPROCESSORCONTEXT_H */

