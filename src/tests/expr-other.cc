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

#include <teng.h>

#include "catch.hpp"
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "2");
            }
        }

        WHEN("Expression with MUL and MINUS operators evaluated") {
            Teng::Error_t err;
            auto t = "${2 * 3 - 2}";
            auto result = g(err, t, root);

            THEN("The precedence of operators is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "4");
            }
        }

        WHEN("Expression with MINUS and MUL operators evaluated") {
            Teng::Error_t err;
            auto t = "${2 - 3 * 2}";
            auto result = g(err, t, root);

            THEN("The precedence of operators is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
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
                    Teng::Error_t::ERROR,
                    {1, 7},
                    "Runtime: Left operand of - numeric operator is string"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefinedc");
            }
        }

        WHEN("Invalid numeric exrepssion in parentheses is evaluated") {
            Teng::Error_t err;
            auto t = "${(1 % 0) + 3}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 5},
                    "Runtime: Right operand of % division operator is zero"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Runtime: Left operand of + numeric operator is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The suspicious defined() operator",
    "[exprx]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("a", "");
        root.addVariable("b", "(b)");
        root.addVariable("_this", "111");
        root.addFragment("f");
        auto &frag = root.addFragment("g");
        frag.addVariable("a", "(a)");
        frag.addFragmentList("h");

        WHEN("The defined operator is applied on nothing") {
            Teng::Error_t err;
            auto t = "${defined()}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid variable identifier in defined()"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=R_PAREN, view=)"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The defined operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${defined(missing)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 19},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The defined operator is applied on present empty variable") {
            Teng::Error_t err;
            auto t = "${defined(a)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The defined operator is applied on present non empty variable") {
            Teng::Error_t err;
            auto t = "${defined(b)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "(b)");
            }
        }

        WHEN("The defined operator is applied on empty fragment") {
            Teng::Error_t err;
            auto t = "${defined(f)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The defined operator is applied on non empty fragment") {
            Teng::Error_t err;
            auto t = "${defined(g)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The defined operator is applied on empty frag list fragment") {
            Teng::Error_t err;
            auto t = "${defined(h)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The defined operator is applied on variable with $") {
            Teng::Error_t err;
            auto t = "${defined($a)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Variable identifier must not start with '$'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid identifier in defined() operator"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The defined operator is applied on invalid expression") {
            Teng::Error_t err;
            auto t = "${defined(a.1)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Invalid identifier in defined() operator"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The defined operator is applied on _this") {
            Teng::Error_t err;
            auto t = "${defined(_this)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The _this variable name is reserved"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 17},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "111");
            }
        }

        WHEN("The defined operator is applied on _parent") {
            Teng::Error_t err;
            auto t = "${defined(_parent)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The _parent variable name is reserved"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 19},
                    "Runtime: The defined() operator is deprecated"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "The suspicious exists() operator",
    "[expr][!mayfail]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("a", "");
        root.addVariable("b", "(b)");
        root.addVariable("_this", "111");
        root.addFragment("f");
        auto &frag = root.addFragment("g");
        frag.addVariable("a", "(a)");
        frag.addFragmentList("h");

        WHEN("The exists operator is applied on nothing") {
            Teng::Error_t err;
            auto t = "${exists()}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 8},
                    "Invalid identifier in exists() operator"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${exists(missing)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on present empty variable") {
            Teng::Error_t err;
            auto t = "${exists(a)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on present non empty variable") {
            Teng::Error_t err;
            auto t = "${exists(b)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty fragment") {
            Teng::Error_t err;
            auto t = "${exists(f)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on non empty fragment") {
            Teng::Error_t err;
            auto t = "${exists(g)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty frag list fragment") {
            Teng::Error_t err;
            auto t = "${exists(h)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on variable with $") {
            Teng::Error_t err;
            auto t = "${exists($a)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Variable identifier must not start with '$'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Invalid identifier in exists() operator"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on invalid expression") {
            Teng::Error_t err;
            auto t = "${exists(a.1)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Invalid identifier in exists() operator"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on _this") {
            Teng::Error_t err;
            auto t = "${exists(_this)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The _this variable name is reserved"
                }};
                // TODO(burlog): fakt tam ma smysl varovat? ^^^^
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on _parent") {
            Teng::Error_t err;
            auto t = "${exists(_parent)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The _parent variable name is reserved"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Invalid end of expressions",
    "[exprx]"
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "1hi");
            }
        }
    }
}

