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

#include "tengerror.h"
#include "tenglogging.h"
#include "tengprogram.h"
#include "tengformatter.h"
#include "tengfragmentstack.h"
#include "tengcontenttype.h"
#include "tengparservalue.h"
#include "tengconfiguration.h"

namespace Teng {

// types
using Value_t = Parser::Value_t;
namespace exec {using Result_t = Value_t;}

/** 
 */
struct FragStackMimic_t {
    virtual const Value_t *findVariable(const Identifier_t &name) const = 0;
    virtual const Fragment_t *findFragment(const Identifier_t &name) const = 0;
};

/** Used for recognizing type of pointer stored on value stack for PUSH_ATTR,
 * PUSH_ATTR_AT, PUSH_ROOT_FRAG, PUSH_THIS_FRAG and REPR instructions.
 */
enum class FRAG_PTR {NULLPTR = 0, FRAGMENT, LIST, VALUE};

/** Processor context variables that does not depend on runtime data and can be
 * used for evaluation during compile time.
 */
struct EvalCtx_t {
    Error_t &err;                   //!< error log
    const Program_t &program;       //!< program (translated template)
    const Dictionary_t &dict;       //!< language specific dictionary
    const Configuration_t &cfg;     //!< param dictionary
    const std::string &encoding;    //!< the template charset
    uint32_t log_suppressed = 0;    //!< if errors should be written
    const Instruction_t *instr = nullptr; //!< current instruction or nullptr
    const FragStackMimic_t *frag_mimic = nullptr; //!< mimics frag stack
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
        const Configuration_t &cfg,
        const std::string &encoding,
        const ContentType_t *contentType,
        const Fragment_t &root,
        Formatter_t &output
    ): EvalCtx_t{err, program, dict, cfg, encoding},
       escaper(contentType), contentType(contentType), root(root),
       output(output), frag_stack(&root, err, cfg.isErrorFragmentEnabled())
    {}

    /** D'tor.
     */
    ~RunCtx_t();

    Escaper_t escaper;                 //!< stack of escapers
    const ContentType_t *contentType;  //!< the template content/mime type
    const Fragment_t &root;            //!< the fragment root
    Formatter_t &output;               //!< where write processor output
    FragmentStack_t frag_stack;        //!< create fragment stack
};

/** This structure is control flow exception, that is used when some
 * instruction implementation encounters the situation that can't be processed
 * with instance of EvalCtx_t and it needs the RunCtx_t to continue.
 */
struct runtime_ctx_needed_t {};

/** It's supposed to use as default argument of function that need RunCtx_t.
 */
struct RunCtxPtr_t {
    RunCtxPtr_t(RunCtx_t *ptr): ptr(ptr) {}
    RunCtxPtr_t(EvalCtx_t *) {throw runtime_ctx_needed_t{};}
    RunCtx_t *operator->() const {return ptr;}
    RunCtx_t &operator*() const {return *ptr;}
    RunCtx_t *ptr;
};

/** Returns the top item on stack.
 */
Value_t &stack_top(std::stack<Value_t> &stack) {
    if (stack.empty())
        throw std::runtime_error("value stack underflow");
    return stack.top();
}

/** Steals the top item from the stack.
 */
Value_t move_back(std::stack<Value_t> &stack) {
    if (stack.empty())
        throw std::runtime_error("value stack underflow");
    Value_t value = std::move(stack.top());
    stack.pop();
    return value;
}

/** Returns the top item on stack.
 */
Value_t &stack_top(std::vector<Value_t> &stack) {
    if (stack.empty())
        throw std::runtime_error("program stack underflow");
    return stack.back();
}

/** Steals the top item from the stack.
 */
Value_t move_back(std::vector<Value_t> &stack) {
    if (stack.empty())
        throw std::runtime_error("program stack underflow");
    Value_t value = std::move(stack.back());
    stack.pop_back();
    return value;
}

/** Returns position of instr in template source.
 */
Pos_t position(const Instruction_t *instr) {
    return instr? instr->pos: Pos_t{1, 0};
}

/** Writes fatal message to log.
 */
void logFatal(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logFatal(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

/** Writes error message to log.
 */
void logError(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logError(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

/** Writes warning message to log.
 */
void logWarning(EvalCtx_t &ctx, const std::string &msg) {
    if (ctx.log_suppressed) return;
    logWarning(ctx.err, position(ctx.instr), "Runtime: " + msg);
}

RunCtx_t::~RunCtx_t() {
    // TODO(burlog): dat to jinde? nebo si na to udelat spec stack?
    // if (!programStack.empty())
    //     logWarning(*this, "Program stack is not empty");
    // if (!stack.empty())
    //     logWarning(*this, "Value stack is not empty");
    // if (!fragmentstack.empty())
    //     logWarning(*this, "Fragment value stack is not empty");
    output.flush();
}

} // namespace Teng

#endif /* TENGPROCESSORCONTEXT_H */

