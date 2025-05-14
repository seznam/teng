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
 * Teng engine -- test of teng builtin variables.
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
    "Using _this/_parent to traverse through frag tree for absolute path",
    "[vars][builtinvars]"
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
                    "Runtime: The '_this' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "Runtime: The '_parent' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "root_var");
            }
        }

        WHEN("Accessing root variable via _this") {
            Teng::Error_t err;
            auto result = g(err, "${_this.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "nested_1_var");
            }
        }

        WHEN("Accessing root variable via _parent") {
            Teng::Error_t err;
            auto result = g(err, "${.nested_1._parent.var}", root);

            THEN("Result is variable value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "nested_2_var");
            }
        }
    }
}

SCENARIO(
    "Using _this/_parent to traverse through frag tree for relative path",
    "[vars][builtinvars]"
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "Runtime: The '_this' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
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
                    "Runtime: The '_parent' identifier is reserved; "
                    "don't use it, please"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "bad_var_name_parent_2");
            }
        }
    }
}

SCENARIO(
    "Builtin variables in root fragment",
    "[vars][builtinvars]"
) {
    GIVEN("Data with no fragments") {
        Teng::Fragment_t root;

        WHEN("The _first variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_first}";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_last}";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_inner}";
            auto result = g(err, t, root);

            THEN("It expands to false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _index variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_index}";
            auto result = g(err, t, root);

            THEN("It expands to 0 (first frag)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _count variable is used in root fragment") {
            Teng::Error_t err;
            auto t = "${_count}";
            auto result = g(err, t, root);

            THEN("It expands to 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Builtin variables in nested fragments",
    "[vars][builtinvars]"
) {
    GIVEN("Data with no fragments") {
        Teng::Fragment_t root;

        WHEN("The _first variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_first}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to nothing") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0 (first frag)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "10");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "01");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "00");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0, 1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "01");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 2, 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
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
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "100");
            }
        }

        WHEN("The _last variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_last}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, false, true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "001");
            }
        }

        WHEN("The _inner variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_inner}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to false, true, false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "010");
            }
        }

        WHEN("The _index variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_index}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 0, 1, 2") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "012");
            }
        }

        WHEN("The _count variable is used in nested fragment") {
            Teng::Error_t err;
            auto t = "<?teng frag nested?>${_count}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It expands to 3, 3, 3") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "333");
            }
        }
    }
}

