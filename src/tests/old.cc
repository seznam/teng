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
    "Escaping quote",
    "[old]"
) {
    GIVEN("Fragment with one string variable containing quote") {
        Teng::Fragment_t data;
        auto &fragment = data.addFragmentList("fff");
        auto &fragment_first = fragment.addFragment();
        fragment_first.addVariable("string", "\"");

        WHEN("The template with directly accessed variable is generated") {
            Teng::Error_t err;
            auto res = g(err, "${$$.fff.string}", data);

            THEN("The double quote is escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "&quot;");
            }
        }

        WHEN("The template with the variable is generated") {
            Teng::Error_t err;
            auto res = g(
                err,
                "<?teng frag fff ?>${string}<?teng endfrag ?>",
                data
            );

            THEN("The doublequote is escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "&quot;");
            }
        }
    }
}

SCENARIO(
    "Teng function exists",
    "[old]"
) {
    GIVEN("Teng data with one fragment and some variables") {
        Teng::Fragment_t data;
        auto &fragment = data.addFragment("fff");
        fragment.addVariable("empty_string", "");
        fragment.addVariable("nonempty_string", "abc");
        fragment.addVariable("nonzero_number", 1.9);
        fragment.addVariable("zero_number", 0.0);

        WHEN("Existence is queried for empty_string variable") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        err,
                        "<?teng frag fff ?>"
                        "${exists(empty_string)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "1"
                );
            }
        }


        WHEN("Existence is queried for nonempty_string variable") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        err,
                        "<?teng frag fff ?>"
                        "${exists(nonempty_string)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for fff.empty_string variable") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        err,
                        "${exists($$fff.empty_string)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for fff.nonempty_string variable") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        err,
                        "${exists($$fff.nonempty_string)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried (abs) for existing fragments") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "${exists($$fff)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for existing fragments") {
            Teng::Error_t err;
            THEN("The exists function should return true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "${exists(fff)}",
                        data
                    ) == "1"
                );
            }
        }

        WHEN("Existence is queried for abc variables") {
            Teng::Error_t err;
            THEN("The exists function should return false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "<?teng frag fff ?>"
                        "${exists(abc)}"
                        "<?teng endfrag ?>",
                        data
                    ) == "0"
                );
            }
        }

        WHEN("Existence is queried for fff.abc variables") {
            Teng::Error_t err;
            THEN("The exists function should return false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "${exists($$fff.abc)}",
                        data
                    ) == "0"
                );
            }
        }

        WHEN("Existence is queried for non-existing fragments") {
            Teng::Error_t err;
            THEN("The exists function should return false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "${exists($$ggg)}",
                        data
                    ) == "0"
                );
            }
        }

        WHEN("Existence is queried for non-existing fragments") {
            Teng::Error_t err;
            THEN("The exists function should return false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(
                    g(
                        "${exists(ggg)}",
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
        std::string templ = "${escape('<div>')}";

        WHEN("Is escaped") {
            Teng::Error_t err;
            auto res = g(err, templ);
            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(res == "&lt;div&gt;");
            }
        }
    }
}

SCENARIO(
    "AlwaysEscape",
    "[old]"
) {
    // TODO(burlog): is that wanted behaviour?
    // each assigning does escaping

    GIVEN("Mysterious template") {
        std::string templ = "<?teng set $.q = '\"kaktus&<>\"'?>"
                            "<?teng set $.r = $.q?>"
                            "<?teng set $.s = $.r?>"
                            "<?teng set $.t = $.s?>"
                            "${.t}";

        WHEN("Is escaped") {
            Teng::Error_t err;
            auto res = g(err, templ);
            THEN("Dangerous characters are escaped") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 11},
                    "Don't use dollar sign here please"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 43},
                    "Don't use dollar sign here please"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 65},
                    "Don't use dollar sign here please"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 87},
                    "Don't use dollar sign here please"
                }};
                REQUIRE(err.getEntries() == errs);
                std::string result = "&amp;amp;amp;quot;kaktus&amp;amp;amp;amp;"
                                     "&amp;amp;amp;lt;&amp;amp;amp;gt;&amp;amp;"
                                     "amp;quot;";
                REQUIRE(res == result);
            }
        }
    }
}

