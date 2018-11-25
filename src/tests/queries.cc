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
 * Teng engine -- runtime queries.
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
    "The suspicious defined() operator",
    "[queries]"
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
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid variable identifier in defined()"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 10},
                    "Unexpected token: name=R_PAREN, view=)"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The defined operator is applied on variable with $") {
            Teng::Error_t err;
            auto t = "${defined($a)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "In query expression the identifier shouldn't be "
                    "denoted by $ sign"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The defined operator is applied on invalid expression") {
            Teng::Error_t err;
            auto t = "${defined(a.1)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid variable identifier in defined()"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The builtin _parent variable has crossed root "
                    "boundary; converting it to _this"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "The suspicious defined() operator - runtime variables",
    "[queries]"
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

        WHEN("The defined operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${defined($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The defined operator is applied on present empty variable") {
            Teng::Error_t err;
            auto t = "${defined($$a)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The defined operator is applied on present non empty variable") {
            Teng::Error_t err;
            auto t = "${defined($$b)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "(b)");
            }
        }

        WHEN("The defined operator is applied on empty fragment") {
            Teng::Error_t err;
            auto t = "${defined($$f)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The defined operator is applied on non empty fragment") {
            Teng::Error_t err;
            auto t = "${defined($$g)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The defined operator is applied on empty frag list fragment") {
            Teng::Error_t err;
            auto t = "${defined($$h)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The defined operator is applied on _this") {
            Teng::Error_t err;
            auto t = "${defined($$_this)}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The defined() query is deprecated; "
                    "use isempty() or exists() instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "111");
            }
        }

        // WHEN("The defined operator is applied on _parent") {
        //     Teng::Error_t err;
        //     auto t = "${defined($$_parent)}";
        //     auto result = g(err, t, root);
        //
        //     THEN("Result is false") {
        //         std::vector<Teng::Error_t::Entry_t> errs = {{
        //             Teng::Error_t::WARNING,
        //             {1, 2},
        //            "Runtime: The defined() operator is deprecated"
        //         }};
        //         ERRLOG_TEST(err.getEntries(), errs);
        //         REQUIRE(result == "0");
        //     }
        // }
    }
}

SCENARIO(
    "The exists() operator",
    "[queries]"
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
                    {1, 2},
                    "Invalid variable identifier in exists()"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Unexpected token: name=R_PAREN, view=)"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${exists(missing)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on present empty variable") {
            Teng::Error_t err;
            auto t = "${exists(a)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on present non empty variable") {
            Teng::Error_t err;
            auto t = "${exists(b)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty fragment") {
            Teng::Error_t err;
            auto t = "${exists(f)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on non empty fragment") {
            Teng::Error_t err;
            auto t = "${exists(g)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty frag list fragment") {
            Teng::Error_t err;
            auto t = "${exists(h)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on variable with $") {
            Teng::Error_t err;
            auto t = "${exists($a)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "In query expression the identifier shouldn't be "
                    "denoted by $ sign"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on invalid expression") {
            Teng::Error_t err;
            auto t = "${exists(a.1)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid variable identifier in exists()"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on _this") {
            Teng::Error_t err;
            auto t = "${exists(_this)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "The builtin _parent variable has crossed root "
                    "boundary; converting it to _this"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on _parent in nested frag") {
            Teng::Error_t err;
            auto t = "<?teng frag g?>${exists(_parent)}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "The exists() operator - runtime variables",
    "[queries]"
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

        WHEN("The exists operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${exists($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on present empty variable") {
            Teng::Error_t err;
            auto t = "${exists($$a)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on present non empty variable") {
            Teng::Error_t err;
            auto t = "${exists($$b)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty fragment") {
            Teng::Error_t err;
            auto t = "${exists($$f)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on non empty fragment") {
            Teng::Error_t err;
            auto t = "${exists($$g)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty frag list fragment") {
            Teng::Error_t err;
            auto t = "${exists($$h)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on _this") {
            Teng::Error_t err;
            auto t = "${exists($$_this)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        // WHEN("The exists operator is applied on _parent") {
        //     Teng::Error_t err;
        //     auto t = "${exists($$_parent)}";
        //     auto result = g(err, t, root);
        //
        //     THEN("Result is false") {
        //         std::vector<Teng::Error_t::Entry_t> errs;
        //         ERRLOG_TEST(err.getEntries(), errs);
        //         REQUIRE(result == "0");
        //     }
        // }
    }
}

