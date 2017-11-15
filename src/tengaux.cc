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
 * $Id: tenginstruction.h,v 1.6 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * gethostname() holder.
 *
 * AUTHORS
 * Martin Fousek <martin.fousek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <cstdlib>
#include <unistd.h>

#include "tengaux.h"

namespace Teng {
namespace {

std::string resolve_tld() {
    static const std::string default_tld = "__default__";

    // take it from env
    if (char *teng_tld = getenv("TENG_TLD"))
        return teng_tld;

    // resolve local hostname
    char buf[1024] = {0};
    if (gethostname(buf, sizeof(buf))) return default_tld;

    // take it from host domain
    std::string result = buf;
    size_t pos = result.rfind(".");
    if (pos == std::string::npos) return default_tld;
    return result.substr(pos + 1);
}

} // namespace

const std::string &get_tld() {
    static const std::string value = resolve_tld();
    return value;
}

} // namespace Teng

