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
 * Teng engine -- formatting tests.
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
    "Change formatting of text",
    "[format]"
) {
    GIVEN("Format block that change formatting to NOWHITE") {
        auto t = " { \t <?teng format space='nowhite'?>"
                 " a  b  c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces in formated block are discarted") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == " { \t abc \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to ONESPACE") {
        auto t = " { \t <?teng format space='onespace'?>"
                 " a  b  c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces in formated block are merged") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == " { \t  a b c  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to STRIPLINES") {
        auto t = " { \t <?teng format space='striplines'?>"
                 " a \n b \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("No leading and trailing whitespaces") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == " { \t  a\nb\nc  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to JOINLINES") {
        auto t = " { \t <?teng format space='joinlines'?>"
                 " a \n b \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces and newlines are merged") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == " { \t  a b c  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to NOWHITELINES") {
        auto t = " { \t <?teng format space='nowhitelines'?>"
                 " a \n\n b \n  \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Lines that contains whitespaces only are removed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == " { \t  a \n b \n c  \t } ");
            }
        }
    }
}

SCENARIO(
    "The misplaced endformat directive",
    "[format]"
) {
    GIVEN("Template with directive closing not opened format") {
        auto t = "{<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Unexpected token: [263] directive '<?teng endformat'"
                }, {
                    Teng::Error_t::FATAL,
                    {1, 1},
                    "Misplaced <?teng endformat?> directive (syntax error)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}");
            }
        }
    }
}

SCENARIO(
    "Bad options in format directive",
    "[format]"
) {
    GIVEN("Template with bad options in format directive") {
        auto t = "{<?teng format 1=1?>x<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: [331] integer literal '1'"
                }, {
                    Teng::Error_t::FATAL,
                    {1, 14},
                    "Invalid <?teng format ...?> directive (syntax error)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{x}");
            }
        }
    }
}

SCENARIO(
    "Unclosed format directive",
    "[format]"
) {
    GIVEN("Template with unclosed format directive") {
        auto t = "{<?teng format<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: [263] directive '<?teng endformat'"
                }, {
                    Teng::Error_t::FATAL,
                    {1, 14},
                    "Syntax error in teng format block; discarding it "
                    "(syntax error)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}");
            }
        }
    }
}

