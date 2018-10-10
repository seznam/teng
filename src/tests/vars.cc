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
 * Teng engine -- test of teng variables.
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
    "With one variable in root fragment",
    "[vars][regvars]"
) {
    GIVEN("Template with one variable") {
        auto t = "${var}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Variable '.var' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Generated with defined variable") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            auto result = g(err, t, root);

            THEN("It contains data from fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "(var)");
            }
        }

        WHEN("Generated with parallel variable") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("other", "(other)");
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Variable '.var' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Generated with fragment of same name") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("var");
            auto result = g(err, t, root);

            THEN("It is $list$ string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 6},
                    "Runtime: Variable is a fragment list, not a scalar value"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "$list$");
            }
        }
    }
}

SCENARIO(
    "With one variable in nested fragment",
    "[vars][regvars]"
) {
    GIVEN("Template with one variable") {
        auto t = "<?teng frag sample?>${var}<?teng endfrag?>";

        WHEN("Generated with fragment without variable") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 22},
                    "Runtime: Variable '.sample.var' is undefined "
                    "[open_frags=.sample, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Generated with defined variable") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &sample = root.addFragment("sample");
            sample.addVariable("var", "(var)");
            auto result = g(err, t, root);

            THEN("It contains data from fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "(var)");
            }
        }

        WHEN("Generated with parallel variable") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &sample = root.addFragment("sample");
            sample.addVariable("other", "(other)");
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 22},
                    "Runtime: Variable '.sample.var' is undefined "
                    "[open_frags=.sample, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Generated with variable of same name in root fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("var", "(var)");
            auto &sample = root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 22},
                    "Runtime: Variable '.sample.var' is undefined "
                    "[open_frags=.sample, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Generated with fragment of same name") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &sample = root.addFragment("sample");
            sample.addFragment("var");
            auto result = g(err, t, root);

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 26},
                    "Runtime: Variable is a fragment list, not a scalar value"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "$list$");
            }
        }
    }
}

