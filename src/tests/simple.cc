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
 * Teng engine -- basic tests.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <teng/teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The empty template",
    "[basic]"
) {
    GIVEN("Empty template") {
        auto t = "";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It is still empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Some text",
    "[basic]"
) {
    GIVEN("Template with some text") {
        auto t = "some text !@#$%^&*IO";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It equals to template") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == t);
            }
        }

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It equals to template") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == t);
            }
        }
    }
}

SCENARIO(
    "The syntax error",
    "[basic]"
) {
    GIVEN("Badly ordered frag directives") {
        auto t = "<?teng endfrag?><?teng frag sample?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 16},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The unknown teng directive",
    "[basic]"
) {
    GIVEN("Teng unknown directive") {
        auto t = "<?teng garf sample?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Unknown Teng directive: <?teng garf?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unexpected token: name=IDENT, view=sample"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Numeric config values",
    "[basic]"
) {
    GIVEN("Teng conf with maxdebugvallength") {
        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, "", root);

            THEN("no warning with bad numeric value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Teng lexer level 1 comments",
    "[basic]"
) {
    GIVEN("None data") {
        Teng::Fragment_t root;

        WHEN("Template with one comment") {
            Teng::Error_t err;
            auto result = g(err, "<!--- aaa --->", root);

            THEN("Comment is swallowed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Template with unclosed comment") {
            Teng::Error_t err;
            auto result = g(err, "<!--- aaa -->", root);

            THEN("Comment is swallowed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Incomplete utf-8 char in comment") {
            Teng::Error_t err;
            auto result = g(err, "<!---ùö", root);

            THEN("Comment is swallowed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

