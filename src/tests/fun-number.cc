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
 * Teng engine -- number functions.
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

SCENARIO(
    "The random function",
    "[fun][number]"
) {
    GIVEN("The template with random(3)") {
        Teng::Fragment_t root;
        auto t = "${random(3)}";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is number up to 3") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(3).epsilon(1));
            }
        }
    }
}

SCENARIO(
    "The number formatting",
    "[fun][number]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("Fewer args than is needed") {
            Teng::Error_t err;
            auto t = "${numformat(3)}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "numformat(): the function expects from 2 to 4 args"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("More args that is needed") {
            Teng::Error_t err;
            auto t = "${numformat(3, 3, 3, 3, 3)}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "numformat(): the function expects from 2 to 4 args"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("Valid args") {
            Teng::Error_t err;
            auto t = "${numformat(123123.456, 1, ',', '.')}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "123.123,5");
            }
        }
    }
}

SCENARIO(
    "The number rounding",
    "[fun][number]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The floating point number with decimal factor less than 0.5") {
            Teng::Error_t err;
            auto t = "${round(3.14, 0)}";
            auto result = g(err, t, root);

            THEN("The result is integral part") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.0");
            }
        }

        WHEN("The floating point number with decimal factor greater than 0.5") {
            Teng::Error_t err;
            auto t = "${round(3.74, 0)}";
            auto result = g(err, t, root);

            THEN("The result is integral part plus one") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "4.0");
            }
        }
    }
}

SCENARIO(
    "The conversion of strings to int",
    "[fun][number]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The floating point number") {
            Teng::Error_t err;
            auto t = "${int(3.14) + 0}";
            auto result = g(err, t, root);

            THEN("The result is a number") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The string containing int only is converted") {
            Teng::Error_t err;
            auto t = "${int('3') + 0}";
            auto result = g(err, t, root);

            THEN("The result is a number") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The string containing int with suffix (disabled errors)") {
            Teng::Error_t err;
            auto t = "${int('3suffix', 1) + 0}";
            auto result = g(err, t, root);

            THEN("The result is a number") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3");
            }
        }

        WHEN("The string containing int with suffix (defaulted errors)") {
            Teng::Error_t err;
            auto t = "${int('3suffix') + 0}";
            auto result = g(err, t, root);

            THEN("The result is a number") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "int(): can't convert string to int"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 17},
                    "Runtime: Left operand of + numeric operator is undefined"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The string containing int with suffix (enabled errors)") {
            Teng::Error_t err;
            auto t = "${int('3suffix', 0) + 0}";
            auto result = g(err, t, root);

            THEN("The result is a number") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "int(): can't convert string to int"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 20},
                    "Runtime: Left operand of + numeric operator is undefined"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The is_number function",
    "[fun][number]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The floating point number") {
            Teng::Error_t err;
            auto t = "${isnumber(3.14)}";
            auto result = g(err, t, root);

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The integral number") {
            Teng::Error_t err;
            auto t = "${isnumber(3)}";
            auto result = g(err, t, root);

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The string") {
            Teng::Error_t err;
            auto t = "${isnumber('s')}";
            auto result = g(err, t, root);

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The regex") {
            Teng::Error_t err;
            auto t = "${isnumber(/s/)}";
            auto result = g(err, t, root);

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}


