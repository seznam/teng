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
 * Teng engine -- test of teng runtime variables.
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
    "Local runtime variables syntax",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        root.addVariable("var", "0");
        auto &first = root.addFragment("first");
        first.addVariable("var", "1");
        auto &second = first.addFragment("second");
        second.addVariable("var", "2");
        auto &third = second.addFragment("third");
        third.addVariable("var", 3);

        WHEN("Expanding var with one path segment") {
            Teng::Error_t err;
            auto t = "${$$var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 4},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with one path segment prefixed with _this") {
            Teng::Error_t err;
            auto t = "${$$_this.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with one path segment prefixed with two _this") {
            Teng::Error_t err;
            auto t = "${$$_this._this.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 16},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("One frag is opened") {

            WHEN("Expanding var with one path segment") {
                Teng::Error_t err;
                auto t = "<?teng frag first?>${$$var}<?teng endfrag?>";
                auto result = g(err, t, root);

                THEN("The result is variable value") {
                    std::vector<Teng::Error_t::Entry_t> errs = {{
                        Teng::Error_t::WARNING,
                        {1, 23},
                        "The runtime variable is useless; "
                        "converting it to regular variable"
                    }};
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "1");
                }
            }

            WHEN("Expanding var with one path segment prefixed with _this") {
                Teng::Error_t err;
                auto t = "<?teng frag first?>${$$_this.var}<?teng endfrag?>";
                auto result = g(err, t, root);

                THEN("The result is variable value") {
                    std::vector<Teng::Error_t::Entry_t> errs = {{
                        Teng::Error_t::WARNING,
                        {1, 29},
                        "The runtime variable is useless; "
                        "converting it to regular variable"
                    }};
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "1");
                }
            }

            WHEN("Expanding var with 1 path segment prefixed with two _this") {
                Teng::Error_t err;
                auto t = "<?teng frag first?>"
                         "${$$_this._this.var}"
                         "<?teng endfrag?>";
                auto result = g(err, t, root);

                THEN("The result is variable value") {
                    std::vector<Teng::Error_t::Entry_t> errs = {{
                        Teng::Error_t::WARNING,
                        {1, 35},
                        "The runtime variable is useless; "
                        "converting it to regular variable"
                    }};
                    ERRLOG_TEST(err.getEntries(), errs);
                    REQUIRE(result == "1");
                }
            }
        }
    }
}

SCENARIO(
    "Absolute runtime variables syntax",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        root.addVariable("var", "0");
        auto &first = root.addFragment("first");
        first.addVariable("var", "1");
        auto &second = first.addFragment("second");
        second.addVariable("var", "2");
        auto &third = second.addFragment("third");
        third.addVariable("var", 3);

        WHEN("Expanding var with one path segment") {
            Teng::Error_t err;
            auto t = "${$$.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with one path segment prefixed with _this") {
            Teng::Error_t err;
            auto t = "${$$._this._this.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with one path segment suffixed with _this") {
            Teng::Error_t err;
            auto t = "${$$.var._this._this}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with two path segments") {
            Teng::Error_t err;
            auto t = "${$$.first.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two path segments using indices and name") {
            Teng::Error_t err;
            auto t = "${$$._this['first'].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two path segments using indices only") {
            Teng::Error_t err;
            auto t = "${$$._this['first']['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with three path segments") {
            Teng::Error_t err;
            auto t = "${$$.first.second.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }

        WHEN("Expanding var with three path segments using indices and name") {
            Teng::Error_t err;
            auto t = "${$$.first['second'].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2");
            }
        }
    }
}

