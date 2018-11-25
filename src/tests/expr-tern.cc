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
 * Teng engine -- ternar operator.
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
    "The ternary operator",
    "[expr][tern]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("three", 3);

        WHEN("Simple numeric ternary operator is evaluated with false") {
            Teng::Error_t err;
            auto t = "${0? 1: 2}";
            auto result = g(err, t, root);

            THEN("Result of: 0? 1: 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }

        WHEN("Simple numeric ternary operator is evaluated with true") {
            Teng::Error_t err;
            auto t = "${3? 1: 2}";
            auto result = g(err, t, root);

            THEN("Result of: 3? 1: 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Simple numeric ternary operator is evaluated with variable") {
            Teng::Error_t err;
            auto t = "${three? 1: 2}";
            auto result = g(err, t, root);

            THEN("Result of: three? 1: 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "The ternary operator syntax errors",
    "[expr][tern]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("three", 3);

        WHEN("Incomplete ternary operator: cond. expr and question mark only") {
            Teng::Error_t err;
            auto t = "${0?}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 4},
                    "Missing expression in true branch of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 4},
                    "Unexpected token: name=SHORT_END, view=}"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Incomplete ternary operator: colon token missing") {
            Teng::Error_t err;
            auto t = "${0?1}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 5},
                    "Missing colon token of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 5},
                    "Unexpected token: name=SHORT_END, view=}"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The empty true branch is missing") {
            Teng::Error_t err;
            auto t = "${0?:1}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 4},
                    "Missing expression in true branch of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 4},
                    "Unexpected token: name=COLON, view=:"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The expression in false branch is missing") {
            Teng::Error_t err;
            auto t = "${0?1:}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 6},
                    "Missing expression in false branch of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 6},
                    "Unexpected token: name=SHORT_END, view=}"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The invalid expression in true branch is missing") {
            Teng::Error_t err;
            auto t = "${0?1*^*1:1}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 6},
                    "Invalid expression in true branch of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 6},
                    "Unexpected token: name=BITXOR, view=^"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The invalid expression in false branch is missing") {
            Teng::Error_t err;
            auto t = "${0?1:1*^*1}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 8},
                    "Invalid expression in false branch of ternary operator"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 8},
                    "Unexpected token: name=BITXOR, view=^"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The ternary operator precedence",
    "[expr][tern]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("three", 3);

        WHEN("ternary operator condition contains plus operator") {
            Teng::Error_t err;
            auto t = "${1+0? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The plus operator wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "a");
            }
        }

        WHEN("ternary operator condition contains negation operator") {
            Teng::Error_t err;
            auto t = "${!0? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The negation operator wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "a");
            }
        }

        WHEN("ternary operator condition contains multiplication operator") {
            Teng::Error_t err;
            auto t = "${0*1? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The multiplication operator wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "b");
            }
        }

        WHEN("ternary operator condition contains comparison operator") {
            Teng::Error_t err;
            auto t = "${0==1? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The comparison operator wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "b");
            }
        }

        WHEN("ternary operator condition contains or operator") {
            Teng::Error_t err;
            auto t = "${1||0? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The or operator wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "a");
            }
        }

        WHEN("ternary operator condition contains and operator") {
            Teng::Error_t err;
            auto t = "${0&&1? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The comparison and wins precedence game") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "b");
            }
        }
    }
}

SCENARIO(
    "The nested ternary operators",
    "[expr][tern]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("false", 0);
        root.addVariable("true", 1);

        WHEN("False ternary operator before other ternary operator") {
            Teng::Error_t err;
            auto t = "${$false? 1: $false ? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The second operator is 'false branch'") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "b");
            }
        }

        WHEN("True ternary operator before other ternary operator") {
            Teng::Error_t err;
            auto t = "${$true? 'c': $false ? 'a': 'b'}";
            auto result = g(err, t, root);

            THEN("The second operator is 'false branch'") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "c");
            }
        }

        WHEN("Ternary operator in true branch of false ternary operator") {
            Teng::Error_t err;
            auto t = "${$false? $false ? 'a': 'b': 'c'}";
            auto result = g(err, t, root);

            THEN("The second operator is 'true branch'") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "c");
            }
        }

        WHEN("Ternary operator in true branch of true ternary operator") {
            Teng::Error_t err;
            auto t = "${$true? $false ? 'a': 'b': 'c'}";
            auto result = g(err, t, root);

            THEN("The second operator is 'true branch'") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "b");
            }
        }
    }
}

SCENARIO(
    "The two consecutive ternary operators",
    "[expr][tern]"
) {
    GIVEN("Two ternary operators expressions") {
        std::string t = "before"
                        "${a? 1: 2}"
                        "between"
                        "${b? 3: 4}"
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
                REQUIRE(result == "before2between4after");
            }
        }

        WHEN("Variables match 'false'/'true' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 0);
            root.addVariable("b", 1);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before2between3after");
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
                REQUIRE(result == "before1between4after");
            }
        }

        WHEN("Variables match 'true' branches") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 1);
            root.addVariable("b", 1);
            auto result = g(err, t, root);

            THEN("The result is combination of false branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before1between3after");
            }
        }
    }
}

