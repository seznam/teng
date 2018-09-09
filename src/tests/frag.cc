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

#include <teng.h>

// TODO(burlog): remove
#include <iostream>

#include "catch.hpp"
#include "utils.h"

// TODO(burlog): !!!!!!!!!!!!!!!
// "${$$.g[0]['h']['f']}"
// "${$$.g[-1]['h']['f']}"
// "${$$.a.b[0].c}"
// "${$$.a[0].b[0+0].c}"
// "${$$.a['b'].c}"
// "${.z}==${$$.z}|"
// "${.alpha}==${$$.alpha}|"
// "<?teng frag f?>${$$f}<?teng endfrag?>" // ==${$$.f.f}|"
// "<?teng frag a?><?teng frag b?>${c}<?teng endfrag?>==${$$.a.b.c}<?teng endfrag?>|"
// "<?teng frag f?>${.alpha}==${$$.f[0+a].f ++ $$.alpha}<?teng endfrag?>"
// "${.alpha}${alpha}${$$alpha}${0+0}"
// "<?teng frag f?>${exists($$.f[0])}<?teng endfrag?>"
// "<?teng frag f?>${exists($$.f[3])}<?teng endfrag?>"
// "<?teng frag g?><?teng frag h?>${exists($$.g[0].h)}<?teng endfrag?><?teng endfrag?>"
// "${z}"
// "<?teng frag b?>"
// "<?teng frag .a?>"
// "${b.c}"
// "${z}"
// "<?teng endfrag?>"
// "<?teng endfrag?>"
// "<?teng endfrag?>"
//"${'abc'}"
//"${'abc'}"
//"<?teng format space='nospace'?>"
//"${'abc'}"
//"<?teng endformat ?>"
// "${1||2||3}"
//"<?teng format 'text'?>"
// "<?teng if 0?>"
// "a"
// "<?teng elif 0?>"
// "b"
// "<?teng elif 0?>"
// "b2"
// "${case(2, 1: 'jedna'+a, 3,6,7,2: 'dva'++'dva', 5,4: 'dva'++'dva', *: 'ostatni')}"
// "${1 || (1 + a)}"
// "<?teng else?>"
// "c"
// "<?teng endif?>"
//"<?teng endformat a ?>"

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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with empty parent fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &parent = root.addFragment("parent");
            auto result = g(err, t, root);

            THEN("It contains data from parent fragment") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Generated with empty parent fragment") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto &parent = root.addFragment("parent");
            auto result = g(err, t, root);

            THEN("It contains data from 'both' parent fragments") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
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
                REQUIRE(err.getEntries() == errs);
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
        auto t = "{<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Missing <?teng frag ...?> of this endfrag"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "Misplaced or excessive '?>' token"
                }};
                REQUIRE(err.getEntries() == errs);
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
                    {1, 13},
                    "Unexpected token: name=DEC_INT, view=1"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Invalid fragment identifier; discarding fragment "
                    "block content"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}");
            }
        }
    }
}

SCENARIO(
    "Unclosed frag directive",
    "[frags]"
) {
    GIVEN("Template with unclosed frag directive") {
        auto t = "{<?teng frag<?teng endfrag?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Missing <?teng frag ...?> of this endfrag"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 12},
                    "Invalid fragment identifier; discarding fragment "
                    "block content"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 29},
                    "Unexpected token: name=<EOF>, view="
                }, {
                    Teng::Error_t::FATAL,
                    {0, 0},
                    "Unrecoverable syntax error; discarding whole program"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Unclosed <?teng frag ?> directive"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "");
            }
        }
    }
}


