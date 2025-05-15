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
 * Teng engine -- various expressions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <teng/teng.h>

#include "catch2/catch_test_macros.hpp"
#include "utils.h"

SCENARIO(
    "The positions in utf-8 encoded template",
    "[expr]"
) {
    GIVEN("Template with utf-8 encoded strings") {
        Teng::Fragment_t root;
        auto t = "žžž${fail+^+fail}ššš";

        WHEN("The template contains errors") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("The position respect utf-8 characters") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 5},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=BITXOR, view=^"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "žžžundefinedššš");
            }
        }
    }
}


SCENARIO(
    "The utf-8 encoded variable names",
    "[expr]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The invalid variable name encoded in utf-8") {
            Teng::Error_t err;
            auto t = "${žš}";
            auto result = g(err, t, root);

            THEN("The precedence of parentheses is respected") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Unexpected utf-8 encoded character 'ž'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Unexpected token: name=INV, view=ž"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 4},
                    "Unexpected utf-8 encoded character 'š'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 4},
                    "Unexpected token: name=INV, view=š"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The precedence of various numeric operators",
    "[expr]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Expression with parentheses evaluated") {
            Teng::Error_t err;
            auto t = "${2 * (3 - 2)}";
            auto result = g(err, t, root);

            THEN("The precedence of parentheses is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }

        WHEN("Expression with MUL and MINUS operators evaluated") {
            Teng::Error_t err;
            auto t = "${2 * 3 - 2}";
            auto result = g(err, t, root);

            THEN("The precedence of operators is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "4");
            }
        }

        WHEN("Expression with MINUS and MUL operators evaluated") {
            Teng::Error_t err;
            auto t = "${2 - 3 * 2}";
            auto result = g(err, t, root);

            THEN("The precedence of operators is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-4");
            }
        }
    }
}

SCENARIO(
    "The propagating of undefined value through parentheses",
    "[expr]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Invalid string exrepssion in parentheses is evaluated") {
            Teng::Error_t err;
            auto t = "${('a' - 'b') + 'c'}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 7},
                    "Runtime: Left operand of - numeric operator is string_ref"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefinedc");
            }
        }

        WHEN("Invalid numeric exrepssion in parentheses is evaluated") {
            Teng::Error_t err;
            auto t = "${(1 % 0) + 3}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 5},
                    "Runtime: Right operand of % division operator is zero"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "Runtime: Left operand of + numeric operator is undefined"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Invalid exrepssion in parentheses is evaluated") {
            Teng::Error_t err;
            auto t = "${(1 +) + 3}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 6},
                    "Unexpected token: name=R_PAREN, view=)"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Invalid end of expressions",
    "[expr]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Short expression contains unterminated comment only") {
            Teng::Error_t err;
            auto templ = "${/*1}hi";
            auto res = g(err, templ);

            THEN("The expression is replaced with undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Unexpected token: name=INV, view=/*1}"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefinedhi");
            }
        }

        WHEN("Short expression contains unterminated comment") {
            Teng::Error_t err;
            auto templ = "${1/*1}hi";
            auto res = g(err, templ);

            THEN("The expression is evaluated up to the comment only") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Invalid expression; the behaviour is undefined"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 3},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 3},
                    "Unexpected token: name=INV, view=/*1}"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1hi");
            }
        }

        WHEN("Expression contains unterminated comment only") {
            Teng::Error_t err;
            auto templ = "<?teng expr /*1?>hi";
            auto res = g(err, templ);

            THEN("The expression is replaced with undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unexpected token: name=INV, view=/*1?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefinedhi");
            }
        }

        WHEN("Expression contains unterminated comment") {
            Teng::Error_t err;
            auto templ = "<?teng expr 1/*1?>hi";
            auto res = g(err, templ);

            THEN("The expression is evaluated up to the comment only") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Invalid expression; the behaviour is undefined"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=INV, view=/*1?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1hi");
            }
        }
    }
}

SCENARIO(
    "The two consecutive binary operators",
    "[expr]"
) {
    GIVEN("Two binary operators expressions") {
        std::string t = "before"
                        "${a || 'A'}"
                        "between"
                        "${b || 'B'}"
                        "after";

        WHEN("Variables match 'false' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 0);
            root.addVariable("b", 0);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "beforeAbetweenBafter");
            }
        }

        WHEN("Variables match 'false'/'true' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 0);
            root.addVariable("b", 2);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "beforeAbetween2after");
            }
        }

        WHEN("Variables match 'true'/'false' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 1);
            root.addVariable("b", 0);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before1betweenBafter");
            }
        }

        WHEN("Variables match 'true' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 1);
            root.addVariable("b", 2);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before1between2after");
            }
        }
    }
}

