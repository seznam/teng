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
 * Teng engine -- test of teng fragments.
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

#include <sstream>

SCENARIO(
    "Zero Teng fragments",
    "[frags]"
) {
    GIVEN("Template with one Teng fragment") {
        auto t = "<?teng frag sample?>content<?teng endfrag?>";

        WHEN("Generated with empty fragmentlist") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragmentList("sample");
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "One Teng fragment",
    "[frags]"
) {
    GIVEN("Template with one Teng fragment") {
        auto t = "<?teng frag sample?>content<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with one fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains data from fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "content");
            }
        }

        WHEN("Generated with two fragments") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains data from fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "contentcontent");
            }
        }

        WHEN("Generated with parallel fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("other");
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with variable of same name") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("sample", "");
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Two Teng fragments",
    "[frags]"
) {
    GIVEN("Template with two Teng fragments") {
        auto t = "<?teng frag sample1?>one<?teng endfrag?>"
                 "<?teng frag sample2?>two<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with with data for first fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample1");
            auto result = g(err, t, root);

            THEN("It contains data from first fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "one");
            }
        }

        WHEN("Generated with with data for second fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample2");
            auto result = g(err, t, root);

            THEN("It contains data from second fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "two");
            }
        }

        WHEN("Generated with with data for both fragments") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample1");
            root.addFragment("sample2");
            auto result = g(err, t, root);

            THEN("It contains data from both fragments") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "onetwo");
            }
        }

        WHEN("Generated with with more data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample1");
            root.addFragment("sample1");
            root.addFragment("sample2");
            auto result = g(err, t, root);

            THEN("It contains two times data from first and one from second") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "oneonetwo");
            }
        }
    }
}

SCENARIO(
    "Nested fragment",
    "[frags]"
) {
    GIVEN("Template with nested fragment") {
        auto t = "<?teng frag parent?>{<?teng frag child?>"
                 "child"
                 "<?teng endfrag?>}<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with empty parent fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("parent");
            auto result = g(err, t, root);

            THEN("It contains data from parent fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }

        WHEN("Generated with one child fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &parent = root.addFragment("parent");
            parent.addFragment("child");
            auto result = g(err, t, root);

            THEN("It contains data from parent and child fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{child}");
            }
        }

        WHEN("Generated with two children fragments") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &parent = root.addFragment("parent");
            parent.addFragment("child");
            parent.addFragment("child");
            auto result = g(err, t, root);

            THEN("It contains data from parent and both children fragments") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{childchild}");
            }
        }

        WHEN("Generated with parallel fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("other");
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with variable of same name") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("parent", "");
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "One Teng fragment referenced by absolute path",
    "[frags]"
) {
    GIVEN("Template with one Teng fragment") {
        auto t = "<?teng frag .sample?>content<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with one fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains data from fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "content");
            }
        }
    }

    GIVEN("Template with nested fragment") {
        auto t = "<?teng frag .parent?>{<?teng frag .parent?>"
                 "parent"
                 "<?teng endfrag?>}<?teng endfrag?>";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with empty parent fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("parent");
            auto result = g(err, t, root);

            THEN("It contains data from 'both' parent fragments") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{parent}");
            }
        }

        WHEN("Generated with one child parent fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &parent = root.addFragment("parent");
            parent.addFragment("parent");
            auto result = g(err, t, root);

            THEN("It contains data from parent and child fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{parent}");
            }
        }
    }
}

SCENARIO(
    "The misplaced endfrag directive",
    "[frags]"
) {
    GIVEN("Template with directive closing not opened frag") {
        auto t = "{<?teng endfrag ++++ 1 1 + 2?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }
    }
}

SCENARIO(
    "Bad name in frag directive",
    "[frags]"
) {
    GIVEN("Template with bad name in frag directive") {
        auto t = "{<?teng frag 1?>x<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid fragment identifier; discarding fragment "
                    "block content"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }
    }
}

