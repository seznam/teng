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
 * Teng engine -- ctype tests.
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
    "Change content type of text",
    "[ctype]"
) {
    GIVEN("Content type block that change ctype to application/x-sh") {
        auto t = "${danger}<?teng ctype 'application/x-sh'?>"
                 "${danger}<?teng endctype?>${danger}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("danger", "&<>\"");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped in html blocks only") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    result == "&amp;&lt;&gt;&quot;&<>\"&amp;&lt;&gt;&quot;"
                );
            }
        }
    }

    GIVEN("Content type block that change ctype to unknown ctype") {
        auto t = "${danger}<?teng ctype 'unknown/unknown'?>"
                 "${danger}<?teng endctype?>${danger}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("danger", "&<>\"");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 9},
                    "Invalid content type 'unknown/unknown'; using top instead"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    result
                    == "&amp;&lt;&gt;&quot;"
                       "&amp;&lt;&gt;&quot;&amp;&lt;&gt;&quot;"
                );
            }
        }
    }
}

SCENARIO(
    "Unclosed ctype directive",
    "[ctype]"
) {
    GIVEN("Template with unclosed ctype directive") {
        auto t = "${amp}<?teng ctype 'application/x-sh'?>${amp}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 6},
                    "The closing directive of this <?teng ctype?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 45},
                    "Unexpected token: name=<EOF>, view="
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&amp;&");
            }
        }
    }

    GIVEN("Template with unclosed ctype directive wrapping closed one") {
        auto t = "<?teng ctype 'text/html'?>${amp}"
                 "<?teng ctype 'application/x-sh'?>${amp}<?teng endctype?>"
                 "${amp}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 0},
                    "The closing directive of this <?teng ctype?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 94},
                    "Unexpected token: name=<EOF>, view="
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&amp;&&amp;");
            }
        }
    }

    GIVEN("Template with closed and next unclosed ctype directive") {
        auto t = "${amp}<?teng ctype 'x-sh'?>${amp}<?teng endctype?>"
                 "${amp}<?teng ctype 'text/html'?>${amp}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 56},
                    "The closing directive of this <?teng ctype?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 88},
                    "Unexpected token: name=<EOF>, view="
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&amp;&&amp;&amp;");
            }
        }
    }

    GIVEN("Template with two unclosed ctype blocks") {
        auto t = "${amp}<?teng ctype 'application/x-sh'?>${amp}"
                 "<?teng ctype 'text/html'?>${amp}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 6},
                    "The closing directive of this <?teng ctype?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 45},
                    "The closing directive of this <?teng ctype?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 77},
                    "Unexpected token: name=<EOF>, view="
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&amp;&&amp;");
            }
        }
    }
}

SCENARIO(
    "The invalid open ctype directive",
    "[ctype]"
) {
    GIVEN("Template with open ctype directive without content type") {
        auto t = "#<?teng ctype?>${amp}<?teng endctype?>#";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("The default content type is used") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "The <?teng ctype?> directive must contain content type "
                    "name (e.g. <?teng ctype 'text/html'?>; "
                    "using top content type instead"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 13},
                    "Unexpected token: name=END, view=?>"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "#&amp;#");
            }
        }
    }

    GIVEN("Template with open ctype directive with error before name") {
        auto t = "#<?teng ctype 1 'application/x-sh'?>${amp}<?teng endctype?>#";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("The default content type is used") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid or excessive tokens in <?teng ctype?> directive; "
                    "using top content type instead"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "#&amp;#");
            }
        }
    }

    GIVEN("Template with open ctype directive with error after name") {
        auto t = "#<?teng ctype 'applicaton/x-sh' 1?>  &  <?teng endctype?>#";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("The default content type is used") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid or excessive tokens in <?teng ctype?> directive; "
                    "using top content type instead"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 32},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }
}

SCENARIO(
    "The invalid close ctype directive",
    "[ctype]"
) {
    GIVEN("Template with close ctype directive with excessive tokens") {
        auto t = "#<?teng ctype 'application/x-sh'?>${amp}<?teng endctype 1?>#";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("The space formatting is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 40},
                    "Ignoring invalid excessive tokens in <?teng endctype?> "
                    "directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 56},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "#&#");
            }
        }
    }

    GIVEN("Template with open ctype directive with options") {
        auto t = "#<?teng ctype 'application/x-sh'?>"
                 "${amp}"
                 "<?teng endctype a=1?>#";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("The space formatting is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 56},
                    "This directive doesn't accept any options; ignoring them"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "#&#");
            }
        }
    }
}

SCENARIO(
    "The unopened ctype directive",
    "[ctype]"
) {
    GIVEN("Template with directive closing not opened ctype") {
        auto t = "{<?teng endctype?>}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "The <?teng endctype?> directive closes unopened "
                    "ctype block"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}");
            }
        }
    }

    GIVEN("Template with two directives closing not opened ctype") {
        auto t = "{<?teng endctype?>}{<?teng endctype?>}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "The <?teng endctype?> directive closes unopened "
                    "ctype block"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 20},
                    "The <?teng endctype?> directive closes unopened "
                    "ctype block"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{}{}");
            }
        }
    }

    GIVEN("Template with ctype after directive closing not opened ctype") {
        auto t = "{<?teng endctype?>${amp}}"
                 "{<?teng ctype 'application/x-sh'?>${amp}<?teng endctype?>}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "The <?teng endctype?> directive closes unopened "
                    "ctype block"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{&amp;}{&}");
            }
        }
    }

    GIVEN("Template with ctype before directive closing not opened ctype") {
         auto t = "{<?teng ctype 'application/x-sh'?>${amp}<?teng endctype?>}"
                  "{<?teng endctype?>${amp}}";

        WHEN("Generated with variable contaning dangerous characters") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            root.addVariable("amp", "&");
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 59},
                    "The <?teng endctype?> directive closes unopened "
                    "ctype block"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "{&}{&amp;}");
            }
        }
    }
}

