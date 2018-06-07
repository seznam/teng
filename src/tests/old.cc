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
 * Teng engine -- old tests
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
    "Escaping doubled dolars",
    "[old]"
) {
    GIVEN("fragment with one string variable containing backslash") {
        Teng::Fragment_t data;
        auto &fragment = data.addFragmentList("fff");
        auto &fragment_first = fragment.addFragment();
        fragment_first.addVariable("string", "\"");

        WHEN("The template with directly accessed variable is generated") {
            auto res = g("${$$.fff.string}", data);

            THEN("The backslash is escaped") {
                REQUIRE(res == "&quot;");
            }
        }

        WHEN("The template with the variable is generated") {
            auto res = g("<?teng frag fff ?>${string}<?teng endfrag ?>", data);

            THEN("The backslash is escaped") {
                REQUIRE(res == "&quot;");
            }
        }
    }
}

SCENARIO(
    "Teng function exist",
    "[old]"
) {
    GIVEN("Teng data with one fragment and some variables") {
        Teng::Fragment_t data;
        auto &fragment = data.addFragment("fff");
        fragment.addVariable("empty_string", "");
        fragment.addVariable("nonempty_string", "abc");
        fragment.addVariable("nonzero_number", 1.9);
        fragment.addVariable("zero_number", 0.0);

        WHEN("Existence is queried for existing variables") {
            THEN("The exist function should return true") {
                REQUIRE(
                    g(
                        "<?teng frag fff ?>"
                        "${exist(empty_string)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "1"
                );
                REQUIRE(
                    g(
                        "<?teng frag fff ?>"
                        "${exist(nonempty_string)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "1"
                );
                REQUIRE(
                    g(
                        "${exist($$fff.empty_string)}",
                        data
                    ) == "1"
                );
                REQUIRE(
                    g(
                        "${exist($$fff.nonempty_string)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for existing fragments") {
            THEN("The exist function should return true") {
                REQUIRE(
                    g(
                        "${exist($$fff)}",
                        data
                    ) == "1"
                );
                REQUIRE(
                    g(
                        "${exist(fff)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for non-existing variables") {
            THEN("The exist function should return true") {
                REQUIRE(
                    g(
                        "<?teng frag fff ?>"
                        "${exist(abc)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "0"
                );
                REQUIRE(
                    g(
                        "${exist($$fff.abc)}",
                        data
                    ) == "0"
                );
            }
        }

        WHEN("Existence is queried for non-existing fragments") {
            THEN("The exist function should return true") {
                REQUIRE(
                    g(
                        "${exist($$ggg)}",
                        data
                    ) == "0"
                );
                REQUIRE(
                    g(
                        "${exist(ggg)}",
                        data
                    ) == "0"
                );
            }
        }
    }
}

SCENARIO(
    "Basic html escaping",
    "[old]"
) {
    GIVEN("The div tag") {
        std::string templ = "${escape(\"<div>\")}";

        WHEN("Is escaped") {
            THEN("Dangerous characters are escaped") {
                REQUIRE(g(templ) == "&lt;div&gt;");
            }
        }
    }
}

SCENARIO(
    "AlwaysEscape",
    "[old]"
) {
    GIVEN("Mysterious template") {
        std::string templ = "<?teng set $.__q = '\"kaktus&<>\"'?>"
                            "<?teng set $.__r = $.__q?>"
                            "<?teng set $.__s = $.__r?>"
                            "<?teng set $.__t = $.__s?>"
                            "${.__t}";

        WHEN("Is escaped") {
            THEN("Dangerous characters are escaped") {
                std::string result = "&amp;amp;amp;quot;kaktus&amp;amp;amp;amp;"
                                     "&amp;amp;amp;lt;&amp;amp;amp;gt;&amp;amp;"
                                     "amp;quot;";
                REQUIRE(g(templ) == result);
            }
        }
    }
}

SCENARIO(
    "Basic int conversion",
    "[old]"
) {
    WHEN("Floating point number is converted to int") {
        THEN("The result is integer part") {
            REQUIRE(g("${int(22.567)}") == "22");
        }
    }
    WHEN("Inplace created floating point number is converted to int") {
        THEN("The result is integer part") {
            REQUIRE(g("${int(\"12\"++\"3.5\")}") == "123");
        }
    }
    WHEN("Floating point number with invalid suffix is converted to int") {
        THEN("The result is undefined") {
            REQUIRE(g("${int(\"12.6er\")}") == "undefined");
        }
    }
    WHEN("The integer with suffix (pixel units)") {
        THEN("The result is the integer") {
            REQUIRE(g("${int(\"128px\",1)}") == "128");
        }
    }
}

SCENARIO(
    "Querying for number",
    "[old]"
) {
    WHEN("The integer is given to isnumber") {
        THEN("The result is true") {
            REQUIRE(g("${isnumber(123)}") == "1");
        }
    }
    WHEN("The string is given to isnumber") {
        THEN("The result is false") {
            REQUIRE(g("${isnumber(\"123\")}") == "0");
        }
    }
}

SCENARIO(
    "String replacing",
    "[old]"
) {
    WHEN("'jede' is replace with 'leze'") {
        std::string templ = "${replace(\"jede jede šnek\",\"jede\",\"leze\")}";
        THEN("'leze' is replaced") {
            REQUIRE(g(templ) == "leze leze šnek");
        }
    }
}

SCENARIO(
    "Floating number rounding",
    "[old]"
) {
    WHEN("The floating number is rounded with precision set to 2") {
        THEN("The result has two fractional digits") {
            REQUIRE(g("${round(123.1245,2)}") == "123.12");
        }
    }
    WHEN("The floating number is rounded with precision set to 3") {
        THEN("The result has three fractional digits") {
            REQUIRE(g("${round(123.1245,3)}") == "123.125");
        }
    }
}

SCENARIO(
    "Converting seconds to time",
    "[old]"
) {
    WHEN("Converting zero seconds to time") {
        THEN("The zero time is returned") {
            REQUIRE(g("${sectotime(0)}") == "0:00:00");
        }
    }
    WHEN("Converting some amount of seconds to time") {
        THEN("The result is some time") {
            REQUIRE(g("${sectotime(7425)}") == "2:03:45");
        }
    }
}

SCENARIO(
    "Getting substring of string",
    "[old]"
) {
    WHEN("Substring from some index to end is requested") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"Dlouhý text\",7)}") == "text");
        }
    }
    WHEN("Substring from some index to end is requested with prefix") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"Dlouhý text\",7,\":\")}") == ":text");
        }
    }
    WHEN("Substring from some index to end is requested with prefix & suffix") {
        THEN("It is returned without suffix") {
            // prefix resp suffix are used only if left resp right end of the
            // substring is not begin resp end of the original string
            REQUIRE(g("${substr(\"Dlouhý text\",7,\":\",\";\")}") == ":text");
        }
    }
    WHEN("Substring from some index to some end") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"Dlouhý text\",5,8)}") == "ý t");
        }
    }
    WHEN("Substring from some index to some end with prefix & auto suffix") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"Dlouhý text\",5,8,\":\")}") == ":ý t:");
        }
    }
    WHEN("Substring from some index to some end with prefix & expl suffix") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"Dlouhý text\",5,8,\":\",\";\")}") == ":ý t;");
        }
    }
    WHEN("Substring from start to end with prefix & expl suffix") {
        THEN("It is returned") {
            REQUIRE(g("${substr(\"ý t\",0,3,\":\",\";\")}") == "ý t");
        }
    }
}