SCENARIO(
    "Relative runtime variables syntax",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        root.addVariable("var", "0");
        auto &first = root.addFragment("first");
        first.addVariable("var", "1");
        auto &second = first.addFragment("second");
        second.addVariable("var", "2");
        auto &third = second.addFragment("third");
        third.addVariable("var", 3);

        WHEN("Expanding var with two path segments") {
            Teng::Error_t err;
            auto t = "${$$first.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two path segments prefixed with _this") {
            Teng::Error_t err;
            auto t = "${$$_this._this.first.var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two path segments suffixed with _this") {
            Teng::Error_t err;
            auto t = "${$$first.var._this._this}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two path segments surrounded by _this") {
            Teng::Error_t err;
            auto t = "${$$_this.first.var._this}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with _this segment indexed") {
            Teng::Error_t err;
            auto t = "${$$_this['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with last _this segment indexed") {
            Teng::Error_t err;
            auto t = "${$$_this._this['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Expanding var with one segment with last segment indexed") {
            Teng::Error_t err;
            auto t = "${$$first['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Expanding var with two segments with last segment indexed") {
            Teng::Error_t err;
            auto t = "${$$_this.first['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Relative runtime variables using indices",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        first.addVariable("var", "VAR");
        auto &second = first.addFragment("second");
        auto &third = second.addFragment("third");
        third.addVariable("var", 3);

        WHEN("The fragment indexed by string") {
            Teng::Error_t err;
            auto t = "${$$first[0]['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }

        WHEN("The variable path use indexing only") {
            Teng::Error_t err;
            auto t = "${$$_this['first'][0]['second']['third']['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("Negative indices are used") {
            Teng::Error_t err;
            auto t = "${$$first[-1].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }

        WHEN("Expression in index") {
            Teng::Error_t err;
            auto t = "${$$first[$$.first.second.third.var - 3].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }
    }
}

SCENARIO(
    "Absolute runtime variables using indices",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        first.addVariable("var", "VAR");
        auto &second = first.addFragment("second");
        auto &third = second.addFragment("third");
        third.addVariable("var", 3);

        WHEN("The fragment indexed by string") {
            Teng::Error_t err;
            auto t = "${$$.first[0]['var']}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }

        WHEN("Negative indices are used") {
            Teng::Error_t err;
            auto t = "${$$.first[-1].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }

        WHEN("Expression in index") {
            Teng::Error_t err;
            auto t = "${$$.first[$$.first.second.third.var - 3].var}";
            auto result = g(err, t, root);

            THEN("The result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "VAR");
            }
        }
    }
}

SCENARIO(
    "Builtin variables as var name in auto runtime variables",
    "[vars][rtvars]"
) {
    GIVEN("Data with one fragment") {
        Teng::Fragment_t root;
        auto &nested = root.addFragment("nested");
        nested.addVariable("_first", "x");
        nested.addVariable("_inner", "x");
        nested.addVariable("_last", "x");
        nested.addVariable("_index", "x");

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._first}";
            auto result = g(err, t, root);

            THEN("It use builtin variable") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._inner}";
            auto result = g(err, t, root);

            THEN("It use builtin variable") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._last}";
            auto result = g(err, t, root);

            THEN("It use builtin variable") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._index}";
            auto result = g(err, t, root);

            THEN("It use builtin variable") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _this variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._this}";
            auto result = g(err, t, root);

            THEN("It expands to $list$") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 15},
                    "Runtime: Variable is a fragment list, not a scalar value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "$list$");
            }
        }

        WHEN("The _parent variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._parent}";
            auto result = g(err, t, root);

            THEN("It expands to $frag$") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 17},
                    "Runtime: Variable is a fragment, not a scalar value"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "$frag$");
            }
        }
    }

    GIVEN("Data with three fragments") {
        Teng::Fragment_t root;
        root.addFragment("nested");
        root.addFragment("nested");
        root.addFragment("nested");

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._count}";
            auto result = g(err, t, root);

            THEN("It expands to the number of nested fragments") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The _count variable is used for missing fragment") {
            Teng::Error_t err;
            auto t = "${missing._count}";
            auto result = g(err, t, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'missing' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Absolute runtime variables error reporting",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        auto &second = first.addFragment("second");
        second.addFragment("third");

        WHEN("The missing root variable is requested") {
            Teng::Error_t err;
            auto t = "${$$.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 5},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of first nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Runtime: The path expression '.first' references fragment "
                    "that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of second nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.second.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 18},
                    "Runtime: The path expression '.first.second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of third nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.second.third.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 24},
                    "Runtime: The path expression '.first.second.third' "
                    "references fragment that doesn't contain any value for "
                    "key 'var' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.missing.third.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Runtime: The path expression '.first' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Relative runtime variables error reporting",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        auto &second = first.addFragment("second");
        second.addFragment("third");

        WHEN("The missing root variable is requested") {
            Teng::Error_t err;
            auto t = "${$$var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 4},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 4},
                    "Runtime: Variable '.var' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Variable '.first.var' is undefined "
                    "[open_frags=.first, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing nested fragment runtime variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$second.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 30},
                    "Runtime: The path expression 'second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=.first, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing nested fragment variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${second.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression 'second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=.first, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${missing.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'missing' [open_frags=.first, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Runtime variables error reporting in fragments with more iteration",
    "[vars][rtvars]"
) {
    GIVEN("The data three nested fragments and first has more iterations") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        first.addVariable("one", "1");
        auto &second = first.addFragment("second");
        second.addVariable("test", "TEST");
        second.addFragment("third");
        root.addFragment("first");
        root.addFragment("first");

        WHEN("Variable of first fragment not exist in some iteration") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$one}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Variable '.first.one' is undefined "
                    "[open_frags=.first, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Variable '.first.one' is undefined "
                    "[open_frags=.first, iteration=2/3]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1undefinedundefined");
            }
        }

        WHEN("Variable in nested fragment not exist in some iteration") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${second.test}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'second' [open_frags=.first, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'second' [open_frags=.first, iteration=2/3]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "TESTundefinedundefined");
            }
        }

        WHEN("Dot syntax is used for list with more frags") {
            Teng::Error_t err;
            auto t = "${$$.first.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Runtime: The path expression '.first' references "
                    "fragment list of '3' fragments; "
                    "the expression is ambiguous [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Dot syntax is used for list with more frags") {
            Teng::Error_t err;
            auto t = "${$$.first.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Runtime: The path expression '.first' references "
                    "fragment list of '3' fragments; "
                    "the expression is ambiguous [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Absolute runtime variables error reporting using indexing",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0);
        auto &first = root.addFragment("first");
        auto &second = first.addFragment("second");
        second.addVariable("test", "TEST");
        second.addFragment("third");
        root.addFragment("first");
        root.addFragment("first").addVariable("last_first", "LAST");

        WHEN("The 'first' fragment identified by index that is out of range") {
            Teng::Error_t err;
            auto t = "${$$.first[10].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "Runtime: The index '10' is out of valid range <0, 3) "
                    "of the fragments list referenced by this path expression "
                    "'.first' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The 'first' fragment identified by negative index") {
            Teng::Error_t err;
            auto t = "${$$.first[-10].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "Runtime: The index '-10' is out of valid range <0, 3) "
                    "of the fragments list referenced by this path expression "
                    "'.first' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The 'first' fragment identified by string index") {
            Teng::Error_t err;
            auto t = "${$$.first['invalid'].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 10},
                    "Runtime: The path expression '.first' references "
                    "fragment list of '3' fragments; the expression is "
                    "ambiguous [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The fragment indexed by number") {
            Teng::Error_t err;
            auto t = "${$$.first[0][0]}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The path expression '.first[0]' references "
                    "fragment which can't be subscripted by values of "
                    "'integral' type with value '0' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The fragment indexed by string") {
            Teng::Error_t err;
            auto t = "${$$.first[0]['invalid']}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 13},
                    "Runtime: The path expression '.first[0]' references "
                    "fragment that doesn't contain any value for key 'invalid' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The variable is indexed by string") {
            Teng::Error_t err;
            auto t = "${$$._this['zero']['invalid']}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 18},
                    "Runtime: The path expression '._this['zero']' "
                    "references object of 'integral' type with value '0' that "
                    "is not subscriptable [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The first 'first' fragment identified by index") {
            Teng::Error_t err;
            auto t = "${$$.first[0].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 14},
                    "Runtime: The path expression '.first[0]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The last 'first' fragment identified by index") {
            Teng::Error_t err;
            auto t = "${$$.first[2].var + $$.first[2].last_first}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 14},
                    "Runtime: The path expression '.first[2]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefinedLAST");
            }
        }

        WHEN("The first 'first' fragment identified by complex scalar index") {
            Teng::Error_t err;
            auto t = "${$$.first[2 - 2].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 18},
                    "Runtime: The path expression '.first[2 - 2]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The first 'first' fragment identified by runtime index") {
            Teng::Error_t err;
            auto t = "${$$.first[$$.zero].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path expression '.first[$$.zero]' "
                    "references fragment that doesn't contain any value for "
                    "key 'var' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The builtin variable _parent optimalization",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        root.addVariable("var", "root-var");
        auto &first = root.addFragment("first");
        first.addVariable("var", "first-var");
        auto &second = first.addFragment("second");
        second.addVariable("var", "second-var");
        auto &third = second.addFragment("third");
        third.addVariable("var", "third-var");

        WHEN("The _parent is after 'push_this_frag'") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$_parent.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root-var");
            }
        }

        WHEN("The _parent is after 'push_attr' in absolute var") {
            Teng::Error_t err;
            auto t = "${$$.first._parent.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root-var");
            }
        }

        WHEN("The _parent is after 'push_attr' in relative var") {
            Teng::Error_t err;
            auto t = "${$$first.second.third._parent.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "second-var");
            }
        }

        WHEN("The multiple _parent vars are after 'push_this_frag'") {
            Teng::Error_t err;
            auto t = "<?teng frag first?><?teng frag second?>"
                     "${$$_parent._parent.var}"
                     "<?teng endfrag?><?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root-var");
            }
        }

        WHEN("The _parent is after 'push_attr_at'") {
            Teng::Error_t err;
            auto t = "${$$.first['second']._parent.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-var");
            }
        }

        WHEN("The _parent is after 'push_attr_at' but violates root boundary") {
            Teng::Error_t err;
            auto t = "${$$.first['second']._parent._parent._parent.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 37},
                    "The builtin _parent variable has crossed root "
                    "boundary; converting it to _this"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root-var");
            }
        }

        WHEN("The _parent is after 'push_attr_at' with nested rtvar") {
            Teng::Error_t err;
            auto t = "${$$.first[$$.first['second']]._parent.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "first-var");
            }
        }
    }
}