SCENARIO(
    "Builtin variables in root fragment",
    "[vars][regvars]"
) {
    GIVEN("Data with no fragments") {
        Teng::Fragment_t root;

        WHEN("The _first variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_first}";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_last}";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_inner}";
            auto result = g(err, t, root);

            THEN("It expands to false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _index variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_index}";
            auto result = g(err, t, root);

            THEN("It expands to 0 (first frag)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _count variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_count}";
            auto result = g(err, t, root);

            THEN("It expands to 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Builtin variables in nested fragments",
    "[vars][regvars]"
) {
    GIVEN("Data with no fragments") {
        Teng::Fragment_t root;

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_first}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Data with one fragment") {
        Teng::Fragment_t root;
        root.addFragment("nested");

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_first}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0 (first frag)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }
    }

    GIVEN("Data with two fragments") {
        Teng::Fragment_t root;
        root.addFragment("nested");
        root.addFragment("nested");

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_first}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to true, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "10");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "01");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "00");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0, 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "01");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 2, 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "22");
            }
        }
    }

    GIVEN("Data with three fragments") {
        Teng::Fragment_t root;
        root.addFragment("nested");
        root.addFragment("nested");
        root.addFragment("nested");

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_first}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to true, false, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "100");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, false, true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "001");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, true, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "010");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0, 1, 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "012");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 3, 3, 3") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "333");
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

            THEN("It use common variable") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Using the _first builtin variable as variable name of "
                    "runtime variable does not make sense; converting to "
                    "common variable lookup"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "x");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._inner}";
            auto result = g(err, t, root);

            THEN("It use common variable") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Using the _inner builtin variable as variable name of "
                    "runtime variable does not make sense; converting to "
                    "common variable lookup"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "x");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._last}";
            auto result = g(err, t, root);

            THEN("It use common variable") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Using the _last builtin variable as variable name of "
                    "runtime variable does not make sense; converting to "
                    "common variable lookup"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "x");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "${nested._index}";
            auto result = g(err, t, root);

            THEN("It use common variable") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Using the _index builtin variable as variable name of "
                    "runtime variable does not make sense; converting to "
                    "common variable lookup"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "x");
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "$frag$");
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which "
                    "count() query is undefined [open_frags=., iteration=0]"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'missing' [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Regular variable syntax",
    "[vars][regvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        root.addVariable("_underscore", "___");
        root.addVariable("_this", "bad_var_name_this");
        root.addVariable("_parent", "bad_var_name_parent");
        root.addVariable("_count", "bad_var_name_count");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");

        WHEN("Regular root variable is expanded") {
            Teng::Error_t err;
            auto result = g(err, "${var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Regular nested_1 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>${var}<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Regular nested_2 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>${var}<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("The variable starting with underscore") {
            Teng::Error_t err;
            auto result = g(err, "${_underscore}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The variable names starting with an underscore are "
                    "reserved, and might cause undefined behaviour in future: "
                    "var=_underscore"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "___");
            }
        }

        WHEN("The overriden _this only") {
            Teng::Error_t err;
            auto result = g(err, "${_this}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Identifier '_this' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_this");
            }
        }

        WHEN("The plain _this only") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>${_this}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Result is $frag$") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 30},
                    "Runtime: Variable is a fragment, not a scalar value"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "$frag$");
            }
        }

        WHEN("The overriden _parent only") {
            Teng::Error_t err;
            auto result = g(err, "${_parent}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The builtin _parent variable has crossed root boundary; "
                    "converting it to _this"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Identifier '_parent' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_parent");
            }
        }

        WHEN("The plain _parent only") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>${_parent}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Result is $list$") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 32},
                    "Runtime: Variable is a fragment, not a scalar value"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "$frag$");
            }
        }

        WHEN("The _count only") {
            Teng::Error_t err;
            auto result = g(err, "${_count}", root);

            THEN("Result is the number of root fragments -> 1") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _first only") {
            Teng::Error_t err;
            auto result = g(err, "${_first}", root);

            THEN("Result is the true") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last only") {
            Teng::Error_t err;
            auto result = g(err, "${_last}", root);

            THEN("Result is the true") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner only") {
            Teng::Error_t err;
            auto result = g(err, "${_inner}", root);

            THEN("Result is the false") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Absolute variable syntax",
    "[vars][absvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");
        nested_2.addVariable("_underscore", "___");
        auto &other = root.addFragment("other");
        other.addVariable("var", "other_var");

        WHEN("Absolute root variable is expanded") {
            Teng::Error_t err;
            auto result = g(err, "${.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Absolute nested_1 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "${.nested_1.var}"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Absolute nested_2 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("Absolute invalid variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.invalid.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'invalid' [open_frags=.nested_1.nested_2, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Absolute variable from previous frame is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag .other?>"
                         "${.nested_1.var}"
                         "${.other.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is composed from variables from two frames") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_varother_var");
            }
        }

        WHEN("Absolute nested_1.invalid variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1.invalid.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Runtime: The path expression '.nested_1' references "
                    "fragment that doesn't contain any value for key 'invalid' "
                    "[open_frags=.nested_1.nested_2, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Absolute missing variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1.nested_2.missing}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: Variable '.nested_1.nested_2.missing' is "
                    "undefined [open_frags=.nested_1.nested_2, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Reserved name of variable") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1.nested_2._underscore}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "The variable names starting with an underscore are "
                    "reserved, and might cause undefined behaviour in future: "
                    "var=.nested_1.nested_2._underscore"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "___");
            }
        }
    }
}

SCENARIO(
    "Absolute variable syntax - builtin variables",
    "[vars][absvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        root.addVariable("_this", "bad_var_name_this");
        root.addVariable("_parent", "bad_var_name_parent");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");

        WHEN("The ._this only") {
            Teng::Error_t err;
            auto result = g(err, "${._this}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Identifier '_this' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_this");
            }
        }

        WHEN("The ._parent only") {
            Teng::Error_t err;
            auto result = g(err, "${._parent}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The builtin _parent variable has crossed root boundary; "
                    "converting it to _this"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Identifier '_parent' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_parent");
            }
        }

        WHEN("Accessing root variable via excessive _this") {
            Teng::Error_t err;
            auto result = g(err, "${._this.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Ignoring useless '_this' variable path segment"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Accessing root variable via _this") {
            Teng::Error_t err;
            auto result = g(err, "${_this.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Accessing root variable via root boundary violating _parent") {
            Teng::Error_t err;
            auto result = g(err, "${._parent.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "The _parent violates the root boundary"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Accessing nested_1 variable via excessive _this") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1._this.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 56},
                    "Ignoring useless '_this' variable path segment"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Accessing root variable via _parent") {
            Teng::Error_t err;
            auto result = g(err, "${.nested_1._parent.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Accessing nested_1 variable via _parent") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${_parent.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Accessing root variable via _parent sequence") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${_parent._parent.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Root boundary violating _parent sequence") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1._parent._parent.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 64},
                    "The _parent violates the root boundary"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Root boundary violating _parent sequence - nested_2.var") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${.nested_1._parent._parent.nested_1.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 64},
                    "The _parent violates the root boundary"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("Root boundary violating _parent - nested_2.var") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${._parent.nested_1.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 47},
                    "The _parent violates the root boundary"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }
    }
}

