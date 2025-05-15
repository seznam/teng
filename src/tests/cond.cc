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
 * Teng engine -- conditional statements.
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
    "The simple conditional statement",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng endif?>";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is true branch") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "true-branch");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The conditional statement with else branch",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng else?>false-branch<?teng endif?>";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is true branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "true-branch");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is false branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }
    }
}

SCENARIO(
    "The conditional statement with elif branch",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>first-branch<?teng elif ";
        std::string t3 = "?>second-branch<?teng endif?>";

        WHEN("The both conditions contain true expressions") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "1" + t3, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The first condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "0" + t3, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The second condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "1" + t3, root);

            THEN("The result is second branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "second-branch");
            }
        }

        WHEN("The none conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "0" + t3, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The conditional statement with more elif branches",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>first-branch<?teng elif ";
        std::string t3 = "?>second-branch<?teng elif ";
        std::string t4 = "?>third-branch<?teng endif?>";

        WHEN("The all conditions contain true expressions") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "1" + t3 + "1" + t4, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The first two conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "1" + t3 + "0" + t4, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The second and third conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "1" + t3 + "1" + t4, root);

            THEN("The result is second branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "second-branch");
            }
        }

        WHEN("The first and third conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "0" + t3 + "1" + t4, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The first condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "0" + t3 + "0" + t4, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The second condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "1" + t3 + "0" + t4, root);

            THEN("The result is second branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "second-branch");
            }
        }

        WHEN("The third condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "0" + t3 + "1" + t4, root);

            THEN("The result is third branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }

        WHEN("The none conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "0" + t3 + "0" + t4, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The conditional statement with elif branch and else branch",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>first-branch<?teng elif ";
        std::string t3 = "?>second-branch<?teng else?>"
                         "third-branch<?teng endif?>";

        WHEN("The both conditions contain true expressions") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "1" + t3, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The first condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "0" + t3, root);

            THEN("The result is first branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-branch");
            }
        }

        WHEN("The second condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "1" + t3, root);

            THEN("The result is second branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "second-branch");
            }
        }

        WHEN("The none conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "0" + t3, root);

            THEN("The result is third branch value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }
    }
}

