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
 * $Id: $
 *
 * DESCRIPTION
 * Teng engine -- test of url encoding teng functions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Coverted to catch2.
 */

#include <teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "Escaping string with urlencoding",
    "[function][urlencode]"
) {
    WHEN("String with special characters") {
        THEN("Some of them are escaped") {
            auto t = "${urlescape(\"'asdf!@#$%^&*\(\\\"\")}";
            REQUIRE(g(t) == "'asdf!@%23$%^&*(%22");
        }
    }
}

SCENARIO(
    "Unescaping urlencoding",
    "[function][urlencode]"
) {
    WHEN("Urlescaped string") {
        THEN("Urlescape sequencies are decoded") {
            auto t = "${urlunescape(\"%27asdf%21%40%23%24%25%5E%26%2A%28\")}";
            REQUIRE(g(t) == "'asdf!@#$%^&*(");
        }
    }
}

