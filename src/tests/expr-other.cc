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

        WHEN("Expression with MUL and SUB operators evaluated") {
            Teng::Error_t err;
            auto t = "${2 * 3 - 2}";
            auto result = g(err, t, root);

            THEN("The precedence of operators is respected") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "4");
            }
        }

        WHEN("Expression with SUB and MUL operators evaluated") {
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
            auto t = "${('a' + 'b') ++ 'c'}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 21},
                    "Runtime: left operand of + numeric operator is string"
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
                    {1, 14},
                    "Runtime: right operand of % division operator is zero"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Runtime: left operand of + numeric operator is undefined"
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
                    {1, 6},
                    "Unexpected token: [314] character ')'"
                }, {
                    Teng::Error_t::FATAL,
                    {1, 6},
                    "Invalid sub-expression (in parentheses) (syntax error)"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Runtime: left operand of + numeric operator is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The ternary operator",
    "[expr]"
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "2");
            }
        }

        WHEN("Simple numeric ternary operator is evaluated with true") {
            Teng::Error_t err;
            auto t = "${3? 1: 2}";
            auto result = g(err, t, root);

            THEN("Result of: 3 1: 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Simple numeric ternary operator is evaluated with variable") {
            Teng::Error_t err;
            auto t = "${three? 1: 2}";
            auto result = g(err, t, root);

            THEN("Result of: three 1: 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "The switch/case operator",
    "[expr]"
) {
    GIVEN("Some variables") {
        Teng::Fragment_t root;
        root.addVariable("aaa", "(aaa)");
        root.addVariable("bbb", "(bbb)");

        WHEN("The case with branch matching variable value is evaluated") {
            Teng::Error_t err;
            auto t = "${case($aaa, '(aaa)': 'aaa == (aaa)', *: '(xxx)')}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "aaa == (aaa)");
            }
        }

        WHEN("The case with branch no matching variable value is evaluated") {
           Teng::Error_t err;
           auto t = "${case($aaa, '(xxx)': 'aaa != (xxx)', *: '(aaa)')}";
           auto result = g(err, t, root);

           THEN("The right (default) branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "(aaa)");
           }
        }

        WHEN("The case with three branches is evaluated (first branch)") {
           Teng::Error_t err;
           root.addVariable("n", 1);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The first branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "a");
           }
        }

        WHEN("The case with three branches is evaluated (second branch)") {
           Teng::Error_t err;
           root.addVariable("n", 2);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The second branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "b");
           }
        }

        WHEN("The case with three branches is evaluated (third branch)") {
           Teng::Error_t err;
           root.addVariable("n", 3);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The third branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "c");
           }
        }

        WHEN("The case with no default branch is evaluated") {
           Teng::Error_t err;
           root.addVariable("n", 4);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c')}";
           auto result = g(err, t, root);

           THEN("Result is undefined") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "undefined");
           }
        }

        WHEN("The case complex expressions is evaluated") {
           Teng::Error_t err;
           root.addVariable("n", 3);
           root.addVariable("m", 4);
           auto t = "${case(n - m, -1: 'n' ++ 'm', *: '')}";
           auto result = g(err, t, root);

           THEN("The right branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "nm");
           }
        }
    }
}

SCENARIO(
    "The suspicious defined() operator",
    "[expr]"
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
                    {1, 9},
                    "Invalid identifier in defined() operator"
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
    "[expr]"
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
    "aaaaaaaaaa",
    "[expr][xxx]"
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
            auto t = "<?teng frag g1?>"
                     "<?teng frag g2?>"
                     "<?teng frag g3?>"
                     "<?teng frag g4?>"
                     "<?teng frag .g1?>"
                     "<?teng frag g5?>"
                     "<?teng frag .g1?>"
                     "<?teng frag g5?>"
                     "<?teng frag .g1?>"
                     "<?teng frag g5?>"
                     "${exists($$.g1.a)}"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