SCENARIO(
    "The nested if statements",
    "[cond]"
) {
    GIVEN("The nested conditional statements") {
        std::string t = "<?teng format space='nospace'?>"
                        "<?teng if o1?>"
                        "    <?teng if a1?>"
                        "        aaa-first-branch"
                        "    <?teng elif a2?>"
                        "        aaa-second-branch"
                        "    <?teng else?>"
                        "        aaa-third-branch"
                        "    <?teng endif?>"
                        "<?teng elif o2?>"
                        "    <?teng if b1?>"
                        "        bbb-first-branch"
                        "    <?teng elif b2?>"
                        "        bbb-second-branch"
                        "    <?teng else?>"
                        "        bbb-third-branch"
                        "    <?teng endif?>"
                        "<?teng else?>"
                        "    <?teng if c1?>"
                        "        ccc-first-branch"
                        "    <?teng elif c2?>"
                        "        ccc-second-branch"
                        "    <?teng else?>"
                        "        ccc-third-branch"
                        "    <?teng endif?>"
                        "<?teng endif?>"
                        "<?teng endformat?>";

        GIVEN("The o1 variable that is evaluable to true value") {
            Teng::Fragment_t root;
            root.addVariable("o1", 1);
            root.addVariable("o2", 0);

            WHEN("All nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("a1", 1);
                root.addVariable("a2", 1);
                auto result = g(err, t, root);

                THEN("The result is first branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-first-branch");
                }
            }

            WHEN("The second nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("a1", 0);
                root.addVariable("a2", 1);
                auto result = g(err, t, root);

                THEN("The result is second branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-second-branch");
                }
            }

            WHEN("The first nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("a1", 1);
                root.addVariable("a2", 0);
                auto result = g(err, t, root);

                THEN("The result is first branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-first-branch");
                }
            }

            WHEN("None of nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("a1", 0);
                root.addVariable("a2", 0);
                auto result = g(err, t, root);

                THEN("The result is else branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-third-branch");
                }
            }
        }

        GIVEN("The o2 variable that is evaluable to true value") {
            Teng::Fragment_t root;
            root.addVariable("o1", 0);
            root.addVariable("o2", 1);

            WHEN("All nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("b1", 1);
                root.addVariable("b2", 1);
                auto result = g(err, t, root);

                THEN("The result is first branch of second nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "bbb-first-branch");
                }
            }

            WHEN("The second nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("b1", 0);
                root.addVariable("b2", 1);
                auto result = g(err, t, root);

                THEN("The result is second branch of second nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "bbb-second-branch");
                }
            }

            WHEN("The first nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("b1", 1);
                root.addVariable("b2", 0);
                auto result = g(err, t, root);

                THEN("The result is first branch of second nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "bbb-first-branch");
                }
            }

            WHEN("None of nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("b1", 0);
                root.addVariable("b2", 0);
                auto result = g(err, t, root);

                THEN("The result is else branch of second nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "bbb-third-branch");
                }
            }
        }

        GIVEN("Both of o1 and o2 variables that are evaluable to false value") {
            Teng::Fragment_t root;
            root.addVariable("o1", 0);
            root.addVariable("o2", 0);

            WHEN("All nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("c1", 1);
                root.addVariable("c2", 1);
                auto result = g(err, t, root);

                THEN("The result is first branch of third nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "ccc-first-branch");
                }
            }

            WHEN("The second nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("c1", 0);
                root.addVariable("c2", 1);
                auto result = g(err, t, root);

                THEN("The result is second branch of third nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "ccc-second-branch");
                }
            }

            WHEN("The first nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("c1", 1);
                root.addVariable("c2", 0);
                auto result = g(err, t, root);

                THEN("The result is first branch of third nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "ccc-first-branch");
                }
            }

            WHEN("None of nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("c1", 0);
                root.addVariable("c2", 0);
                auto result = g(err, t, root);

                THEN("The result is else branch of third nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "ccc-third-branch");
                }
            }
        }

        GIVEN("The o1 and o2 variables that are evaluable to true value") {
            Teng::Fragment_t root;
            root.addVariable("o1", 1);
            root.addVariable("o2", 1);

            WHEN("All nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("a1", 1);
                root.addVariable("a2", 1);
                auto result = g(err, t, root);

                THEN("The result is first branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-first-branch");
                }
            }

            WHEN("The second nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("a1", 0);
                root.addVariable("a2", 1);
                auto result = g(err, t, root);

                THEN("The result is second branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-second-branch");
                }
            }

            WHEN("The first nested condition contains true value") {
                Teng::Error_t err;
                root.addVariable("a1", 1);
                root.addVariable("a2", 0);
                auto result = g(err, t, root);

                THEN("The result is first branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-first-branch");
                }
            }

            WHEN("None of nested conditions contain true value") {
                Teng::Error_t err;
                root.addVariable("a1", 0);
                root.addVariable("a2", 0);
                auto result = g(err, t, root);

                THEN("The result is else branch of first nested if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "aaa-third-branch");
                }
            }
        }
    }
}

