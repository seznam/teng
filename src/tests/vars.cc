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
                    {1, 6},
                    "Runtime: Variable '.var' is undefined"
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
                    {1, 6},
                    "Runtime: Variable '.var' is undefined"
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

            THEN("It is undefined string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 6},
                    "Runtime: Variable '.var' is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
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
                    {1, 26},
                    "Runtime: Variable '.sample.var' is undefined"
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
                    {1, 26},
                    "Runtime: Variable '.sample.var' is undefined"
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
                    {1, 26},
                    "Runtime: Variable '.sample.var' is undefined"
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
                    "Runtime: Variable '.sample.var' is undefined"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
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
    "Regular variable syntax",
    "[vars][regvars]"
) {
    GIVEN("Some data with nested fragments and with variables") {
        Teng::Fragment_t root;
        root.addVariable("var", "root_var");
        root.addVariable("_underscore", "___");
        root.addVariable("_this", "bad_var_name_this");
        root.addVariable("_parent", "bad_var_name_parent");
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
                    "The variable names starting with underscore are "
                    "reserved"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "___");
            }
        }

        WHEN("The _this only") {
            Teng::Error_t err;
            auto result = g(err, "${_this}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The _this variable name is reserved"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_this");
            }
        }

        WHEN("The _parent only") {
            Teng::Error_t err;
            auto result = g(err, "${_parent}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "The _parent variable name is reserved"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_parent");
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
                    "Variable '.invalid.nested_2.var' doesn't match any "
                    "fragment in any context; replacing variable with "
                    "undefined value."
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
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
                    "Variable '.nested_1.invalid.var' doesn't match any "
                    "fragment in any context; replacing variable with "
                    "undefined value."
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
                    {1, 73},
                    "Runtime: Variable '.nested_1.nested_2.missing' is "
                    "undefined"
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
                    {1, 65},
                    "The variable names starting with underscore are reserved"
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
                    {1, 3},
                    "The _this variable name is reserved"
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
                    {1, 3},
                    "The _parent variable name is reserved"
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
                    "Ignoring useless _this variable path segment"
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
                    "Ignoring useless _this variable path segment"
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
                    "Variable identifier 'invalid.nested_2.var' not found"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Variable identifier '' is invalid"
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
                    "Variable identifier 'invalid.var' not found"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Variable identifier '' is invalid"
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
                    "Ignoring useless _this relative variable path segment"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 70},
                    "Ignoring useless _this relative variable path segment"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "nested_2_var");
            }
        }

        WHEN("The _parent inside relative path is ignored") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>"
                         "<?teng frag nested_2?>"
                         "${nested_2._parent.var}"
                         "<?teng endfrag?>"
                         "<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 55},
                    "Ignoring invalid _parent relative variable path segment"
                }};
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
                    {1, 55},
                    "The _this variable name is reserved"
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
                    {1, 55},
                    "The _parent variable name is reserved"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "bad_var_name_parent_2");
            }
        }
    }
}