SCENARIO(
    "Relative variable syntax",
    "[vars][relvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");
        auto &nested_3 = nested_2.addFragment("nested_3");
        nested_3.addVariable("var", "3");
        auto &nested_4 = nested_3.addFragment("nested_4");
        nested_4.addVariable("var", "4");

        WHEN("Relative nested_1 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "${nested_1.var}"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Relative nested_2 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("Relative nested_1.nested_2 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_1.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("Invalid nested variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${invalid.nested_2.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'invalid' [open_frags=.nested_1.nested_2, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Invalid nested_1 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${invalid.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'invalid' [open_frags=.nested_1.nested_2, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Relative variables are expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "<?teng frag nested_3?>"
                         "<?teng frag nested_4?>"
                         "${nested_2.nested_3.var}"
                         "${nested_3.var}"
                         "${nested_1.var}"
                         "${nested_1.nested_2.var}"
                         "${nested_2.nested_3.nested_4.var}"
                         "${nested_4.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "33nested_1_varnested_2_var44");
            }
        }

    }
}

SCENARIO(
    "Relative variable syntax - builtin variables",
    "[vars][relvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        nested_1.addVariable("_this", "bad_var_name_this_1");
        nested_1.addVariable("_parent", "bad_var_name_parent_1");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");
        nested_2.addVariable("_this", "bad_var_name_this_2");
        nested_2.addVariable("_parent", "bad_var_name_parent_2");
        auto &nested_3 = nested_2.addFragment("nested_3");
        nested_3.addVariable("var", "3");
        auto &nested_4 = nested_3.addFragment("nested_4");
        nested_4.addVariable("var", "4");

        WHEN("Ignoring _this builtin variable in variable path") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_1._this.nested_2._this.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 55},
                    "Ignoring useless '_this' variable path segment"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 70},
                    "Ignoring useless '_this' variable path segment"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("The _parent inside relative path") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_2._parent.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("The _this relative variable") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_2._this}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: Identifier '_this' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_this_2");
            }
        }

        WHEN("The _parent relative variable") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_2._parent}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: Identifier '_parent' is reserved, "
                    "please don't use it"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_parent_2");
            }
        }
    }
}

SCENARIO(
    "Variables setting",
    "[vars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        auto &nested_1 = root.addFragment("nested_1");
        nested_1.addVariable("var", "nested_1_var");
        auto &nested_2 = nested_1.addFragment("nested_2");
        nested_2.addVariable("var", "nested_2_var");
        auto &nested_3 = nested_2.addFragment("nested_3");
        nested_3.addVariable("var", "3");
        auto &nested_4 = nested_3.addFragment("nested_4");
        nested_4.addVariable("var", "4");

        WHEN("Setting root variable from root") {
            Teng::Error_t err;
            auto t = "<?teng set .var_set = 'root_var_set'?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var_set");
            }
        }

        WHEN("Setting root variable from nested frag") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>"
                     "<?teng set .var_set = 'root_var_set'?>${.var_set}"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var_set");
            }
        }

        WHEN("Setting nested_1 variable from nested frag") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>"
                     "<?teng frag nested_2?>"
                     "<?teng frag .nested_1?>"
                     "<?teng set .nested_1.var_set = 'xxx'?>"
                     "${var_set},"
                     "${nested_1.var_set},"
                     "${.nested_1.var_set},"
                     "<?teng set nested_1.var_set = 'yyy'?>"
                     "${var_set},"
                     "${nested_1.var_set},"
                     "${.nested_1.var_set}"
                     "<?teng endfrag?>|"
                     "${nested_1.var_set},"
                     "${.nested_1.var_set}"
                     "<?teng endfrag?>"
                     "${var_set},"
                     "${nested_1.var_set},"
                     "${.nested_1.var_set}"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Variable is defined only in inner frame") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 264},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 284},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 320},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 331},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 351},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "xxx,xxx,xxx,yyy,yyy,yyy"
                        "|undefined,undefinedundefined,undefined,undefined");
            }
        }

        WHEN("Setting nested_2 variable from nested frag from next frame") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>"
                     "<?teng frag nested_2?>"
                     "<?teng frag .nested_1?>"
                     "<?teng set nested_2.var_set = 'xxx'?>"
                     "${var_set},"
                     "${nested_2.var_set},"
                     "${.nested_1.nested_2.var_set},"
                     "<?teng set nested_2.var_set = 'yyy'?>"
                     "${var_set},"
                     "${nested_2.var_set},"
                     "${.nested_1.nested_2.var_set}"
                     "<?teng endfrag?>|"
                     "${var_set},"
                     "${nested_2.var_set},"
                     "${.nested_1.nested_2.var_set},"
                     "<?teng endfrag?>"
                     "${var_set},"
                     "${nested_2.var_set},"
                     "${.nested_1.nested_2.var_set}"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Variable is defined in outer frame") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 106},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 204},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 358},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0]"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 369},
                    "Runtime: The path expression 'nested_2' references "
                    "fragment that doesn't contain any value for key 'var_set' "
                    "[open_frags=.nested_1, iteration=0]"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 389},
                    "Runtime: The path expression '.nested_1.nested_2' "
                    "references fragment that doesn't contain any value for "
                    "key 'var_set' [open_frags=.nested_1, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined,xxx,xxx,undefined,yyy,yyy"
                        "|yyy,yyy,yyy,undefined,undefined,undefined");
            }
        }

        WHEN("Frag with set variable is closed and reopend") {
            Teng::Error_t err;
            auto t = "<?teng frag nested_1?>"
                     "<?teng set var_set = 'nested_1_var_set'?>${var_set}"
                     "<?teng endfrag?>"
                     "<?teng frag nested_1?>"
                     "${var_set}"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("Variable become undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 113},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_1_var_setundefined");
            }
        }
    }
}

