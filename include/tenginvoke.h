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
 * $Id: tengfunction.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
 *
 * DESCRIPTION
 * Invoker for teng builtin and user defined funtions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#ifndef TENGINVOKE_H
#define TENGINVOKE_H

#include <vector>
#include <stdexcept>

#include <tengvalue.h>

namespace Teng {

/** This structure is control flow exception, that is used when some function
 * needs runtime context or can't be evaluated during template compile time
 * because of some other reasons (e.g. rand, time, ...).
 */
struct runtime_functx_needed_t {};

// forwards
struct Pos_t;
class Processor_t;
class FragmentStack_t;
class Dictionary_t;
class Configuration_t;
class ContentType_t;
class Escaper_t;
class Error_t;

/** This structure must be added as param to all user teng functions
 *  used by string function.
 */
struct FunctionCtx_t {
public:
    FunctionCtx_t(
        Error_t &err,
        const Pos_t &pos,
        const string_view_t &encoding,
        const Escaper_t *escaper,
        const Configuration_t &params,
        const Dictionary_t &dict
    ): err(err), pos(pos), encoding(encoding), params(params), dict(dict),
       escaper_ptr(escaper)
    {}

    /** Throws runtime_functx_needed_t if it is invoked during compile time.
     */
    const Escaper_t &escaper() const {
        if (escaper_ptr) return *escaper_ptr;
        throw runtime_functx_needed_t();
    }

    /** Throws runtime_functx_needed_t.
     */
    void runtime_ctx_needed() const {
        if (!escaper_ptr) throw runtime_functx_needed_t();
    }

    Error_t &err;                  //!< error log
    const Pos_t &pos;              //!< the position in source code
    const string_view_t &encoding; //!< encoding of template
    const Configuration_t &params; //!< current configuration
    const Dictionary_t &dict;      //!< current dictionary

//protected:
    const Escaper_t *escaper_ptr;  //!< string escaping machine
};

// shostcuts
using FunctionArgs_t = std::vector<Value_t>;
using FunctionResult_t = Value_t;

/** Invoker for teng builtin and user defined funtions.
 */
template <typename function_type>
struct Invoker_t {
    using Ctx_t = FunctionCtx_t;
    using Result_t = FunctionResult_t;
    using Args_t = FunctionArgs_t;

    /** Invoking of functions that take context as first argument.
     */
    template <typename call_t>
    auto invoke(const call_t &call, Ctx_t &ctx, const Args_t &args) const
    -> decltype(call(ctx, args)) {return call(ctx, args);}

    /** Invoking of functions that don't take context as first argument.
     */
    template <typename call_t>
    auto invoke(const call_t &call, Ctx_t &, const Args_t &args) const
    -> decltype(call(args)) {return call(args);}

    /** Invokes held function and translates all exceptions to errors.
     */
    template <typename PCtx_t>
    Result_t operator()(PCtx_t &pctx, Ctx_t &ctx, const Args_t &args) const {
        try {
            return invoke(function, ctx, args);
        } catch (const std::invalid_argument &e) {
            logError(*pctx, name +  "(): invalid arguments: " + e.what());
        } catch (const std::exception &e) {
            logError(*pctx, name + "(): function failed: " + e.what());
        } catch (...) {
            logError(*pctx, name + "(): function failed: unknown exception");
        }
        return Result_t();
    }

    /** Returns true if held function is valid.
     */
    explicit operator bool() const {return static_cast<bool>(function);}

    const std::string &name; //!< the function name
    function_type function;  //!< the function to invoke
};

} // namespace Teng

#endif /* TENGINVOKE_H */