SCENARIO(
    "Html unescape",
    "[old]"
) {
    WHEN("String with html entities is unescaped") {
        THEN("The entities are expanded") {
            REQUIRE(g("${unescape(\"&lt;b&gt;č&lt;/b&gt;\")}") == "<b>č</b>");
        }
    }
}

SCENARIO(
    "PCRE regex based string replacing",
    "[old]"
) {
    WHEN("Replacing words") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"foo bar\", \"\\\\w+\", \"($0)\")}";
            REQUIRE(g(t) == "(foo) (bar)");
        }
    }
    WHEN("Replacing words") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"foo <b>ar\", \"<[^>]*>\", \"\")}";
            REQUIRE(g(t) == "foo ar");
        }
    }
    WHEN("Replacing words") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"velmivelkéslovo\", "
                     "\"([^\\\\s]{5})\", \"$1 \")}";
            REQUIRE(g(t) == "velmi velké slovo ");
        }
    }
    WHEN("Replacing words") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"velmivelkéslovo\", "
                     "\"([^\\\\s]{6})\", \"$1 \")}";
            REQUIRE(g(t) == "velmiv elkésl ovo");
        }
    }
    WHEN("Replacing words") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"velmivelkéslovo\", "
                     "\"([^\\\\s]{4})\", \"$1 \")}";
            REQUIRE(g(t) == "velm ivel késl ovo");
        }
    }
    WHEN("Interleaving characters") {
        THEN("They are replaced") {
            auto t = "${regex_replace(\"ééé\", \"([^\\\\s]{1})\", \"$1 \")}";
            REQUIRE(g(t) == "é é é ");
        }
    }
}