SCENARIO(
    "The if statement with invalid expression",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng else?>"
                         "false-branch<?teng endif?>";

        WHEN("The if statement contains none expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + t2, root);

            THEN("The result is false branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "You forgot write condition of the if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "**^**" + t2, root);

            THEN("The result is false branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=REPEAT, view=**"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }

        WHEN("The if statement contains partially valid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1+1+*1" + t2, root);

            THEN("The result is false branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: name=MUL, view=*"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }

        WHEN("The if statement contains unterminated comment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1+1+/*1" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: name=INV, view=/*1?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }
    }
}


SCENARIO(
    "The elif statement with invalid expression",
    "[cond]"
) {
    GIVEN("Conditional statement") {
        std::string t1 = "<?teng if 0?>first-branch<?teng elif ";
        std::string t2 = "?>second-branch<?teng else?>"
                         "third-branch<?teng endif?>";

        WHEN("The if statement contains none expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + t2, root);

            THEN("The result is third branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 25},
                    "You forgot write condition of the elif statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "**^**" + t2, root);

            THEN("The result is third branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 25},
                    "Invalid expression in the elif statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Unexpected token: name=REPEAT, view=**"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }

        WHEN("The if statement contains partially valid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1+1+*1" + t2, root);

            THEN("The result is third branch") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 25},
                    "Invalid expression in the elif statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 41},
                    "Unexpected token: name=MUL, view=*"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }

        WHEN("The if statement contains unterminated comment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1+1+/*1" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 25},
                    "Invalid expression in the elif statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 41},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 41},
                    "Unexpected token: name=INV, view=/*1?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "third-branch");
            }
        }
    }
}

SCENARIO(
    "Unterminated if statement",
    "[cond]"
) {
    GIVEN("Unterminated true branch") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 24},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 24},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Unterminated false branch") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng else?>false-branch";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 49},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 49},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Unterminated elif branch") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng elif ";
        std::string t3 = "?>elif-branch";

        WHEN("The both conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "1" + t3, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 50},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The if condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2 + "0" + t3, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 50},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The elif condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "1" + t3, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 50},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The none conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2 + "0" + t3, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 50},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Unterminated false branch after elif") {
        std::string t1 = "<?teng if 0?>true-branch<?teng elif ";
        std::string t2 = "?>elif-branch<?teng else?>";

        WHEN("The elif condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 63},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The elif condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive of "
                    "<?teng if?> statement; discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 63},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Crossed frag and if blocks",
    "[cond]"
) {
    GIVEN("The endfrag in true branch") {
        std::string t1 = "<?teng frag a?><?teng if ";
        std::string t2 = "?>true-branch<?teng endfrag?>a<?teng endif?>b";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:39; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 39},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 56},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:39; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 39},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 56},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }
    }

    GIVEN("The endfrag false branch") {
        std::string t1 = "<?teng frag a?><?teng if ";
        std::string t2 = "?>true-branch<?teng else?>false-branch"
                         "<?teng endfrag?>a<?teng endif?>b";

        WHEN("The condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "1" + t2, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:64; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 64},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 81},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }

        WHEN("The condition contains false expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "0" + t2, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:64; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 64},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 81},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }
    }

    GIVEN("The endfrag elif branch") {
        std::string t1 = "<?teng frag a?><?teng if ";
        std::string t2 = "?>true-branch<?teng elif ";
        std::string t3 = "?>elif-branch<?teng endfrag?>a<?teng endif?>b";

        WHEN("The both conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "1" + t2 + "1" + t3, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:65; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 82},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }

        WHEN("The if condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "1" + t2 + "0" + t3, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:65; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 82},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }

        WHEN("The elif condition contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "0" + t2 + "1" + t3, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:65; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 82},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }

        WHEN("The none conditions contain true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("a");
            auto result = g(err, t1 + "0" + t2 + "0" + t3, root);

            THEN("The result is text after first error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "The <?teng if?> block crosses the parent fragment block "
                    "ending at=(no file):1:65; discarding whole if statement"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "The <?teng endfrag?> directive closes unopened fragment "
                    "block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 82},
                    "The <?teng endif?> directive closes unopened if block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "ab");
            }
        }
    }
}

SCENARIO(
    "Bad order of elif and else branches",
    "[cond]"
) {
    GIVEN("Template with else before elif branch") {
        std::string t = "<?teng if 1?>true-branch<?teng else?>false-branch"
                        "<?teng elif 1?>elif-branch<?teng endif?>";

        WHEN("The conditions contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Disordered elif/else branches in <?teng if?> statement; "
                    "discarding whole if statement"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Template with else in the middle of elif branches") {
        std::string t = "<?teng if 1?>true-branch<?teng elif 1?>elif-1-branch"
                        "<?teng else?>false-branch"
                        "<?teng elif 1?>elif-2-branch<?teng endif?>";

        WHEN("The conditions contains true expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Disordered elif/else branches in <?teng if?> statement; "
                    "discarding whole if statement"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Invalid expression in if statement",
    "[cond]"
) {
    GIVEN("Simple if statement") {
        std::string t1 = "<?teng if ";
        std::string t2 = "?>true-branch<?teng endif?>";

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1**^**2" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=BITXOR, view=^"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Unterminated comment in expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1/*2" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Unexpected token: name=INV, view=/*2?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "true-branch");
            }
        }
    }
}

SCENARIO(
    "Invalid expression in elif statement",
    "[cond]"
) {
    GIVEN("Simple if statement") {
        std::string t1 = "<?teng if 0?>true-branch<?teng elif ";
        std::string t2 = "?>elif-branch<?teng endif?>";

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1**^**2" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 24},
                    "Invalid expression in the elif statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 36},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 39},
                    "Unexpected token: name=BITXOR, view=^"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Unterminated comment in expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t1 + "1/*2" + t2, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 24},
                    "Invalid expression in the elif statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Unexpected token: name=INV, view=/*2?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "elif-branch");
            }
        }
    }
}

SCENARIO(
    "Invalid tokens in else end endif directives",
    "[cond]"
) {
    GIVEN("The bad tokens in else directive") {
        std::string t = "<?teng if 0?>true-branch"
                        "<?teng else 1 ?>false-branch<?teng endif?>";

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 24},
                    "Ignoring invalid excessive tokens in <?teng else?> "
                    "directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 36},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "false-branch");
            }
        }
    }

    GIVEN("The bad tokens in endif directive") {
        std::string t = "<?teng if 1?>true-branch<?teng endif 1?>";

        WHEN("The if statement contains invalid expression") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The result is empty") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 24},
                    "Ignoring invalid excessive tokens in <?teng endif?> "
                    "directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 37},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "true-branch");
            }
        }
    }
}

SCENARIO(
    "Two consecutive if blocks",
    "[cond]"
) {
    GIVEN("First if block with else and second without") {
        std::string t = "before"
                        "<?teng if $f?>"
                        "    1-true-branch"
                        "<?teng else?>"
                        "    1-false-branch"
                        "<?teng endif?>"
                        "between"
                        "<?teng if $s?>"
                        "    2-true-branch"
                        "<?teng endif?>"
                        "after";

        WHEN("The f is false and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of first if") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "    1-false-branch"
                                  "between"
                                  "after");
            }
        }

        WHEN("The f is false and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-false-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }

        WHEN("The f is true and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "    1-true-branch"
                                  "between"
                                  "after");
            }
        }

        WHEN("The f is true and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }
    }

    GIVEN("First if block with else and second with else too") {
        std::string t = "before"
                        "<?teng if $f?>"
                        "    1-true-branch"
                        "<?teng else?>"
                        "    1-false-branch"
                        "<?teng endif?>"
                        "between"
                        "<?teng if $s?>"
                        "    2-true-branch"
                        "<?teng else?>"
                        "    2-false-branch"
                        "<?teng endif?>"
                        "after";

        WHEN("The f is false and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of first if") {
                THEN("and content of false brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-false-branch"
                                      "between"
                                      "    2-false-branch"
                                      "after");
                }
            }
        }

        WHEN("The f is false and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-false-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }

        WHEN("The f is true and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of false brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-false-branch"
                                      "after");
                }
            }
        }

        WHEN("The f is true and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }
    }

    GIVEN("First if block without else and second with else") {
        std::string t = "before"
                        "<?teng if $f?>"
                        "    1-true-branch"
                        "<?teng endif?>"
                        "between"
                        "<?teng if $s?>"
                        "    2-true-branch"
                        "<?teng else?>"
                        "    2-false-branch"
                        "<?teng endif?>"
                        "after";

        WHEN("The f is false and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of second if") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "between"
                                  "    2-false-branch"
                                  "after");
            }
        }

        WHEN("The f is false and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of false brach of second if") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "between"
                                  "    2-true-branch"
                                  "after");
            }
        }

        WHEN("The f is true and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of false brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-false-branch"
                                      "after");
                }
            }
        }

        WHEN("The f is true and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }
    }

    GIVEN("Both blocks have else branch") {
        std::string t = "before"
                        "<?teng if $f?>"
                        "    1-true-branch"
                        "<?teng endif?>"
                        "between"
                        "<?teng if $s?>"
                        "    2-true-branch"
                        "<?teng endif?>"
                        "after";

        WHEN("The f is false and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result does not contain any if branches content") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "between"
                                  "after");
            }
        }

        WHEN("The f is false and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 0);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of second if") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "before"
                                  "between"
                                  "    2-true-branch"
                                  "after");
            }
        }

        WHEN("The f is true and s false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "after");
            }
        }

        WHEN("The f is true and s true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("f", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("result contains content of true brach of first if") {
                THEN("and content of true brach of second if") {
                    std::vector<Teng::Error_t::Entry_t> errs;
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
                }
            }
        }
    }

    GIVEN("First if block with elif and second without") {
        std::string t = "before"
                        "<?teng if $ff?>"
                        "    1-true-branch"
                        "<?teng elif fs?>"
                        "    1-elif-branch"
                        "<?teng endif?>"
                        "between"
                        "<?teng if $s?>"
                        "    2-true-branch"
                        "<?teng endif?>"
                        "after";

        WHEN("The all variables are false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("ff", 0);
            root.addVariable("fs", 0);
            root.addVariable("s", 0);
            auto result = g(err, t, root);

            THEN("Result does not contain data from if blocks") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "beforebetweenafter");
            }
        }

        WHEN("The all variables are true") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("ff", 1);
            root.addVariable("fs", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("Result contains data from true branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-true-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
            }
        }

        WHEN("The second variable of first if block is false") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("ff", 0);
            root.addVariable("fs", 1);
            root.addVariable("s", 1);
            auto result = g(err, t, root);

            THEN("Result contains data from elif and true branches") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "before"
                                      "    1-elif-branch"
                                      "between"
                                      "    2-true-branch"
                                      "after");
            }
        }
    }
}

SCENARIO(
    "Fuzzer problems in conditional statements",
    "[cond][fuzzer]"
) {
    GIVEN("Two consecutive invalid empty expressions in if's") {
        std::string t = "<?teng if ?>s<?teng elseif ?>g${abc}<?teng endif?>";

        WHEN("The second one is discarded") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("abc", 0);
            auto result = g(err, t, root);

            THEN("The ctx->expr_start_point.update_allowed is set to true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "You forgot write condition of the if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 13},
                    "You forgot write condition of the elif statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 27},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 27},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Two nested invalid if statements") {
        std::string t = "\t<?teng if?>\t<?teng frag?><?teng if\x94?>t";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Both if statements are discarded") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 1},
                    "You forgot write condition of the if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "The <?teng frag?> directive must contain the frag name "
                    "(e.g. <?teng frag example?>; "
                    "discarding fragment block content"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 24},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 26},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 26},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 35},
                    "Unexpected character '\x94'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 35},
                    "Unexpected token: name=INV, view=\x94"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 35},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 36},
                    "Unexpected token: name=END, view=?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\tt");
            }
        }
    }

    GIVEN("Assign after expr") {
        std::string t = "<?teng if e = ?>";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The parser accepts it") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unexpected token: name=ASSIGN, view=="
                }, {
                    Teng::Error_t::ERROR,
                    {1, 16},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Assign after expr + additional tokens") {
        std::string t = "<?teng if e = / / / ?>";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The parser accepts it") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unexpected token: name=ASSIGN, view=="
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Invalid frag between two teng_if's") {
        std::string t(
            "\"p<?teng if $\x05\x00\x00<?teng frag ?><?teng if\x00\x00"
            "e = ?>",
            47
        );

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Both if statements are discarded") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 2},
                    "Invalid expression in the if statement condition"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected character '\x05'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=INV, view=\x05"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 16},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 16},
                    "The <?teng frag?> directive must contain the frag name "
                    "(e.g. <?teng frag example?>; "
                    "discarding fragment block content"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 28},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 30},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 41},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 43},
                    "Unexpected token: name=ASSIGN, view=="
                }, {
                    Teng::Error_t::ERROR,
                    {1, 47},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"p");
            }
        }
    }

    GIVEN("Another invalid frag between two teng_if's") {
        std::string t = "<?teng if?><?teng frag	<?ifn if?>";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The parser accepts it") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::DIAG,
                    {1, 0},
                    "You forgot write condition of the if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "Missing <?teng endif?> closing directive "
                    "of <?teng if?> statement; "
                    "discarding whole if statement"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Unexpected token: name=END, view=?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Invalid expression, fix it please; "
                    "replacing whole expression with undefined value"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Invalid fragment identifier; "
                    "discarding fragment block content"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected character '\x7f'"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=INV, view=\x7f"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 34},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Disordered if statement in true branch of if statement") {
        std::string t = "<?if a?>"
                        "    <?if b?>"
                        "    <?else?>"
                        "    <?elif c?>"
                        "    <?endif?>"
                        "<?endif?>";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("a", 1);
            root.addVariable("b", 1);
            root.addVariable("c", 1);
            auto result = g(err, t, root);

            THEN("The parser accepts it") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Disordered elif/else branches in <?teng if?> statement; "
                    "discarding whole if statement"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "    ");
            }
        }
    }
}

