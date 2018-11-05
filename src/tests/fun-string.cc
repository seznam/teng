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
 * Teng engine -- string functions.
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
    "The len function",
    "[fun][string]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("empty", "");
        root.addVariable("nonempty", "FGHJ");
        root.addVariable("utf", "čřžýa");

        WHEN("The number") {
            Teng::Error_t err;
            auto t = "${len(3)}";
            auto result = g(err, t, root);

            THEN("The result is 0") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The floating point number") {
            Teng::Error_t err;
            auto t = "${len(3.12345678)}";
            auto result = g(err, t, root);

            THEN("The result is 0") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "8");
            }
        }

        WHEN("The empty string") {
            Teng::Error_t err;
            auto t = "${len('')}";
            auto result = g(err, t, root);

            THEN("The result is 0") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The nonempty string") {
            Teng::Error_t err;
            auto t = "${len('FGHJ')}";
            auto result = g(err, t, root);

            THEN("The result is 4") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "4");
            }
        }

        WHEN("The utf string") {
            Teng::Error_t err;
            auto t = "${len('ščřžý')}";
            auto result = g(err, t, root);

            THEN("The result is 5") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "5");
            }
        }

        WHEN("The empty var") {
            Teng::Error_t err;
            auto t = "${len(.empty)}";
            auto result = g(err, t, root);

            THEN("The result is 0") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The nonempty var") {
            Teng::Error_t err;
            auto t = "${len(.nonempty)}";
            auto result = g(err, t, root);

            THEN("The result is 4") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "4");
            }
        }

        WHEN("The utf var") {
            Teng::Error_t err;
            auto t = "${len(.utf)}";
            auto result = g(err, t, root);

            THEN("The result is 5") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "5");
            }
        }
    }
}

SCENARIO(
    "The nl2br function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The string without new lines") {
            Teng::Error_t err;
            auto t = "%{nl2br('absakfsdjfkjsd')}";
            auto result = g(err, t, root);

            THEN("The result is same string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "absakfsdjfkjsd");
            }
        }

        WHEN("The string with new lines") {
            Teng::Error_t err;
            auto t = "%{nl2br('\nab\nsak\nfsdjd\n')}";
            auto result = g(err, t, root);

            THEN("The new lines are translated to <br>") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<br />\nab<br />\nsak<br />\nfsdjd<br />\n");
            }
        }
    }
}

SCENARIO(
    "The substr function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The empty string is passed") {
            Teng::Error_t err;
            auto t = "${substr('', 0, 10)}";
            auto result = g(err, t, root);

            THEN("The result is empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The non empty string is passed") {
            Teng::Error_t err;
            auto t = "${substr('abc', 0, 10)}";
            auto result = g(err, t, root);

            THEN("The result is substr") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "abc");
            }
        }

        WHEN("The non empty string is passed and 2 chars are requested") {
            Teng::Error_t err;
            auto t = "${substr('abcd', 1, 3)}";
            auto result = g(err, t, root);

            THEN("The result is substr") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "bc");
            }
        }

        WHEN("The utf-8 string is passed and 2 chars are requested") {
            Teng::Error_t err;
            auto t = "${substr('řžýá', 1, 3)}";
            auto result = g(err, t, root);

            THEN("The result is substr") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "žý");
            }
        }

        WHEN("The negative indicies are used") {
            Teng::Error_t err;
            auto t = "${substr('abcd', -3, -1)}";
            auto result = g(err, t, root);

            THEN("The result is substr like in python") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "bc");
            }
        }

    }
}

SCENARIO(
    "The wordsubstr function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The index is at: are m.ore") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 11, 13)}";
            auto result = g(err, t, root);

            THEN("The result is hit word") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "more");
            }
        }

        WHEN("The index is at: are .more") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 10, 13)}";
            auto result = g(err, t, root);

            THEN("The result is hit word") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "more");
            }
        }

        WHEN("The index is at: are. more") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 9, 13)}";
            auto result = g(err, t, root);

            THEN("The result is hit word") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "more");
            }
        }

        WHEN("The index is at: ar.e more") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 8, 13)}";
            auto result = g(err, t, root);

            THEN("The result is two words") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "are more");
            }
        }

        WHEN("The index is at: '.there'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 0, 2)}";
            auto result = g(err, t, root);

            THEN("The first word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "there");
            }
        }

        WHEN("The end index is at: '.  there'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('  there are more words', 0, 1)}";
            auto result = g(err, t, root);

            THEN("The empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The end index is at: ' . there'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('  there are more words', 0, 2)}";
            auto result = g(err, t, root);

            THEN("The empty string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The end index is at: '  .there") {
            Teng::Error_t err;
            auto t = "${wordsubstr('  there are more words', 0, 3)}";
            auto result = g(err, t, root);

            THEN("The first word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "there");
            }
        }

        WHEN("The end index is at: 'words.'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 18, 20)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'word.s'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 18, 19)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'wor.ds'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words', 17, 18)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'words  .'") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words  ', 18, 22)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'words . '") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words  ', 18, 21)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'words.  '") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words  ', 17, 20)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }

        WHEN("The end index is at: 'word.s  '") {
            Teng::Error_t err;
            auto t = "${wordsubstr('there are more words  ', 17, 19)}";
            auto result = g(err, t, root);

            THEN("The last word of string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "words");
            }
        }
    }
}

SCENARIO(
    "The reorder function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The no format args and no used") {
            Teng::Error_t err;
            auto t = "${reorder('longtext')}";
            auto result = g(err, t, root);

            THEN("The result format string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "longtext");
            }
        }

        WHEN("The more format args than used") {
            Teng::Error_t err;
            auto t = "${reorder('longtext', 1)}";
            auto result = g(err, t, root);

            THEN("The result format string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "longtext");
            }
        }

        WHEN("The less format args than used") {
            Teng::Error_t err;
            auto t = "${reorder('lon%{1}gt%{2}ext', 1)}";
            auto result = g(err, t, root);

            THEN("The result format string") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "reorder(): invalid or missing index in format '%{2}'"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "lon1gt%{2}ext");
            }
        }

        WHEN("The complex usage") {
            Teng::Error_t err;
            auto t = "${reorder('some%{1}long%{1}text%{2}', ' ', '!')}";
            auto result = g(err, t, root);

            THEN("The result format string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some long text!");
            }
        }
    }
}

SCENARIO(
    "The replace function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The no pattern found") {
            Teng::Error_t err;
            auto t = "${replace('some text some text', 'nono', 'repl')}";
            auto result = g(err, t, root);

            THEN("The result origin string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "some text some text");
            }
        }

        WHEN("The pattern found two times") {
            Teng::Error_t err;
            auto t = "${replace('some text some text', 'some', 'repl')}";
            auto result = g(err, t, root);

            THEN("The result origin string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "repl text repl text");
            }
        }
    }
}

SCENARIO(
    "The strtolower function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Some utf-8 string") {
            Teng::Error_t err;
            auto t = "${strtolower('aa345678FGHJKžýáiŮŘČÁÉÍÚ')}";
            auto result = g(err, t, root);

            THEN("The result is lowerized string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "aa345678fghjkžýáiůřčáéíú");
            }
        }
    }
}

SCENARIO(
    "The strtoupper function",
    "[fun][string]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Some utf-8 string") {
            Teng::Error_t err;
            auto t = "${strtoupper('aa345678FGHJKžýáiŮŘČÁÉÍÚ')}";
            auto result = g(err, t, root);

            THEN("The result is upperized string") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "AA345678FGHJKŽÝÁIŮŘČÁÉÍÚ");
            }
        }
    }
}


