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

/** 
 */
struct OFFApi_t {
    /** D'tor.
     */
    virtual ~OFFApi_t() noexcept = default;

    virtual Value_t frag(uint16_t frame_offset, uint16_t frag_offset) const = 0;

    virtual Value_t frag_attr(const Value_t &, string_view_t) const = 0;

    virtual Value_t value_at(const Value_t &, const Value_t &) const = 0;

    virtual std::string current_path() const = 0;

    virtual std::size_t current_list_i() const = 0;

    virtual Value_t repr(const Value_t &) const = 0;

    virtual Value_t exists(const Value_t &) const = 0;
};

} // namespace Teng

#endif /* TENGOPENFRAMESAPI_H */

