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
 * Teng engine -- test of url encoding teng functions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Coverted to catch2.
 */

#include <teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "Escaping string with urlencoding",
    "[fun][escaping]"
) {
    WHEN("String with special characters - escaping print used") {
        THEN("Some of them are escaped") {
            auto t = "%{urlescape(\"'asdf!@#$%^&*\(\\\"\")}";
            REQUIRE(g(t) == "'asdf!@%23$%^&*(%22");
        }
    }

    WHEN("String with special characters - raw print used") {
        THEN("Some of them are escaped") {
            auto t = "${urlescape(\"'asdf!@#$%^&*\(\\\"\")}";
            REQUIRE(g(t) == "'asdf!@%23$%^&amp;*(%22");
        }
    }
}

SCENARIO(
    "Unescaping urlencoding",
    "[fun][escaping]"
) {
    WHEN("Urlescaped string - raw print used") {
        THEN("Urlescape sequencies are decoded") {
            auto t = "%{urlunescape(\"%27asdf%21%40%23%24%25%5E%26%2A%28\")}";
            REQUIRE(g(t) == "'asdf!@#$%^&*(");
        }
    }

    WHEN("Urlescaped string - escaping print used") {
        THEN("Urlescape sequencies are decoded") {
            auto t = "${urlunescape(\"%27asdf%21%40%23%24%25%5E%26%2A%28\")}";
            REQUIRE(g(t) == "'asdf!@#$%^&amp;*(");
        }
    }
}

SCENARIO(
    "The escape function",
    "[fun][escaping]"
) {
    GIVEN("The default html content type") {
        Teng::Fragment_t root;
        auto t = "%{escape('some<b>text&\\'\"')}";

        WHEN("The escape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some&lt;b&gt;text&amp;'&quot;");
            }
        }
    }

    GIVEN("The quoted string content type") {
        Teng::Fragment_t root;
        auto t = "<?teng ctype 'quoted-string'?>"
                 "%{escape('some<b>#text&\\'\"')}"
                 "<?teng endctype?>";

        WHEN("The escape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>#text&\\'\\\"");
            }
        }
    }
}

SCENARIO(
    "The unescape function",
    "[fun][escaping]"
) {
    GIVEN("The default html content type") {
        Teng::Fragment_t root;
        auto t = "%{unescape('some&lt;b&gt;text&amp;\\'&quot;')}";

        WHEN("The unescape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are unescaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>text&'\"");
            }
        }
    }

    GIVEN("The quoted string content type") {
        Teng::Fragment_t root;
        auto t = "<?teng ctype 'quoted-string'?>"
                 "%{unescape('some<b>#text&\\'\\\"')}"
                 "<?teng endctype?>";

        WHEN("The unescape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are unescaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>#text&'\"");
            }
        }
    }
}

SCENARIO(
    "The quoteescape function",
    "[fun][escaping]"
) {
    GIVEN("The default html content type") {
        Teng::Fragment_t root;
        auto t = "%{quoteescape('some<b>text&\\'\"\n')}";

        WHEN("The quoteescape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("The 'quoted string' characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>text&\\'\\\"\\n");
            }
        }
    }

    GIVEN("The quoted string content type") {
        Teng::Fragment_t root;
        auto t = "<?teng ctype 'x-sh'?>"
                 "%{quoteescape('some<b>text&\\'\"\n')}"
                 "<?teng endctype?>";

        WHEN("The escape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("The 'quoted string' characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>text&\\'\\\"\\n");
            }
        }
    }
}

SCENARIO(
    "The print escaping is disabled",
    "[fun][escaping]"
) {
    GIVEN("The default html content type - raw print used") {
        Teng::Fragment_t root;
        auto t = "%{unescape('s&lt;b&gt;t&amp;\\'&quot;')}";

        WHEN("The unescape is called") {
            Teng::Error_t err;
            const char *params = "teng.no-print-escape.conf";
            auto result = g(err, t, root, params);

            THEN("%{} directives are handled as regular text") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "%{unescape('s&lt;b&gt;t&amp;\\'&quot;')}");
            }
        }
    }

    GIVEN("The quoted string content type - raw print used") {
        Teng::Fragment_t root;
        auto t = "<?teng ctype 'quoted-string'?>"
                 "%{unescape('some<b>#text&\\'\\\"')}"
                 "<?teng endctype?>";

        WHEN("The unescape is called") {
            Teng::Error_t err;
            const char *params = "teng.no-print-escape.conf";
            auto result = g(err, t, root, params);

            THEN("%{} directives are handled as regular text") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "%{unescape('some<b>#text&\\'\\\"')}");
            }
        }
    }

    GIVEN("The default html content type - escaping print used") {
        Teng::Fragment_t root;
        auto t = "%{escape('some<b>text&\\'\"')}";

        WHEN("The escape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some&lt;b&gt;text&amp;'&quot;");
            }
        }
    }

    GIVEN("The quoted string content type - escaping print used") {
        Teng::Fragment_t root;
        auto t = "<?teng ctype 'quoted-string'?>"
                 "%{escape('some<b>#text&\\'\"')}"
                 "<?teng endctype?>";

        WHEN("The escape is called") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some<b>#text&\\'\\\"");
            }
        }
    }
}