SCENARIO(
    "Converting new lines to br tags",
    "[old]"
) {
    WHEN("String with new lines is passed to nl2br") {
        THEN("The new lines are replaced with br tags") {
            REQUIRE(g("${nl2br(\"jede\\nmašina\")}") == "jede\n<br />mašina");
        }
    }
}

SCENARIO(
    "Numbers formatting",
    "[old]"
) {
    WHEN("Formating floating number") {
        THEN("Is formatted") {
            REQUIRE(g("${numformat(1230.45666,3,\".\",\",\")}") == "1,230.457");
        }
    }
    WHEN("Formating integer number") {
        THEN("Is formatted") {
            REQUIRE(g("${numformat(12304566,0,\".\",\" \")}") == "12 304 566");
        }
    }
}

SCENARIO(
    "Arguments reordering to string",
    "[old]"
) {
    WHEN("Arguments are reordered") {
        THEN("They are reordered") {
            REQUIRE(g("${reorder(\"%1,%2;%1\",\"c\",\"d\")}") == "c,d;c");
        }
    }
}

SCENARIO(
    "String concating",
    "[old]"
) {
    WHEN("Strings are concated") {
        THEN("They are concated") {
            auto t = "<?teng set .variable = \"a\" ?>"
                     "<?teng set .variable = $.variable ++ \"b\" ?>"
                     "${.variable}";
            REQUIRE(g(t) == "ab");
        }
    }
}

SCENARIO(
    "Converting string to uppercase",
    "[old]"
) {
    WHEN("Coverting ascii text to uppercase") {
        THEN("All lowervase letters are converted") {
            REQUIRE(g("${strtoupper(\"abc ABC, 123\")}") == "ABC ABC, 123");
        }
    }
    WHEN("Coverting czech utf-8 text to uppercase") {
        THEN("All lowervase letters are converted") {
            REQUIRE(g("${strtoupper(\"ěščřžýáíéůúóťň\")}") == "ĚŠČŘŽÝÁÍÉŮÚÓŤŇ");
        }
    }
}

SCENARIO(
    "Converting string to lowercase",
    "[old]"
) {
    WHEN("Coverting ascii text to lowercase") {
        THEN("All uppercase letters are converted") {
            REQUIRE(g("${strtolower(\"abc ABC, 123\")}") == "abc abc, 123");
        }
    }
    WHEN("Coverting czech utf-8 text to lowercase") {
        THEN("All uppercase letters are converted") {
            REQUIRE(g("${strtolower(\"ĚŠČŘŽÝÁÍÉŮÚÓŤŇ\")}") == "ěščřžýáíéůúóťň");
        }
    }
}