SCENARIO(
    "Basic int conversion",
    "[old]"
) {
    WHEN("Floating point number is converted to int") {
        Teng::Error_t err;
        auto templ = "${int(22.567)}";
        auto res = g(err, templ);
        THEN("The result is integer part") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "22");
        }
    }
    WHEN("Inplace created floating point number is converted to int") {
        Teng::Error_t err;
        auto templ = "${int('12'++'3.5')}";
        auto res = g(err, templ);
        THEN("The result is integer part") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "123");
        }
    }
    WHEN("Floating point number with invalid suffix is converted to int") {
        Teng::Error_t err;
        auto templ = "${int('12.6er')}";
        auto res = g(err, templ);
        THEN("The result is undefined") {
            std::vector<Teng::Error_t::Entry_t> errs = {{
                Teng::Error_t::ERROR,
                {1, 2},
                "int(): can't convert string to int"
            }};
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "undefined");
        }
    }
    WHEN("The integer with suffix (pixel units)") {
        Teng::Error_t err;
        auto templ = "${int('128px',1)}";
        auto res = g(err, templ);
        THEN("The result is the integer") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "128");
        }
    }
}

SCENARIO(
    "Querying for number",
    "[old]"
) {
    WHEN("The integer is given to isnumber") {
        Teng::Error_t err;
        auto templ = "${isnumber(123)}";
        auto res = g(err, templ);
        THEN("The result is true") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "1");
        }
    }
    WHEN("The string is given to isnumber") {
        Teng::Error_t err;
        auto templ = "${isnumber('123')}";
        auto res = g(err, templ);
        THEN("The result is false") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "0");
        }
    }
}

SCENARIO(
    "String replacing",
    "[old]"
) {
    WHEN("'jede' is replace with 'leze'") {
        Teng::Error_t err;
        std::string templ = "${replace('jede jede šnek','jede','leze')}";
        auto res = g(err, templ);
        THEN("'leze' is replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(g(templ) == "leze leze šnek");
        }
    }
}

SCENARIO(
    "Floating number rounding",
    "[old]"
) {
    WHEN("The floating number is rounded with precision set to 2") {
        Teng::Error_t err;
        auto templ = "${round(123.1245,2)}";
        auto res = g(err, templ);
        THEN("The result has two fractional digits") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "123.12");
        }
    }
    WHEN("The floating number is rounded with precision set to 3") {
        Teng::Error_t err;
        auto templ = "${round(123.1245,3)}";
        auto res = g(err, templ);
        THEN("The result has three fractional digits") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "123.125");
        }
    }
}

SCENARIO(
    "Converting seconds to time",
    "[old]"
) {
    WHEN("Converting zero seconds to time") {
        Teng::Error_t err;
        auto templ = "${sectotime(0)}";
        auto res = g(err, templ);
        THEN("The zero time is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "0:00:00");
        }
    }
    WHEN("Converting some amount of seconds to time") {
        Teng::Error_t err;
        auto templ = "${sectotime(7425)}";
        auto res = g(err, templ);
        THEN("The result is some time") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "2:03:45");
        }
    }
}

SCENARIO(
    "Getting substring of string",
    "[old]"
) {
    WHEN("Substring from some index to end is requested") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',7)}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "text");
        }
    }
    WHEN("Substring from some index to end is requested with prefix") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',7,':')}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == ":text");
        }
    }
    WHEN("Substring from some index to end is requested with prefix & suffix") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',7,':',';')}";
        auto res = g(err, templ);
        THEN("It is returned without suffix") {
            // prefix resp suffix are used only if left resp right end of the
            // substring is not begin resp end of the original string
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == ":text");
        }
    }
    WHEN("Substring from some index to some end") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',5,8)}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ý t");
        }
    }
    WHEN("Substring from some index to some end with prefix & auto suffix") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',5,8,':')}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == ":ý t:");
        }
    }
    WHEN("Substring from some index to some end with prefix & expl suffix") {
        Teng::Error_t err;
        auto templ = "${substr('Dlouhý text',5,8,':',';')}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == ":ý t;");
        }
    }
    WHEN("Substring from start to end with prefix & expl suffix") {
        Teng::Error_t err;
        auto templ = "${substr('ý t',0,3,':',';')}";
        auto res = g(err, templ);
        THEN("It is returned") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ý t");
        }
    }
}

SCENARIO(
    "Html unescape",
    "[old]"
) {
    WHEN("String with html entities is unescaped") {
        Teng::Error_t err;
        auto templ = "${unescape('&lt;b&gt;č&lt;/b&gt;')}";
        auto res = g(err, templ);
        THEN("The entities are expanded") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "<b>č</b>");
        }
    }
}