SCENARIO(
    "Unclosed frag directive",
    "[frags]"
) {
    GIVEN("Template with unclosed fragment directive") {
        auto t = "{<?teng frag sample?>}";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("The fragment is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 22},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }
    }

    GIVEN("Template with unclosed fragment directive wrapping closed one") {
        auto t = "<?teng frag sample1?>("
                 "{<?teng frag sample2?>}<?teng endfrag?>"
                 ")";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample1")
                .addFragment("sample2");
            auto result = g(err, t, root);

            THEN("The fragment is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 62},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "({})");
            }
        }
    }

    GIVEN("Template with closed and next unclosed fragment directive") {
        auto t = "<?teng frag sample?>(<?teng endfrag?>)"
                 "{<?teng frag sample?>}";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("The fragment is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 39},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 60},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "(){}");
            }
        }
    }

    GIVEN("Template with two unclosed fragment blocks") {
        auto t = "<?teng frag sample1?>()"
                 "{<?teng frag sample2?>}";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample1")
                .addFragment("sample2");
            auto result = g(err, t, root);

            THEN("The fragment is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 24},
                    "The closing directive of this <?teng frag?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 46},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "(){}");
            }
        }
    }

    GIVEN("Template with unclosed frag directive") {
        auto t = "{<?teng frag<?teng endfrag?>}";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The closing directive of this <?teng frag?> "
                    "directive is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid fragment identifier; "
                    "discarding fragment block content"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 12},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 29},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{");
            }
        }
    }
}

SCENARIO(
    "The unopened fragment directive",
    "[frags]"
) {
    GIVEN("Template with directive closing not opened fragment") {
        auto t = "{<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }
    }

    GIVEN("Template with two directives closing not opened fragment") {
        auto t = "{<?teng endfrag?>}{<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 19},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}{}");
            }
        }
    }

    GIVEN("Fragment after directive closing not opened fragment") {
        auto t = "{<?teng endfrag?>}"
                 "{<?teng frag sample?>x<?teng endfrag?>}";

        WHEN("Generated with some data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}{x}");
            }
        }
    }

    GIVEN("Fragment before directive closing not opened fragment") {
         auto t = "{<?teng frag sample?>x<?teng endfrag?>}"
                  "{<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addFragment("sample");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 40},
                    "The <?teng endfrag?> directive closes unopened "
                    "fragment block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{x}{}");
            }
        }
    }
}

SCENARIO(
    "Dump Teng fragment",
    "[frags]"
) {
    GIVEN("One teng fragment in root fragment") {
        Teng::Fragment_t root;
        auto& mediaFrag = root.addFragment("mediaFile");
        mediaFrag.addVariable("tagContent", "cont");

        WHEN("Dump fragment via method dump()") {
            std::stringstream ss;
            root.dump(ss);
            auto result = ss.str();

            THEN("It is string encoded like JSON but with single quote") {
                REQUIRE(result == "{'mediaFile': [{'tagContent': 'cont'}]}");
            }
        }

        WHEN("Dump fragment via method json()") {
            std::stringstream ss;
            root.json(ss);
            auto result = ss.str();

            THEN("It is string encoded as JSON") {
                REQUIRE(result == R"({"mediaFile": [{"tagContent": "cont"}]})");
            }
        }
    }
}

SCENARIO(
    "Fuzzer problems in fragments",
    "[frags][fuzzer]"
) {
    GIVEN("The variable which ident consists from frag and var ident") {
        auto t = "<?frag a.r?><?if _parent.er.r?><?endif?><?endfrag?>";

        WHEN("The data corresponding to open frags") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &a = root.addFragment("a");
            a.addFragment("r");
            a.addFragment("er");
            auto result = g(err, t, root);

            THEN("The path of undefined variable is valid") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 17},
                    "Runtime: The path expression '.a.er' references "
                    "fragment that doesn't contain any value for key 'r' "
                    "[open_frags=.a.r, iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}


