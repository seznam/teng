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
 * $Id: tengfragmentstack.h,v 1.7 2006-06-13 10:04:16 vasek Exp $
 *
 * DESCRIPTION
 * Teng open fragments frames API.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Created.
 */

#ifndef TENGOPENFRAMESAPI_H
#define TENGOPENFRAMESAPI_H

#include <string>

#include "tengvalue.h"

namespace Teng {

/** This structure is control flow exception, that is used when some
 * instruction implementation encounters the state that can't be processed
 * with instance of EvalCtx_t and it needs the RunCtx_t to continue.
 */
struct runtime_ctx_needed_t {};

/** The API that implements common functionality accessing of fragments. It is
 * used to simulate fragmnets tree during parsing to allow expression
 * optimalizer optimalize exressions.
 */
struct OFFApi_t {
    /** D'tor.
     */
    virtual ~OFFApi_t() noexcept = default;

    /** Returns fragment/value at given open frame/fragments offsets.
     */
    virtual Value_t
    value_at(uint16_t frame_offset, uint16_t frag_offset) const = 0;

    /** Returns fragment value of desired name.
     */
    virtual Value_t
    value_at(const Value_t &, const string_view_t &, std::size_t &) const = 0;

    /** Returns fragment/list value of desire name/index.
     */
    virtual Value_t
    value_at(const Value_t &, const Value_t &, std::size_t &) const = 0;

    /** Returns names of open fragments in current frame joined by dots.
     */
    virtual std::string current_path() const = 0;

    /** Returns index of the current fragmnet in current opened fragment list.
     */
    virtual std::size_t current_list_i() const = 0;

    /** Returns size of the current fragmnet list.
     */
    virtual std::size_t current_list_size() const = 0;

    /** Returns true if value is not "undefined" value.
     */
    virtual Value_t exists(const Value_t &) const = 0;
};

} // namespace Teng

#endif /* TENGOPENFRAMESAPI_H */