SCENARIO(
    "The type query",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The type operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${type(missing)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The type operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${type(var_i)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "integral");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${type(var_r)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "real");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${type(var_s)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "string_ref");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${type(var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "string_ref");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${type(empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${type(frag)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${type(empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${type(frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }
    }
}

SCENARIO(
    "The type query - runtime variables",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The type operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${type($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The type operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${type($$var_i)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "integral");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${type($$var_r)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "real");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${type($$var_s)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "string_ref");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${type($$var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "string_ref");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${type($$empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${type($$frag)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on empty frag - indexed") {
            Teng::Error_t err;
            auto t = "${type($$empty_frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "frag_ref");
            }
        }

        WHEN("The exists operator is applied on frag - indexed") {
            Teng::Error_t err;
            auto t = "${type($$frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "frag_ref");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${type($$empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${type($$frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is type string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 9},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "list_ref");
            }
        }
    }
}

SCENARIO(
    "The count query",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The count operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${count(missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The count operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${count(var_i)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'integral' type with value '3' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${count(var_r)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'real' type with value '3.14' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${count(var_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value 'three' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${count(var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value '' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${count(empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is one") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${count(frag)}";
            auto result = g(err, t, root);

            THEN("Result is one") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${count(empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${count(frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is two") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }
    }
}

SCENARIO(
    "The count query - runtime variables",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The count operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${count($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The count operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${count($$var_i)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'integral' type with value '3' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${count($$var_r)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'real' type with value '3.14' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${count($$var_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value 'three' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${count($$var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value '' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${count($$empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is one") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${count($$frag)}";
            auto result = g(err, t, root);

            THEN("Result is one") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on empty frag - indexed") {
            Teng::Error_t err;
            auto t = "${count($$empty_frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on frag - indexed") {
            Teng::Error_t err;
            auto t = "${count($$frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which "
                    "count() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${count($$empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${count($$frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is two") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }
    }
}

SCENARIO(
    "The isempty query",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The isempty operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${isempty(missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The isempty operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${isempty(var_i)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'integral' type with value '3' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${isempty(var_r)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'real' type with value '3.14' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${isempty(var_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value 'three' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${isempty(var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value '' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${isempty(empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${isempty(frag)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${isempty(empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${isempty(frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "The isempty query - runtime variables",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The isempty operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "${isempty($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The isempty operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "${isempty($$var_i)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'integral' type with value '3' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "${isempty($$var_r)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'real' type with value '3.14' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "${isempty($$var_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value 'three' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "${isempty($$var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression references object of "
                    "'string_ref' type with value '' for which "
                    "isempty() query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "${isempty($$empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "${isempty($$frag)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on empty frag - indexed") {
            Teng::Error_t err;
            auto t = "${isempty($$empty_frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on frag - indexed") {
            Teng::Error_t err;
            auto t = "${isempty($$frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "${isempty($$empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is true") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "${isempty($$frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is false") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "The jsonify query",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The jsonify operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "%{jsonify(missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "Runtime: Variable '.missing' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "null");
            }
        }

        WHEN("The jsonify operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "%{jsonify(var_i)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "%{jsonify(var_r)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.14");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "%{jsonify(var_s)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"three\"");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "%{jsonify(var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"\"");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "%{jsonify(empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{}]");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "%{jsonify(frag)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{\"var\": \"VAR\"}]");
            }
        }

        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "%{jsonify(empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[]");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "%{jsonify(frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{}, {}]");
            }
        }
    }
}

SCENARIO(
    "The jsonify query - runtime variables",
    "[queries]"
) {
    GIVEN("Some variables and fragments") {
        Teng::Fragment_t root;
        root.addVariable("var_i", 3);
        root.addVariable("var_s", "three");
        root.addVariable("var_empty_s", "");
        root.addVariable("var_r", 3.14);
        root.addFragment("empty_frag");
        root.addFragment("frag").addVariable("var", "VAR");
        root.addFragmentList("empty_frag_list");
        auto &list = root.addFragmentList("frag_list");
        list.addFragment();
        list.addFragment();

        WHEN("The jsonify operator is applied on missing variable") {
            Teng::Error_t err;
            auto t = "%{jsonify($$missing)}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "Runtime: Variable '.missing' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "null");
            }
        }

        WHEN("The jsonify operator is applied on int variable") {
            Teng::Error_t err;
            auto t = "%{jsonify($$var_i)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The exists operator is applied on real variable") {
            Teng::Error_t err;
            auto t = "%{jsonify($$var_r)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.14");
            }
        }

        WHEN("The exists operator is applied on string variable") {
            Teng::Error_t err;
            auto t = "%{jsonify($$var_s)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"three\"");
            }
        }

        WHEN("The exists operator is applied on empty string variable") {
            Teng::Error_t err;
            auto t = "%{jsonify($$var_empty_s)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"\"");
            }
        }

        WHEN("The exists operator is applied on empty frag") {
            Teng::Error_t err;
            auto t = "%{jsonify($$empty_frag)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{}]");
            }
        }

        WHEN("The exists operator is applied on frag") {
            Teng::Error_t err;
            auto t = "%{jsonify($$frag)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{\"var\": \"VAR\"}]");
            }
        }

        WHEN("The exists operator is applied on empty frag - indexed") {
            Teng::Error_t err;
            auto t = "%{jsonify($$empty_frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }

        WHEN("The exists operator is applied on frag - indexed") {
            Teng::Error_t err;
            auto t = "%{jsonify($$frag[0])}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{\"var\": \"VAR\"}");
            }
        }


        WHEN("The exists operator is applied on empty frag list") {
            Teng::Error_t err;
            auto t = "%{jsonify($$empty_frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[]");
            }
        }

        WHEN("The exists operator is applied on frag list") {
            Teng::Error_t err;
            auto t = "%{jsonify($$frag_list)}";
            auto result = g(err, t, root);

            THEN("Result is json value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "[{}, {}]");
            }
        }
    }
}

SCENARIO(
    "The jsonify query - string escaping",
    "[queries]"
) {
    GIVEN("Some string variables with dagerous characters") {
        Teng::Fragment_t root;
        root.addVariable("var", "čřžý\"',[]{}\n");
        root.addVariable("bin", std::string("\x00\x01\x02", 3));

        WHEN("The jsonify operator on string with dangerous ascii characters") {
            Teng::Error_t err;
            auto t = "%{jsonify($var)}";
            auto result = g(err, t, root);

            THEN("Result is properly escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"čřžý\\\"',[]{}\\n\"");
            }
        }

        WHEN("The jsonify operator on string with binary characters") {
            Teng::Error_t err;
            auto t = "%{jsonify($bin)}";
            auto result = g(err, t, root);

            THEN("Result is properly escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "\"\\u0000\\u0001\\u0002\"");
            }
        }
    }
}

