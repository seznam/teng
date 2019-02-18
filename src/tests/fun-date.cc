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
 * Teng engine -- date functions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <ctime>
#include <iomanip>
#include <sstream>
#include <teng/teng.h>
#include <unistd.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The now function",
    "[fun][date]"
) {
    GIVEN("The template with now()") {
        Teng::Fragment_t root;
        auto t = "${now()}";

        WHEN("The template is rendered") {
            Teng::Error_t err;
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is current timestamp") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(time(nullptr)).epsilon(0.01));
            }
        }
    }
}

SCENARIO(
    "The timestamp function",
    "[fun][date]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The date with year only") {
            Teng::Error_t err;
            auto t = "${timestamp('2018')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "parseDateTime(): invalid format of month; "
                    "use YYYY-MM-DD[ HH:MM:SS[+ZHZM]] format"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "timestamp(): Can't parse date"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(0).epsilon(0.01));
            }
        }

        WHEN("The date with year, month") {
            Teng::Error_t err;
            auto t = "${timestamp('2018-03')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "parseDateTime(): invalid format of day; "
                    "use YYYY-MM-DD[ HH:MM:SS[+ZHZM]] format"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "timestamp(): Can't parse date"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(0).epsilon(0.01));
            }
        }

        WHEN("The date with year, month, day") {
            Teng::Error_t err;
            auto t = "${timestamp('2018-03-03')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is timestamp") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(1520035200.0).epsilon(0.01));
            }
        }

        WHEN("The date with year, month, day, hour") {
            Teng::Error_t err;
            auto t = "${timestamp('2018-03-03 12')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "parseDateTime(): expected ':' as hour/minute separator; "
                    "use YYYY-MM-DD[ HH:MM:SS[+ZHZM]] format"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "timestamp(): Can't parse date"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(0.0).epsilon(0.01));
            }
        }

        WHEN("The date with year, month, day, hour, minute") {
            Teng::Error_t err;
            auto t = "${timestamp('2018-03-03 12:03')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "parseDateTime(): expected ':' as minute/second separator; "
                    "use YYYY-MM-DD[ HH:MM:SS[+ZHZM]] format"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "timestamp(): Can't parse date"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(0.0).epsilon(0.01));
            }
        }

        WHEN("The date with year, month, day, hour, minute, second") {
            Teng::Error_t err;
            auto t = "${timestamp('2018-03-03 12:03:03')}";
            auto result = std::atof(g(err, t, root).c_str());

            THEN("The result is timestamp") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == Approx(1520035200.0).epsilon(0.01));
            }
        }
    }
}

SCENARIO(
    "The date function",
    "[fun][date]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The date has valid args") {
            std::time_t utc = 1520035200; // UTC 2018-03-03T00:00:00+00:00
            std::stringstream localTime; // For Time zone Europe/Prague (CET, +0100): 03.03.2018 01:00:00
            localTime << std::put_time(std::localtime(&utc), "%d.%m.%Y %H:%M:%S");

            Teng::Error_t err;
            auto t = "${date('%d.%m.%Y %H:%M:%S', 1520035200)}";
            auto result = g(err, t, root);

            THEN("The result is formatted time") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == localTime.str());
            }
        }
    }
}

SCENARIO(
    "The sectotime function",
    "[fun][date]"
) {
    GIVEN("No data") {
        Teng::Fragment_t root;

        WHEN("The date has valid args") {
            Teng::Error_t err;
            auto t = "${sectotime(1520035263)}";
            auto result = g(err, t, root);

            THEN("The result is current timestamp") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "422232:01:03");
            }
        }
    }
}


