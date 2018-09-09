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
 * Teng engine -- Teng expressions tests.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The double divison",
    "[regex][expr]"
) {
    GIVEN("The empty data root") {
        Teng::Fragment_t root;

        WHEN("The expression contains two consecutive numeric divisions") {
            Teng::Error_t err;
            auto result = g(err, "${1 / 2 / 3}");

            THEN("It works") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Using regex to matching strings",
    "[regex][expr]"
) {
    GIVEN("The empty data root") {
        Teng::Fragment_t root;

        WHEN("The regex is used with equal operator") {
            Teng::Error_t err;
            auto result = g(err, "${'ahoj' =~ /.*/}");

            THEN("It expand to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The regex is used with not equal operator") {
            Teng::Error_t err;
            auto result = g(err, "${'ahoj' !~ /.*/}");

            THEN("It expand to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Using regex as pattern for regex_replace function",
    "[regex][expr]"
) {
    GIVEN("The empty data root") {
        Teng::Fragment_t root;

        WHEN("The regex literal is used as value for regex_replace") {
            Teng::Error_t err;
            auto result = g(err, "${regex_replace('foo bar', /\\w+/, '$(0)')}");

            THEN("It works") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "(foo) (bar)");
            }
        }

        WHEN("Regex contains escaped slash") {
            Teng::Error_t err;
            auto result = g(err, "/\\w\\/\\w/}");

            THEN("The slash is printed without backslash") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "/\\w/\\w/");
            }
        }

        WHEN("Regex contains common escape sequencies") {
            Teng::Error_t err;
            auto result = g(err, "/\\w\\n\\t\\w/}");

            THEN("They are expanded") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "/\n\t/");
            }
        }
    }
}