SCENARIO(
    "Variables setting errors",
    "[vars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");

        WHEN("Setting set by application") {
            Teng::Error_t err;
            auto t = "<?teng set var = 'root_var_set'?>${.var}";
            auto result = g(err, t, root);

            THEN("Result is origin value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Runtime: Cannot rewrite variable '.var' which is "
                    "already set by the application; nothing set "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Variable of empty name is set") {
            Teng::Error_t err;
            auto t = "<?teng set = 'root_var_set'?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "Invalid variable name in the <?teng set?> directive; "
                    "ignoring the set directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Unexpected token: name=ASSIGN, view=="
                }, {
                    Teng::Error_t::WARNING,
                    {1, 31},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The selector is set") {
            Teng::Error_t err;
            auto t = "<?teng set . = 'root_var_set'?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "Invalid variable name in the <?teng set?> directive; "
                    "ignoring the set directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=ASSIGN, view=="
                }, {
                    Teng::Error_t::WARNING,
                    {1, 33},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Setting the fragment value") {
            Teng::Error_t err;
            auto t = "<?teng set ._this = 'root_var_set'?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Builtin variable '._this' can't be set"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 38},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Invalid expression") {
            Teng::Error_t err;
            auto t = "<?teng set .var_set = *var?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=MUL, view=*"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 30},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Invalid expression - more invalid tokens") {
            Teng::Error_t err;
            auto t = "<?teng set .var_set = **^**?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=REPEAT, view=**"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 31},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Invalid expression with valid prefix") {
            Teng::Error_t err;
            auto t = "<?teng set .var_set = var+var+*var?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 30},
                    "Unexpected token: name=MUL, view=*"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 38},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Unterminated teng set directive") {
            Teng::Error_t err;
            auto t = "<?teng set .var_set = /*?>${.var_set}";
            auto result = g(err, t, root);

            THEN("Result is undefined value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=INV, view=/*?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 28},
                    "Runtime: Variable '.var_set' is undefined "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Absolute runtime variables",
    "[vars][rtvars]"
) {
    GIVEN("Three nested fragments") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        first.addVariable("var", "VAR");
        auto &second = first.addFragment("second");
        auto &third = second.addFragment("third");

        WHEN("The fragment indexed by string") {
            Teng::Error_t err;
            auto t = "${$$.first[0]['var']}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "VAR");
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
        auto &third = second.addFragment("third");

        WHEN("The missing root variable is requested") {
            Teng::Error_t err;
            auto t = "${$$.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 5},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of first nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Runtime: The path expression '.first' references fragment "
                    "that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of second nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.second.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 18},
                    "Runtime: The path expression '.first.second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing variable of third nested fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.second.third.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 24},
                    "Runtime: The path expression '.first.second.third' "
                    "references fragment that doesn't contain any value for "
                    "key 'var' [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment is requested") {
            Teng::Error_t err;
            auto t = "${$$.first.missing.third.var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 11},
                    "Runtime: The path expression '.first' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
        auto &third = second.addFragment("third");

        WHEN("The missing root variable is requested") {
            Teng::Error_t err;
            auto t = "${$$var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 4},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'var' [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 23},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'var' [open_frags=.first, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing nested fragment runtime variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$second.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 30},
                    "Runtime: The path expression 'second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=.first, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing nested fragment variable is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${second.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 21},
                    "Runtime: The path expression 'second' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=.first, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The missing fragment is requested") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${missing.var}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for key "
                    "'missing' [open_frags=.first, iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
        auto &third = second.addFragment("third");
        root.addFragment("first");
        root.addFragment("first");

        WHEN("Variable of first fragment not exist in some iteration") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${$$one}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 23},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'one' [open_frags=.first, iteration=1]"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 23},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'one' [open_frags=.first, iteration=2]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "1undefinedundefined");
            }
        }

        WHEN("Variable in nested fragment not exist in some iteration") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>${second.test}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'second' [open_frags=.first, iteration=1]"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 21},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'second' [open_frags=.first, iteration=2]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "TESTundefinedundefined");
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
        auto &third = second.addFragment("third");
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
                    "Runtime: The index '10' is out of valid range <0, 2) "
                    "of the fragments list referenced by this path expression "
                    ".first [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    "Runtime: The index '-10' is out of valid range <0, 2) "
                    "of the fragments list referenced by this path expression "
                    ".first [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    "fragment lists which it can't be subscripted by values "
                    "of 'string_ref' type with value 'invalid' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    "fragment which it can't be subscripted by values of "
                    "'integral' type with value '0' [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    "is not subscriptable [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The first 'first' fragment identified by index") {
            Teng::Error_t err;
            auto t = "${$$.first[0].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Runtime: The path expression '.first[0]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The last 'first' fragment identified by index") {
            Teng::Error_t err;
            auto t = "${$$.first[2].var + $$.first[2].last_first}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Runtime: The path expression '.first[2]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefinedLAST");
            }
        }

        WHEN("The first 'first' fragment identified by complex scalar index") {
            Teng::Error_t err;
            auto t = "${$$.first[2 - 2].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 18},
                    "Runtime: The path expression '.first[2 - 2]' references "
                    "fragment that doesn't contain any value for key 'var' "
                    "[open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The first 'first' fragment identified by runtime index") {
            Teng::Error_t err;
            auto t = "${$$.first[$$.zero].var}";
            auto result = g(err, t, root);

            THEN("The error report contains valid path") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 20},
                    "Runtime: The path expression '.first[$$.zero]' "
                    "references fragment that doesn't contain any value for "
                    "key 'var' [open_frags=., iteration=0]"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Escaping only if needed",
    "[varsx]"
) {
    GIVEN("Template with transitive setting") {
        std::string templ = "<?teng set .q = '\"kaktus&<>\"'?>"
                            "<?teng set .r = $.q?>"
                            "<?teng set .s = $.r?>"
                            "<?teng set .t = $.s?>"
                            "${.t}";

        WHEN("Is escaped") {
            Teng::Error_t err;
            auto res = g(err, templ);
            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                std::string result = "&amp;amp;amp;quot;kaktus&amp;amp;amp;amp;"
                                     "&amp;amp;amp;lt;&amp;amp;amp;gt;&amp;amp;"
                                     "amp;quot;";
                REQUIRE(res == "&quot;kaktus&amp;&lt;&gt;&quot;");
            }
        }
    }

    GIVEN("String variable with dangerous characters") {
        Teng::Fragment_t root;
        root.addVariable("var", "\"&\"");

        WHEN("Is joined by expression with runtime variables") {
            Teng::Error_t err;
            auto res = g(err, "${len(.var) + len(.var)}", root);

            THEN("Is escaped?") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "6");
            }
        }
    }

    GIVEN("String auto runtime variable with dangerous characters") {
        Teng::Fragment_t root;
        auto &sample = root.addFragment("sample");
        sample.addVariable("var", "\"&\"");

        WHEN("Is joined by expression with runtime variables") {
            Teng::Error_t err;
            auto res = g(err, "${len(.sample.var) + len(.sample.var)}", root);

            THEN("Is escaped?") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "6");
            }
        }
    }

    GIVEN("String runtime variable with dangerous characters") {
        Teng::Fragment_t root;
        root.addVariable("var", "\"&\"");

        WHEN("Is joined by expression with runtime variables") {
            Teng::Error_t err;
            auto res = g(err, "${len($$var) + len($$var)}", root);

            THEN("Is escaped?") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "6");
            }
        }
    }
}

