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
 * Teng engine -- dict related tests.
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
    "Regular dict lookups use raw print as default",
    "[dict][expr]"
) {
    GIVEN("Dictionary (see dict.txt) and variables with dict keys") {
        Teng::Fragment_t root;
        root.addVariable("a", "html_value");
        auto r = "&amp;&lt;b&gt;some "
                 "&lt;i&gt;HTML&lt;/i&gt; text&lt;/b&gt;&amp;";

        WHEN("Regular lookup of the existing key") {
            Teng::Error_t err;
            auto t = "#{html_value}";
            auto result = g(err, t, root);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }

        WHEN("Dict lookup of the existing key - raw print used") {
            Teng::Error_t err;
            auto t = "%{#html_value}";
            auto result = g(err, t, root);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }

        WHEN("Dict lookup of the existing key - escaping print used") {
            Teng::Error_t err;
            auto t = "${#html_value}";
            auto result = g(err, t, root);

            THEN("Replaced with escaped dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == r);
            }
        }

        WHEN("Variable lookup of the existing key - raw print used") {
            Teng::Error_t err;
            auto t = "%{@a}";
            auto result = g(err, t, root);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }

        WHEN("Variable lookup of the existing key - escaping print used") {
            Teng::Error_t err;
            auto t = "${@a}";
            auto result = g(err, t, root);

            THEN("Replaced with escaped dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == r);
            }
        }
    }
}

SCENARIO(
    "Regular dict lookups with print escaping disabled",
    "[dict][expr]"
) {
    GIVEN("Dictionary (see dict.txt) and variables with dict keys") {
        Teng::Fragment_t root;
        root.addVariable("a", "html_value");
        const char * conf = "teng.no-print-escape.conf";

        WHEN("Regular lookup of the existing key") {
            Teng::Error_t err;
            auto t = "#{html_value}";
            auto result = g(err, t, root, "", "text/html", "utf-8", conf);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }

        WHEN("Dict lookup of the existing key - raw print used") {
            Teng::Error_t err;
            auto t = "%{#html_value}";
            auto result = g(err, t, root, "", "text/html", "utf-8", conf);

            THEN("Raw print directive is printed as regular text") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "%{#html_value}");
            }
        }

        WHEN("Dict lookup of the existing key - escaping print used") {
            Teng::Error_t err;
            auto t = "${#html_value}";
            auto result = g(err, t, root, "", "text/html", "utf-8", conf);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }

        WHEN("Variable lookup of the existing key - raw print used") {
            Teng::Error_t err;
            auto t = "%{@a}";
            auto result = g(err, t, root, "", "text/html", "utf-8", conf);

            THEN("Raw print directive is printed as regular text") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "%{@a}");
            }
        }

        WHEN("Variable lookup of the existing key - escaping print used") {
            Teng::Error_t err;
            auto t = "${@a}";
            auto result = g(err, t, root, "", "text/html", "utf-8", conf);

            THEN("Replaced with raw dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "&<b>some <i>HTML</i> text</b>&");
            }
        }
    }
}

SCENARIO(
    "Dict lookup when language is not specified",
    "[dict][expr]"
) {
    GIVEN("Dictionary (see dict.txt) and variables with dict keys") {
        Teng::Fragment_t root;
        root.addVariable("a", "hello_world");
        root.addVariable("b", "hello_world_missing");

        WHEN("Regular lookup of the existing key") {
            Teng::Error_t err;
            auto t = "#{hello_world}";
            auto result = g(err, t, root);

            THEN("Replaced with dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello world");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${#hello_world}";
            auto result = g(err, t, root);

            THEN("Replaced with dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello world");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${#hello_world_missing}";
            auto result = g(err, t, root);

            THEN("Replaced with dict entry name") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Dictionary item 'hello_world_missing' was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${@a}";
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello world");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${@b}";
            auto result = g(err, t, root);

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Dictionary item 'hello_world_missing' "
                    "was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }
    }
}

SCENARIO(
    "Dict lookup when language is specified",
    "[dict][expr]"
) {
    GIVEN("English dictionary (see dict.en.txt) and variables with dict keys") {
        Teng::Fragment_t root;
        root.addVariable("a", "hello_world");
        root.addVariable("b", "hello_world_missing");

        WHEN("Regular lookup of the existing key") {
            Teng::Error_t err;
            auto t = "#{hello_world}";
            auto result = g(err, t, root, "en");

            THEN("Replaced with dict entry value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Hello world!");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${#hello_world}";
            auto result = g(err, t, root, "en");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Hello world!");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${#hello_world_missing}";
            auto result = g(err, t, root, "en");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Dictionary item 'hello_world_missing' was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${@a}";
            auto result = g(err, t, root, "en");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Hello world!");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${@b}";
            auto result = g(err, t, root, "en");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Dictionary item 'hello_world_missing' "
                    "was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }
    }

    GIVEN("Czech dictionary (see dict.cs.txt) and variables with dict keys") {
        Teng::Fragment_t root;
        root.addVariable("a", "hello_world");
        root.addVariable("b", "hello_world_missing");

        WHEN("Regular lookup of the existing key") {
            Teng::Error_t err;
            auto t = "#{hello_world}";
            auto result = g(err, t, root, "cs");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Ahoj svete!");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${#hello_world}";
            auto result = g(err, t, root, "cs");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Ahoj svete!");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${#hello_world_missing}";
            auto result = g(err, t, root, "cs");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Dictionary item 'hello_world_missing' was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }

        WHEN("Lookup of the existing key") {
            Teng::Error_t err;
            auto t = "${@a}";
            auto result = g(err, t, root, "cs");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "Ahoj svete!");
            }
        }

        WHEN("Lookup of the missing key") {
            Teng::Error_t err;
            auto t = "${@b}";
            auto result = g(err, t, root, "cs");

            THEN("It is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Dictionary item 'hello_world_missing' "
                    "was not found"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "hello_world_missing");
            }
        }
    }
}


