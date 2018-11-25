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
 * Teng engine -- other functions.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <teng/teng.h>
#include <unistd.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The isenabled function",
    "[fun][other]"
) {
    GIVEN("The teng.conf with enabled shorttag") {
        Teng::Fragment_t root;

        WHEN("The option is queried") {
            Teng::Error_t err;
            auto t = "${isenabled('shorttag')}";
            auto result = g(err, t, root);

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }

    GIVEN("The teng.conf with not enabled bytecode") {
        Teng::Fragment_t root;

        WHEN("The option is queried") {
            Teng::Error_t err;
            auto t = "${isenabled('bytecode')}";
            auto result = g(err, t, root);

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }

    GIVEN("The teng.conf") {
        Teng::Fragment_t root;

        WHEN("The unknown option is queried") {
            Teng::Error_t err;
            auto t = "${isenabled('unknown-option')}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "isenabled(): Unknown feature 'unknown-option'"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The dictexist function",
    "[fun][other]"
) {
    GIVEN("The dict.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "");

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_cs_txt')}";
            auto result = g(err, t, root, "teng.conf", "");

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }

    GIVEN("The dict.cs.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_cs_txt')}";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }

    GIVEN("The dict.en.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_en_txt')}";
            auto result = g(err, t, root, "teng.conf", "en");

            THEN("The result is true") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${dictexist('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "en");

            THEN("The result is false") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "The getdict function",
    "[fun][other]"
) {
    GIVEN("The dict.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "");

            THEN("The result is value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "value");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_cs_txt')}";
            auto result = g(err, t, root, "teng.conf", "");

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The dict does not contain queried key but default given") {
            Teng::Error_t err;
            auto t = "${getdict('dict_cs_txt', 'default')}";
            auto result = g(err, t, root, "teng.conf", "");

            THEN("The result is default") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "default");
            }
        }
    }

    GIVEN("The dict.cs.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_cs_txt')}";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The result is value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "value");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The dict does not contain queried key but default given") {
            Teng::Error_t err;
            auto t = "${getdict('dict_txt', 'default')}";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The result is default") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "default");
            }
        }
    }

    GIVEN("The dict.en.txt") {
        Teng::Fragment_t root;

        WHEN("The dict contains queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_en_txt')}";
            auto result = g(err, t, root, "teng.conf", "en");

            THEN("The result is value") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "value");
            }
        }

        WHEN("The dict does not contain queried key") {
            Teng::Error_t err;
            auto t = "${getdict('dict_txt')}";
            auto result = g(err, t, root, "teng.conf", "en");

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The dict does not contain queried key but default given") {
            Teng::Error_t err;
            auto t = "${getdict('dict_cs_txt', 'default')}";
            auto result = g(err, t, root, "teng.conf", "en");

            THEN("The result is default") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "default");
            }
        }
    }
}