// SCENARIO(
//     "Runtime queries for variables",
//     "[vars]"
// ) {
//     GIVEN("Some data with nested fragments and with variables") {
//         Teng::Fragment_t root;
//         root.addVariable("var", "root_var");
//         auto &nested_1 = root.addFragment("nested_1");
//         nested_1.addVariable("var", "nested_1_var");
//         auto &nested_2 = nested_1.addFragment("nested_2");
//         nested_2.addVariable("var", "nested_2_var");
//
//         WHEN("Regular (root) variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "root_var");
//             }
//         }
//
//         WHEN("Regular (root) invalid variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$invalid}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs = {{
//                     Teng::Error_t::WARNING,
//                     {1, 12},
//                     "Runtime: Unable to locate member (3) 'invalid'"
//                 }};
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "$null$");
//             }
//         }
//
//         WHEN("Absolute root variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$.var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "root_var");
//             }
//         }
//
//         WHEN("Regular nested variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "<?teng frag nested_1?>"
//                                  "${$$var}"
//                                  "<?teng endfrag?>", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_1_var");
//             }
//         }
//
//         WHEN("Absolute nested variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "<?teng frag nested_1?>"
//                                  "${$$.nested_1.var}"
//                                  "<?teng endfrag?>", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_1_var");
//             }
//         }
//
//         WHEN("Absolute nested invalid variable is expanded") {
//             Teng::Error_t err;
//             auto result = g(err, "<?teng frag nested_1?>"
//                                  "${$$.nested_1.invalid}"
//                                  "<?teng endfrag?>", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs = {{
//                     Teng::Error_t::WARNING,
//                     {1, 44},
//                     "Runtime: Unable to locate member (3) 'invalid'"
//                 }};
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "$null$");
//             }
//         }
//
//         WHEN("Absolute nested second variable is expanded") {
//             Teng::Error_t err;
//             auto &second = root.addFragment("nested_1");
//             second.addVariable("second_var", "FGHJ");
//             auto result = g(err, "${$$.nested_1[1].second_var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "FGHJ");
//             }
//         }
//
//         WHEN("Use string index to access nested fragment") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$nested_1['nested_2'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//
//         WHEN("Use string index to access nested fragment (absolute path)") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$.nested_1['nested_2'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//     }
// }
//
// SCENARIO(
//     "Runtime queries for variables - builtin variables",
//     "[vars]"
// ) {
//     GIVEN("Some data with nested fragments and with variables") {
//         Teng::Fragment_t root;
//         root.addVariable("var", "root_var");
//         auto &nested_1 = root.addFragment("nested_1");
//         nested_1.addVariable("var", "nested_1_var");
//         auto &nested_2 = nested_1.addFragment("nested_2");
//         nested_2.addVariable("var", "nested_2_var");
//
//         WHEN("The _this as first segment in query") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$_this['nested_1'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_1_var");
//             }
//         }
//
//         WHEN("The _this as first (after selector) segment in query") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$._this['nested_1'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_1_var");
//             }
//         }
//
//         WHEN("The _this as first segment in query (invalid variable)") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$_this['nested_1'].invalid}", root);
//
//             THEN("Result is undefined") {
//                 std::vector<Teng::Error_t::Entry_t> errs = {{
//                     Teng::Error_t::WARNING,
//                     {1, 30},
//                     "Runtime: Unable to locate member (3) 'invalid'"
//                 }};
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "$null$");
//             }
//         }
//
//         WHEN("The _this as second segment in query") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$nested_1._this['nested_2'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//
//         WHEN("The _this as second (after selector) segment in query") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$.nested_1._this['nested_2'].var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//
//         WHEN("The _this as second segment in query (invalid segment)") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$invalid._this['nested_2'].var}", root);
//
//             THEN("Result is undefined") {
//                 std::vector<Teng::Error_t::Entry_t> errs = {{
//                     Teng::Error_t::WARNING,
//                     {1, 34},
//                     "Runtime: Unable to locate member (3) 'invalid'"
//                 }, {
//                     Teng::Error_t::WARNING,
//                     {1, 34},
//                     "Runtime: String indices can be used for fragments only"
//                 }};
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "$null$");
//             }
//         }
//
//         WHEN("The _this as second segment in query (invalid index)") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$nested_1._this['invalid'].var}", root);
//
//             THEN("Result is undefined") {
//                 std::vector<Teng::Error_t::Entry_t> errs = {{
//                     Teng::Error_t::WARNING,
//                     {1, 34},
//                     "Runtime: Unable to locate member (1) 'invalid'"
//                 }};
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "$null$");
//             }
//         }
//
//         WHEN("The _this before variable") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$nested_1.nested_2._this.var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//
//         WHEN("The _this in middle of variable path") {
//             Teng::Error_t err;
//             auto result = g(err, "${$$nested_1._this.nested_2.var}", root);
//
//             THEN("Result is variable value") {
//                 std::vector<Teng::Error_t::Entry_t> errs;
//                 REQUIRE(err.getEntries() == errs);
//                 REQUIRE(result == "nested_2_var");
//             }
//         }
//     }
// }
//
