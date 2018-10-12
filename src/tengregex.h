
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
 * $Id: tengfunction.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Teng pcrepp::Pcre fixer.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-14  (burlog)
 *             Created.
 */

#ifndef TENGREGEX_H
#define TENGREGEX_H

#include <cstring>
#include <pcre++.h>

#include "tengvalue.h"

namespace Teng {

/** The class that does double initialization. The pcrepp::Pcre class does not
 * initialize global_t a case_t members to false if they are set to false. This
 * fix prior the calling pcrepp::Pcre c'tor sets memory to zero.
 */
struct FixedPCRE_t {
    /** C'tor: inits memory to zero and then call Pcre c'tor.
     */
    template <typename... Args_t>
    FixedPCRE_t(Args_t &&...args) {
        std::memset(&data, 0, sizeof(regex));
        new (&regex) pcrepp::Pcre(std::forward<Args_t>(args)...);
    }

    /** D'tor.
     */
    ~FixedPCRE_t() {regex.~Pcre();}

    // accessors
    pcrepp::Pcre *operator->() {return &regex;}
    const pcrepp::Pcre *operator->() const {return &regex;}

    // don't move
    FixedPCRE_t(FixedPCRE_t &&) = delete;
    FixedPCRE_t &operator=(FixedPCRE_t &&) = delete;

    // don't copy
    FixedPCRE_t(const FixedPCRE_t &) = delete;
    FixedPCRE_t &operator=(const FixedPCRE_t &) = delete;

    union {char data; pcrepp::Pcre regex;}; //!< the Pcre data
};

/** Converts set of bool to pcrepp flags.
 *
 * Implementation is in tenginstruction.cc.
 */
inline uint32_t to_pcre_flags(regex_flags_t flags) {
    uint32_t result = PCRE_UTF8;
    if (flags.ignore_case)
        result |= PCRE_CASELESS;
    if (flags.global)
        result |= PCRE_GLOBAL;
    if (flags.multiline)
        result |= PCRE_MULTILINE;
    if (flags.extended)
        result |= PCRE_EXTENDED;
    if (flags.extra)
        result |= PCRE_EXTRA;
    if (flags.ungreedy)
        result |= PCRE_UNGREEDY;
    if (flags.anchored)
        result |= PCRE_ANCHORED;
    if (flags.dollar_endonly)
        result |= PCRE_DOLLAR_ENDONLY;
    return result;
}

} // namespace Teng

#endif /* TENGREGEX_H */