SCENARIO(
    "PCRE regex based string replacing",
    "[old]"
) {
    WHEN("Replacing words") {
        Teng::Error_t err;
        auto templ = "${regex_replace('foo bar', '\\\\w+', '($0)')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "(foo) (bar)");
        }
    }
    WHEN("Replacing words") {
        Teng::Error_t err;
        auto templ = "${regex_replace('foo <b>ar', '<[^>]*>', '')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "foo ar");
        }
    }
    WHEN("Replacing words") {
        Teng::Error_t err;
        auto templ = "${regex_replace('velmivelkéslovo', "
                     "'([^\\\\s]{5})', '$1 ')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "velmi velké slovo ");
        }
    }
    WHEN("Replacing words") {
        Teng::Error_t err;
        auto templ = "${regex_replace('velmivelkéslovo', "
                     "'([^\\\\s]{6})', '$1 ')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "velmiv elkésl ovo");
        }
    }
    WHEN("Replacing words") {
        Teng::Error_t err;
        auto templ = "${regex_replace('velmivelkéslovo', "
                     "'([^\\\\s]{4})', '$1 ')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "velm ivel késl ovo");
        }
    }
    WHEN("Interleaving characters") {
        Teng::Error_t err;
        auto templ = "${regex_replace('ééé', '([^\\\\s]{1})', '$1 ')}";
        auto res = g(err, templ);
        THEN("They are replaced") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "é é é ");
        }
    }
}

SCENARIO(
    "Converting new lines to br tags",
    "[old]"
) {
    WHEN("String with new lines is passed to nl2br") {
        Teng::Error_t err;
        auto templ = "${nl2br('jede\\nmašina')}";
        auto res = g(err, templ);
        THEN("The new lines are replaced with br tags") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "jede<br />\nmašina");
        }
    }
}

SCENARIO(
    "Numbers formatting",
    "[old]"
) {
    WHEN("Formating floating number") {
        Teng::Error_t err;
        auto templ = "${numformat(1230.45666,3,'.',',')}";
        auto res = g(err, templ);
        THEN("Is formatted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "1,230.457");
        }
    }
    WHEN("Formating integer number") {
        Teng::Error_t err;
        auto templ = "${numformat(12304566,0,'.',' ')}";
        auto res = g(err, templ);
        THEN("Is formatted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "12 304 566");
        }
    }
}

SCENARIO(
    "Arguments reordering to string",
    "[old]"
) {
    WHEN("Arguments are reordered") {
        Teng::Error_t err;
        auto templ = "${reorder('%1,%2;%1','c','d')}";
        auto res = g(err, templ);
        THEN("They are reordered") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "c,d;c");
        }
    }
}

SCENARIO(
    "String setting and concating",
    "[old]"
) {
    WHEN("Concated string is set to variable") {
        Teng::Error_t err;
        auto templ = "<?teng set .variable = 'a' ?>"
                     "<?teng set .variable = $.variable ++ 'b' ?>"
                     "${.variable}";
        auto res = g(err, templ);
        THEN("The variable contains the right value") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ab");
        }
    }
}

SCENARIO(
    "Converting string to uppercase",
    "[old]"
) {
    WHEN("Coverting ascii text to uppercase") {
        Teng::Error_t err;
        auto templ = "${strtoupper('abc ABC, 123')}";
        auto res = g(err, templ);
        THEN("All lowervase letters are converted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ABC ABC, 123");
        }
    }
    WHEN("Coverting czech utf-8 text to uppercase") {
        Teng::Error_t err;
        auto templ = "${strtoupper(\"ěščřžýáíéůúóťň\")}";
        auto res = g(err, templ);
        THEN("All lowervase letters are converted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ĚŠČŘŽÝÁÍÉŮÚÓŤŇ");
        }
    }
}

SCENARIO(
    "Converting string to lowercase",
    "[old]"
) {
    WHEN("Coverting ascii text to lowercase") {
        Teng::Error_t err;
        auto templ = "${strtolower('abc ABC, 123')}";
        auto res = g(err, templ);
        THEN("All uppercase letters are converted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "abc abc, 123");
        }
    }
    WHEN("Coverting czech utf-8 text to lowercase") {
        Teng::Error_t err;
        auto templ = "${strtolower('ĚŠČŘŽÝÁÍÉŮÚÓŤŇ')}";
        auto res = g(err, templ);
        THEN("All uppercase letters are converted") {
            std::vector<Teng::Error_t::Entry_t> errs;
            REQUIRE(err.getEntries() == errs);
            REQUIRE(res == "ěščřžýáíéůúóťň");
        }
    }
}

// SCENARIO(
//     "SANDBOX",
//     "[sandbox]"
// ) {
//     WHEN("String with html entities is unescaped") {
//         Teng::Error_t err;
//         auto templ = "${1#<{(|1}ahoj";
//         auto res = g(err, templ);
//         THEN("The entities are expanded") {
//             std::vector<Teng::Error_t::Entry_t> errs;
//             CHECK(err.getEntries() == errs);
//             CHECK(res == "<b>č</b>");
//         }
//     }
// }

