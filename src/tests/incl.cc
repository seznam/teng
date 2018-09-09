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

#include <teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The include directive",
    "[include]"
) {
    GIVEN("Template including empty.txt") {
        auto t = "<?teng include file='empty.txt'?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Template including text.txt") {
        auto t = "<?teng include file='text.txt'?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains data from text.txt") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {TEST_ROOT "text.txt", 1, 48},
                    "Runtime: Variable '.var' is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "some text undefined\n");
            }
        }

        WHEN("Generated with variable defined") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            auto result = g(err, t, root);

            THEN("It contains data from text.txt") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "some text (var)\n");
            }
        }
    }

    GIVEN("Template including text.txt with ignored options") {
        auto t = "<?teng include file='text.txt' some='ignored'?>";

        WHEN("Generated with variable defined") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            auto result = g(err, t, root);

            THEN("It contains data from text.txt") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "some text (var)\n");
            }
        }
    }
}

SCENARIO(
    "The include directive in nested fragment",
    "[include]"
) {
    GIVEN("Template including empty.txt") {
        auto t = "<?teng frag sample?>{"
                 "<?teng include file='empty.txt'?>"
                 "}<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with one fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains fragment data") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}");
            }
        }
    }

    GIVEN("Template including text.txt") {
        auto t = "<?teng frag sample?>{"
                 "<?teng include file='text.txt'?>"
                 "}<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with one fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains data from text.txt and fragment") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {TEST_ROOT "text.txt", 1, 69},
                    "Runtime: Variable '.sample.var' is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{some text undefined\n}");
            }
        }

        WHEN("Generated with variable defined") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &sample = root.addFragment("sample");
            sample.addVariable("var", "(var)");
            auto result = g(err, t, root);

            THEN("It contains data from text.txt") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{some text (var)\n}");
            }
        }
    }
}

SCENARIO(
    "The include directive for missing file ",
    "[include]"
) {
    GIVEN("Template with bad include") {
        auto t = "<?teng include file='missing.txt'?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 35},
                    "System call failed: "
                    "Cannot open input file '" TEST_ROOT "missing.txt' "
                    "(No such file or directory)"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 35},
                    "System call failed: "
                    "Cannot stat file '" TEST_ROOT "missing.txt' "
                    "(No such file or directory)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The syntax error in include directive",
    "[include]"
) {
    GIVEN("Template with number as an option name") {
        auto t = "<?teng include 1='test.include.txt'?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: [331] integer literal '1'"
                }, {
                    Teng::Error_t::FATAL,
                    {1, 14},
                    "Invalid <?teng include ...?> directive (syntax error)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }
    }
}

