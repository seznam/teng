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
    "Local regular variable syntax",
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Regular nested_1 variable is expanded") {
            Teng::Error_t err;
            auto templ = "<?teng frag nested_1?>${var}<?teng endfrag?>";
            auto result = g(err, templ, root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "Runtime: The '_this' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "Runtime: The '_parent' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "$frag$");
            }
        }

        WHEN("The _count only") {
            Teng::Error_t err;
            auto result = g(err, "${_count}", root);

            THEN("Result is the number of root fragments -> 1") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _first only") {
            Teng::Error_t err;
            auto result = g(err, "${_first}", root);

            THEN("Result is the true") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last only") {
            Teng::Error_t err;
            auto result = g(err, "${_last}", root);

            THEN("Result is the true") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner only") {
            Teng::Error_t err;
            auto result = g(err, "${_inner}", root);

            THEN("Result is the false") {
                std::vector<Teng::Error_t::Entry_t> errs = {};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Relative regular variable syntax",
    "[vars][regvars]"
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'invalid' "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'invalid' "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "33nested_1_varnested_2_var44");
            }
        }

    }
}

SCENARIO(
    "Absolute regular variable syntax",
    "[vars][regvars]"
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'invalid' "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: The path expression '.nested_1' references "
                    "fragment that doesn't contain any value for key 'invalid' "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "undefined [open_frags=.nested_1.nested_2, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "___");
            }
        }
    }
}

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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.sample, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.sample, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.sample, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "$list$");
            }
        }
    }
}

SCENARIO(
    "Variables setting",
    "[vars][regvars]"
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 284},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 320},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 331},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 351},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 204},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1.nested_2, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 358},
                    "Runtime: Variable '.nested_1.var_set' is undefined "
                    "[open_frags=.nested_1, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 369},
                    "Runtime: The path expression 'nested_2' references "
                    "fragment that doesn't contain any value for key 'var_set' "
                    "[open_frags=.nested_1, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 389},
                    "Runtime: The path expression '.nested_1.nested_2' "
                    "references fragment that doesn't contain any value for "
                    "key 'var_set' [open_frags=.nested_1, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=.nested_1, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "nested_1_var_setundefined");
            }
        }
    }
}

SCENARIO(
    "Variables setting errors",
    "[vars][regvars]"
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Escaping only if needed",
    "[vars][regvars]"
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 8},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "6");
            }
        }
    }
}