SCENARIO(
    "The builtin variables returns same values regardless of its type",
    "[vars][builtinvars]"
) {
    GIVEN("The _count builtin variable") {
        WHEN("It is used for root fragment") {
            Teng::Error_t err;
            std::string t = "${_count}"
                            "==${._count}"
                            "==${$$_count}"
                            "==${$$._count}"
                            "==${repr(_count)}"
                            "==${repr(._count)}"
                            "==${repr($$_count)}"
                            "==${repr($$._count)}"
                            "==${count(_this)}"
                            "==${count(._this)}"
                            "==${count($$_this)}"
                            "==${count($$._this)}";
            Teng::Fragment_t root;
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 27},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 126},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 143},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 161},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 169},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 180},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for missing fragment") {
            Teng::Error_t err;
            std::string t = "${missing._count}"
                            "==${.missing._count}"
                            "==${$$missing._count}"
                            "==${$$.missing._count}"
                            "==${repr(missing._count)}"
                            "==${repr(.missing._count)}"
                            "==${repr($$missing._count)}"
                            "==${repr($$.missing._count)}"
                            "==${count(missing)}"
                            "==${count(.missing)}"
                            "==${count($$missing)}"
                            "==${count($$.missing)}";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 43},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 190},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 190},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 209},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 209},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 229},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 229},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 237},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 250},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 250},
                    "Runtime: The path expression references object of "
                    "'undefined' type with value 'undefined' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for subfrag") {
            Teng::Error_t err;
            std::string t = "${subfrag._count}"
                            "==${.subfrag._count}"
                            "==${$$subfrag._count}"
                            "==${$$.subfrag._count}"
                            "==${repr(subfrag._count)}"
                            "==${repr(.subfrag._count)}"
                            "==${repr($$subfrag._count)}"
                            "==${repr($$.subfrag._count)}"
                            "==${count(subfrag)}"
                            "==${count(.subfrag)}"
                            "==${count($$subfrag)}"
                            "==${count($$.subfrag)}";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' for which "
                    "is _count builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' for which "
                    "is _count builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 51},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' for which "
                    "is _count builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' for which "
                    "is _count builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 190},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 190},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 209},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 209},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 229},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 229},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 237},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 250},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 250},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for empty frag list") {
            Teng::Error_t err;
            std::string t = "${nested._count}"
                            "==${.nested._count}"
                            "==${$$nested._count}"
                            "==${$$.nested._count}"
                            "==${repr(nested._count)}"
                            "==${repr(.nested._count)}"
                            "==${repr($$nested._count)}"
                            "==${repr($$.nested._count)}"
                            "==${count(nested)}"
                            "==${count(.nested)}"
                            "==${count($$nested)}"
                            "==${count($$.nested)}";
            Teng::Fragment_t root;
            root.addFragmentList("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 182},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 200},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 219},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 227},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 239},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for frag list with one item") {
            Teng::Error_t err;
            std::string t = "${nested._count}"
                            "==${.nested._count}"
                            "==${$$nested._count}"
                            "==${$$.nested._count}"
                            "==${repr(nested._count)}"
                            "==${repr(.nested._count)}"
                            "==${repr($$nested._count)}"
                            "==${repr($$.nested._count)}"
                            "==${count(nested)}"
                            "==${count(.nested)}"
                            "==${count($$nested)}"
                            "==${count($$.nested)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 182},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 200},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 219},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 227},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 239},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for frag list with two items") {
            Teng::Error_t err;
            std::string t = "${nested._count}"
                            "==${.nested._count}"
                            "==${$$nested._count}"
                            "==${$$.nested._count}"
                            "==${repr(nested._count)}"
                            "==${repr(.nested._count)}"
                            "==${repr($$nested._count)}"
                            "==${repr($$.nested._count)}"
                            "==${count(nested)}"
                            "==${count(.nested)}"
                            "==${count($$nested)}"
                            "==${count($$.nested)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 182},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 200},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 219},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 227},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 239},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "2==2==2==2==2==2==2==2==2==2==2==2");
            }
        }

        WHEN("It is used for opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>"
                            "${_count}"
                            "==${.nested._count}"
                            "==${$$_this._count}"
                            "==${$$_count}"
                            "==${$$.nested._count}"
                            "==${repr(_count)}"
                            "==${repr(.nested._count)}"
                            "==${repr($$_count)}"
                            "==${repr($$.nested._count)}"
                            "==${count(.nested)}"
                            "==${count($$.nested)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 193},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 212},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for three opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_first}"
                            "==${.nested._first}"
                            "==${$$_this._first}"
                            "==${$$_first}"
                            "==${$$.nested._first}"
                            "==${repr(_first)}"
                            "==${repr(.nested._first)}"
                            "==${repr($$_first)}"
                            "==${repr($$.nested._first)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            root.addFragment("nested");
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _first variable is undefined "
                    "[open_frags=.nested, iteration=0/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _first variable is undefined "
                    "[open_frags=.nested, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _first variable is undefined "
                    "[open_frags=.nested, iteration=2/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==undefined==1==1==1==undefined"
                               "0==0==0==0==undefined==0==0==0==undefined"
                               "0==0==0==0==undefined==0==0==0==undefined");
            }
        }

        WHEN("It is used for opened nested subfragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag subfrag?>${_count}"
                            "==${.subfrag._count}"
                            "==${$$_this._count}"
                            "==${$$_count}"
                            "==${$$.subfrag._count}"
                            "==${repr(_count)}"
                            "==${repr(.subfrag._count)}"
                            "==${repr($$_count)}"
                            "==${repr($$.subfrag._count)}"
                            "==${count(.subfrag)}"
                            "==${count($$.subfrag)}<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Can't determine '.subfrag' frag count "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 34},
                    "Runtime: Can't determine '.subfrag' frag count "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "Runtime: Can't determine '.subfrag' frag count "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 75},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 75},
                    "Runtime: Can't determine '.subfrag' frag count "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 97},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _count builtin variable undefined "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 158},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 198},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 198},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 218},
                    "The count() query is deprecated; "
                    "use _count builtin variable instead"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 218},
                    "Runtime: The path expression references object of "
                    "'frag_ref' type with value '$frag$' for which count() "
                    "query is undefined [open_frags=.subfrag, iteration=0/0]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined");
            }
        }
    }

    GIVEN("The _first builtin variable") {
        WHEN("It is used for root fragment") {
            Teng::Error_t err;
            std::string t = "${_first}"
                            "==${._first}"
                            "==${$$_first}"
                            "==${$$._first}"
                            "==${repr(_first)}"
                            "==${repr(._first)}"
                            "==${repr($$_first)}"
                            "==${repr($$._first)}";
            Teng::Fragment_t root;
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 27},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for missing fragment") {
            Teng::Error_t err;
            std::string t = "${missing._first}"
                            "==${.missing._first}"
                            "==${$$missing._first}"
                            "==${$$.missing._first}"
                            "==${repr(missing._first)}"
                            "==${repr(.missing._first)}"
                            "==${repr($$missing._first)}"
                            "==${repr($$.missing._first)}";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 43},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for subfrag") {
            Teng::Error_t err;
            std::string t = "${subfrag._first}"
                            "==${.subfrag._first}"
                            "==${$$subfrag._first}"
                            "==${$$.subfrag._first}"
                            "==${repr(subfrag._first)}"
                            "==${repr(.subfrag._first)}"
                            "==${repr($$subfrag._first)}"
                            "==${repr($$.subfrag._first)}";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _first builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _first builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 51},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _first builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _first builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for empty frag list") {
            Teng::Error_t err;
            std::string t = "${nested._first}"
                            "==${.nested._first}"
                            "==${$$nested._first}"
                            "==${$$.nested._first}"
                            "==${repr(nested._first)}"
                            "==${repr(.nested._first)}"
                            "==${repr($$nested._first)}"
                            "==${repr($$.nested._first)}";
            Teng::Fragment_t root;
            root.addFragmentList("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for frag list with one item") {
            Teng::Error_t err;
            std::string t = "${nested._first}"
                            "==${.nested._first}"
                            "==${$$nested._first}"
                            "==${$$.nested._first}"
                            "==${repr(nested._first)}"
                            "==${repr(.nested._first)}"
                            "==${repr($$nested._first)}"
                            "==${repr($$.nested._first)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for frag list with two items") {
            Teng::Error_t err;
            std::string t = "${nested._first}"
                            "==${.nested._first}"
                            "==${$$nested._first}"
                            "==${$$.nested._first}"
                            "==${repr(nested._first)}"
                            "==${repr(.nested._first)}"
                            "==${repr($$nested._first)}"
                            "==${repr($$.nested._first)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _first variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_first}"
                            "==${.nested._first}"
                            "==${$$_this._first}"
                            "==${$$_first}"
                            "==${$$.nested._first}"
                            "==${repr(_first)}"
                            "==${repr(.nested._first)}"
                            "==${repr($$_first)}"
                            "==${repr($$.nested._first)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for three opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_last}"
                            "==${.nested._last}"
                            "==${$$_this._last}"
                            "==${$$_last}"
                            "==${$$.nested._last}"
                            "==${repr(_last)}"
                            "==${repr(.nested._last)}"
                            "==${repr($$_last)}"
                            "==${repr($$.nested._last)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            root.addFragment("nested");
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 58},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 70},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 90},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _last variable is undefined "
                    "[open_frags=.nested, iteration=0/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 90},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _last variable is undefined "
                    "[open_frags=.nested, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 90},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _last variable is undefined "
                    "[open_frags=.nested, iteration=2/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 147},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==undefined==0==0==0==undefined"
                               "0==0==0==0==undefined==0==0==0==undefined"
                               "1==1==1==1==undefined==1==1==1==undefined");
            }
        }

        WHEN("It is used for opened nested subfragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag subfrag?>${_first}"
                            "==${.subfrag._first}"
                            "==${$$_this._first}"
                            "==${$$_this._this._first}"
                            "==${$$_first}"
                            "==${$$.subfrag._first}"
                            "==${repr(_first)}"
                            "==${repr(.subfrag._first)}"
                            "==${repr($$_first)}"
                            "==${repr($$.subfrag._first)}"
                            "<?teng endfrag?>";
                            ;
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 34},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 122},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _first builtin variable undefined "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 183},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined");
            }
        }
    }

    GIVEN("The _last builtin variable") {
        WHEN("It is used for root fragment") {
            Teng::Error_t err;
            std::string t = "${_last}"
                            "==${._last}"
                            "==${$$_last}"
                            "==${$$._last}"
                            "==${repr(_last)}"
                            "==${repr(._last)}"
                            "==${repr($$_last)}"
                            "==${repr($$._last)}";
            Teng::Fragment_t root;
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 25},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 88},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for missing fragment") {
            Teng::Error_t err;
            std::string t = "${missing._last}"
                            "==${.missing._last}"
                            "==${$$missing._last}"
                            "==${$$.missing._last}"
                            "==${repr(missing._last)}"
                            "==${repr(.missing._last)}"
                            "==${repr($$missing._last)}"
                            "==${repr($$.missing._last)}";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 41},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for subfrag") {
            Teng::Error_t err;
            std::string t = "${subfrag._last}"
                            "==${.subfrag._last}"
                            "==${$$subfrag._last}"
                            "==${$$.subfrag._last}"
                            "==${repr(subfrag._last)}"
                            "==${repr(.subfrag._last)}"
                            "==${repr($$subfrag._last)}"
                            "==${repr($$.subfrag._last)}";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _last builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _last builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 49},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _last builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 70},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _last builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for empty frag list") {
            Teng::Error_t err;
            std::string t = "${nested._last}"
                            "==${.nested._last}"
                            "==${$$nested._last}"
                            "==${$$.nested._last}"
                            "==${repr(nested._last)}"
                            "==${repr(.nested._last)}"
                            "==${repr($$nested._last)}"
                            "==${repr($$.nested._last)}";
            Teng::Fragment_t root;
            root.addFragmentList("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 19},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 66},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for frag list with one item") {
            Teng::Error_t err;
            std::string t = "${nested._last}"
                            "==${.nested._last}"
                            "==${$$nested._last}"
                            "==${$$.nested._last}"
                            "==${repr(nested._last)}"
                            "==${repr(.nested._last)}"
                            "==${repr($$nested._last)}"
                            "==${repr($$.nested._last)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for frag list with two items") {
            Teng::Error_t err;
            std::string t = "${nested._last}"
                            "==${.nested._last}"
                            "==${$$nested._last}"
                            "==${$$.nested._last}"
                            "==${repr(nested._last)}"
                            "==${repr(.nested._last)}"
                            "==${repr($$nested._last)}"
                            "==${repr($$.nested._last)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 19},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 46},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 66},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _last variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_last}"
                            "==${.nested._last}"
                            "==${$$_this._last}"
                            "==${$$_last}"
                            "==${$$.nested._last}"
                            "==${repr(_last)}"
                            "==${repr(.nested._last)}"
                            "==${repr($$_last)}"
                            "==${repr($$.nested._last)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 58},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 70},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 147},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "1==1==1==1==1==1==1==1==1");
            }
        }

        WHEN("It is used for opened nested subfragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag subfrag?>${_last}"
                            "==${.subfrag._last}"
                            "==${$$_this._last}"
                            "==${$$_this._this._last}"
                            "==${$$_last}"
                            "==${$$.subfrag._last}"
                            "==${repr(_last)}"
                            "==${repr(.subfrag._last)}"
                            "==${repr($$_last)}"
                            "==${repr($$.subfrag._last)}"
                            "<?teng endfrag?>";
                            ;
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 33},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 84},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 84},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 96},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 96},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 117},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _last builtin variable undefined "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 175},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined");
            }
        }
    }

    GIVEN("The _inner builtin variable") {
        WHEN("It is used for root fragment") {
            Teng::Error_t err;
            std::string t = "${_inner}"
                            "==${._inner}"
                            "==${$$_inner}"
                            "==${$$._inner}"
                            "==${repr(_inner)}"
                            "==${repr(._inner)}"
                            "==${repr($$_inner)}"
                            "==${repr($$._inner)}";
            Teng::Fragment_t root;
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 27},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for missing fragment") {
            Teng::Error_t err;
            std::string t = "${missing._inner}"
                            "==${.missing._inner}"
                            "==${$$missing._inner}"
                            "==${$$.missing._inner}"
                            "==${repr(missing._inner)}"
                            "==${repr(.missing._inner)}"
                            "==${repr($$missing._inner)}"
                            "==${repr($$.missing._inner)}";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 43},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for subfrag") {
            Teng::Error_t err;
            std::string t = "${subfrag._inner}"
                            "==${.subfrag._inner}"
                            "==${$$subfrag._inner}"
                            "==${$$.subfrag._inner}"
                            "==${repr(subfrag._inner)}"
                            "==${repr(.subfrag._inner)}"
                            "==${repr($$subfrag._inner)}"
                            "==${repr($$.subfrag._inner)}";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _inner builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _inner builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 51},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _inner builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _inner builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for empty frag list") {
            Teng::Error_t err;
            std::string t = "${nested._inner}"
                            "==${.nested._inner}"
                            "==${$$nested._inner}"
                            "==${$$.nested._inner}"
                            "==${repr(nested._inner)}"
                            "==${repr(.nested._inner)}"
                            "==${repr($$nested._inner)}"
                            "==${repr($$.nested._inner)}";
            Teng::Fragment_t root;
            root.addFragmentList("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for frag list with one item") {
            Teng::Error_t err;
            std::string t = "${nested._inner}"
                            "==${.nested._inner}"
                            "==${$$nested._inner}"
                            "==${$$.nested._inner}"
                            "==${repr(nested._inner)}"
                            "==${repr(.nested._inner)}"
                            "==${repr($$nested._inner)}"
                            "==${repr($$.nested._inner)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for frag list with two items") {
            Teng::Error_t err;
            std::string t = "${nested._inner}"
                            "==${.nested._inner}"
                            "==${$$nested._inner}"
                            "==${$$.nested._inner}"
                            "==${repr(nested._inner)}"
                            "==${repr(.nested._inner)}"
                            "==${repr($$nested._inner)}"
                            "==${repr($$.nested._inner)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _inner variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_inner}"
                            "==${.nested._inner}"
                            "==${$$_this._inner}"
                            "==${$$_inner}"
                            "==${$$.nested._inner}"
                            "==${repr(_inner)}"
                            "==${repr(.nested._inner)}"
                            "==${repr($$_inner)}"
                            "==${repr($$.nested._inner)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for three opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_inner}"
                            "==${.nested._inner}"
                            "==${$$_this._inner}"
                            "==${$$_inner}"
                            "==${$$.nested._inner}"
                            "==${repr(_inner)}"
                            "==${repr(.nested._inner)}"
                            "==${repr($$_inner)}"
                            "==${repr($$.nested._inner)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            root.addFragment("nested");
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _inner variable is undefined "
                    "[open_frags=.nested, iteration=0/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _inner variable is undefined "
                    "[open_frags=.nested, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _inner variable is undefined "
                    "[open_frags=.nested, iteration=2/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==undefined==0==0==0==undefined"
                               "1==1==1==1==undefined==1==1==1==undefined"
                               "0==0==0==0==undefined==0==0==0==undefined");
            }
        }

        WHEN("It is used for opened nested subfragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag subfrag?>${_inner}"
                            "==${.subfrag._inner}"
                            "==${$$_this._inner}"
                            "==${$$_this._this._inner}"
                            "==${$$_inner}"
                            "==${$$.subfrag._inner}"
                            "==${repr(_inner)}"
                            "==${repr(.subfrag._inner)}"
                            "==${repr($$_inner)}"
                            "==${repr($$.subfrag._inner)}"
                            "<?teng endfrag?>";
                            ;
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 34},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 122},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _inner builtin variable undefined "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 183},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined");
            }
        }
    }

    GIVEN("The _index builtin variable") {
        WHEN("It is used for root fragment") {
            Teng::Error_t err;
            std::string t = "${_index}"
                            "==${._index}"
                            "==${$$_index}"
                            "==${$$._index}"
                            "==${repr(_index)}"
                            "==${repr(._index)}"
                            "==${repr($$_index)}"
                            "==${repr($$._index)}";
            Teng::Fragment_t root;
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 27},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for missing fragment") {
            Teng::Error_t err;
            std::string t = "${missing._index}"
                            "==${.missing._index}"
                            "==${$$missing._index}"
                            "==${$$.missing._index}"
                            "==${repr(missing._index)}"
                            "==${repr(.missing._index)}"
                            "==${repr($$missing._index)}"
                            "==${repr($$.missing._index)}";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 43},
                    "Runtime: This fragment doesn't contain any value for "
                    "key 'missing' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "Runtime: The path expression '.' references fragment "
                    "that doesn't contain any value for key 'missing' "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for subfrag") {
            Teng::Error_t err;
            std::string t = "${subfrag._index}"
                            "==${.subfrag._index}"
                            "==${$$subfrag._index}"
                            "==${$$.subfrag._index}"
                            "==${repr(subfrag._index)}"
                            "==${repr(.subfrag._index)}"
                            "==${repr($$subfrag._index)}"
                            "==${repr($$.subfrag._index)}";
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _index builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _index builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 51},
                    "Runtime: The path expression 'subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _index builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _index builtin variable undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for empty frag list") {
            Teng::Error_t err;
            std::string t = "${nested._index}"
                            "==${.nested._index}"
                            "==${$$nested._index}"
                            "==${$$.nested._index}"
                            "==${repr(nested._index)}"
                            "==${repr(.nested._index)}"
                            "==${repr($$nested._index)}"
                            "==${repr($$.nested._index)}";
            Teng::Fragment_t root;
            root.addFragmentList("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list "
                    "that does not contain any fragment; "
                    "_index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list "
                    "that does not contain any fragment; "
                    "_index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for frag list with one item") {
            Teng::Error_t err;
            std::string t = "${nested._index}"
                            "==${.nested._index}"
                            "==${$$nested._index}"
                            "==${$$.nested._index}"
                            "==${repr(nested._index)}"
                            "==${repr(.nested._index)}"
                            "==${repr($$nested._index)}"
                            "==${repr($$.nested._index)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for frag list with two items") {
            Teng::Error_t err;
            std::string t = "${nested._index}"
                            "==${.nested._index}"
                            "==${$$nested._index}"
                            "==${$$.nested._index}"
                            "==${repr(nested._index)}"
                            "==${repr(.nested._index)}"
                            "==${repr($$nested._index)}"
                            "==${repr($$.nested._index)}";
            Teng::Fragment_t root;
            auto &list = root.addFragmentList("nested");
            list.addFragment();
            list.addFragment();
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 48},
                    "Runtime: The path 'nested' references fragment list of "
                    "2 fragments; _index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 69},
                    "Runtime: The path '.nested' references fragment list of "
                    "2 fragments; _index variable is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined");
            }
        }

        WHEN("It is used for opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_index}"
                            "==${.nested._index}"
                            "==${$$_this._index}"
                            "==${$$_index}"
                            "==${$$.nested._index}"
                            "==${repr(_index)}"
                            "==${repr(.nested._index)}"
                            "==${repr($$_index)}"
                            "==${repr($$.nested._index)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==0==0==0==0==0");
            }
        }

        WHEN("It is used for three opened nested fragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag nested?>${_index}"
                            "==${.nested._index}"
                            "==${$$_this._index}"
                            "==${$$_index}"
                            "==${$$.nested._index}"
                            "==${repr(_index)}"
                            "==${repr(.nested._index)}"
                            "==${repr($$_index)}"
                            "==${repr($$.nested._index)}"
                            "<?teng endfrag?>";
            Teng::Fragment_t root;
            root.addFragment("nested");
            root.addFragment("nested");
            root.addFragment("nested");
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 73},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _index variable is undefined "
                    "[open_frags=.nested, iteration=0/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _index variable is undefined "
                    "[open_frags=.nested, iteration=1/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 94},
                    "Runtime: The path '.nested' references fragment list "
                    "of 3 fragments; _index variable is undefined "
                    "[open_frags=.nested, iteration=2/3]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 154},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "0==0==0==0==undefined==0==0==0==undefined"
                               "1==1==1==1==undefined==1==1==1==undefined"
                               "2==2==2==2==undefined==2==2==2==undefined");
            }
        }

        WHEN("It is used for opened nested subfragment") {
            Teng::Error_t err;
            std::string t = "<?teng frag subfrag?>${_index}"
                            "==${.subfrag._index}"
                            "==${$$_this._index}"
                            "==${$$_this._this._index}"
                            "==${$$_index}"
                            "==${$$.subfrag._index}"
                            "==${repr(_index)}"
                            "==${repr(.subfrag._index)}"
                            "==${repr($$_index)}"
                            "==${repr($$.subfrag._index)}"
                            "<?teng endfrag?>";
                            ;
            Teng::Fragment_t root;
            root.addValue("subfrag", Teng::Fragment_t());
            auto res = g(err, t, root);

            THEN("Returns same values") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 34},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 62},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 100},
                    "Runtime: Can't determine '.subfrag' frag index "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 122},
                    "Runtime: The path expression '.subfrag' references "
                    "object of 'frag_ref' type with value '$frag$' "
                    "for which is _index builtin variable undefined "
                    "[open_frags=.subfrag, iteration=0/0]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 183},
                    "The runtime variable is useless; "
                    "converting it to regular variable"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(res == "undefined==undefined==undefined==undefined"
                               "==undefined==undefined==undefined==undefined"
                               "==undefined==undefined");
            }
        }
    }
}

