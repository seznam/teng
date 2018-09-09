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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser diagnostic codes.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGPARSERDIAG_H
#define TENGPARSERDIAG_H

#include <stack>
#include <string>
#include <memory>
#include <vector>

#include "tengposition.h"

namespace Teng {
namespace Parser {

/** Possible diagnostic codes.
 */
enum diag_code {
    sentinel,
    case_cond,
    case_option,
    case_option_branch,
    case_default_branch,
    if_cond,
    if_branch,
    elif_cond,
    elif_branch,
    else_branch,
    fun_args,
};

// shortcuts
class Token_t;
struct Context_t;
using Addresses_t = std::vector<int32_t>;
using diag_code_type = diag_code;

/** Diagnostic codes entry that describes what happened.
 */
struct ExprDiagEntry_t {
    bool is_sentinel() {return code == diag_code::sentinel;}
    void log_case(Context_t *ctx, const Token_t &token);
    void log_if(Context_t *ctx, const Token_t &token);
    void log(Context_t *ctx, const Token_t &token);
    diag_code_type code; //!< the code value
    Pos_t pos;           //!< pos to corresponding token
};

/** Diagnostic codes are technique to help template writer better understand
 * where the syntax error is. Appropriate diagnostic code is pushed before
 * every diagnosed syntax symbol and popped out after it is properly parsed.
 * If some will remain in this list it means that some complex expression is
 * incorrectly formatted.
 */
struct ExprDiag_t {
    void unwind(Context_t *ctx, const Token_t &token);
    void pop() {entries.pop_back();}
    void push(ExprDiagEntry_t entry) {entries.push_back(std::move(entry));}
    void push_sentinel() {push({diag_code::sentinel, {}});}
    static void log_unexpected_token(Context_t *ctx);
    std::size_t size() const {return entries.size();}
    std::vector<ExprDiagEntry_t> entries; //!< list of diagnostic codes
};

} // namespace Parser
} // namespace Teng

#endif /* TENGPARSERDIAG_H */