SCENARIO(
    "Builtin variables in the middle of path behaves identically "
    "for regular and for runtime variables",
    "[vars][builtinvars]"
) {
    GIVEN("The fragment with _count name") {
        Teng::Fragment_t root;
        auto &first = root.addFragment("first");
        auto &second = first.addFragment("_count");
        second.addVariable("var", "invalid_value");

        WHEN("The _count variable is used as second segment") {
            Teng::Error_t err;
            auto t = "${$$first._count.var}==${$first._count.var}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 17},
                    "Runtime: The path expression 'first._count' "
                    "references fragment that doesn't contain any "
                    "value for key 'var' [open_frags=., iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 26},
                    "Runtime: The path expression 'first._count' "
                    "references fragment that doesn't contain any "
                    "value for key 'var' [open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined==undefined");
            }
        }

        WHEN("The _count variable is used as second segment in opened frag") {
            Teng::Error_t err;
            auto t = "<?teng frag first?>"
                     "${$$_count.var}==${$_count.var}"
                     "<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 23},
                    "Runtime: The path expression '' references object of "
                    "'frag_ref' type with value '$frag$' for which is "
                    "_count builtin variable undefined "
                    "[open_frags=.first, iteration=0/1]"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 39},
                    "Runtime: The path expression '' references object of "
                    "'frag_ref' type with value '$frag$' for which is "
                    "_count builtin variable undefined "
                    "[open_frags=.first, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined==undefined");
            }
        }

        WHEN("The _error fragment is used as second segment") {
            Teng::Error_t err;
            auto t = "${missing}"
                     "${$$first._error.column}==${$first._error.column}";
            auto result = g(err, t, root, "teng.debug.conf");

            THEN("The result is colno of the ${} error") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Variable '.missing' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined2==2");
            }
        }
    }
}

