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
 * Teng engine -- Teng expressions tests.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <limits>
#include <teng/teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "Real numeric == operator",
    "[numeric][expr][real][fuzzer]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);
        root.addVariable("max_real", std::numeric_limits<double>::max());
        root.addVariable("min_real", std::numeric_limits<double>::min());
        root.addVariable("lowest", std::numeric_limits<double>::lowest());

        WHEN("MAX_REAL was printed") {
            Teng::Error_t err;
            auto result = g(err, "${max_real}", root);

            THEN("Library don't crash") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "179769313486231570814527423731704356798070"
                                  "5675258449965989174768031572607800285387605"
                                  "8955863276687817154045895351438246423432132"
                                  "6889464182768467546703537516986049910576551"
                                  "2820762454900903893289440758685084551339423"
                                  "0458323690322294816580855933212334827479782"
                                  "6204144723168738177180919299881250404026184"
                                  "124858368.0");
            }
        }

        WHEN("MIN_REAL was printed") {
            Teng::Error_t err;
            auto result = g(err, "${min_real}", root);

            THEN("Library don't crash") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("LOWEST was printed") {
            Teng::Error_t err;
            auto result = g(err, "${lowest}", root);

            THEN("Library don't crash") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-179769313486231570814527423731704356798070"
                                  "5675258449965989174768031572607800285387605"
                                  "8955863276687817154045895351438246423432132"
                                  "6889464182768467546703537516986049910576551"
                                  "2820762454900903893289440758685084551339423"
                                  "0458323690322294816580855933212334827479782"
                                  "6204144723168738177180919299881250404026184"
                                  "124858368.0");
            }
        }

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three == tri}", root);

            THEN("Result of: three == tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three == two}", root);

            THEN("Result of: three == two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${3.1 == three}", root);

            THEN("Result of: 3.1 == three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three == 2.1}", root);

            THEN("Result of: three == 2.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Real numeric != operator",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three != tri}", root);

            THEN("Result of: three != tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three != two}", root);

            THEN("Result of: three != two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${3.1 != three}", root);

            THEN("Result of: 3.1 != three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three != 2.1}", root);

            THEN("Result of: three != 2.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Real numeric >= operator",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three >= tri}", root);

            THEN("Result of: three >= tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${two >= three}", root);

            THEN("Result of: two >= three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${4.1 >= three}", root);

            THEN("Result of: 4.1 >= three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three >= 4.1}", root);

            THEN("Result of: three >= 4.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Real numeric < operator",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three < tri}", root);

            THEN("Result of: three < tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${two < three}", root);

            THEN("Result of: two < three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${2.1 < three}", root);

            THEN("Result of: 2.1 < three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three < 2.1}", root);

            THEN("Result of: three < 2.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "Real numeric <= operator",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three <= tri}", root);

            THEN("Result of: three <= tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${two <= three}", root);

            THEN("Result of: two <= three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${4.1 <= three}", root);

            THEN("Result of: 4.1 <= three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three <= 4.1}", root);

            THEN("Result of: three <= 4.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Real numeric > operator",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("two", 2.1);
        root.addVariable("three", 3.1);
        root.addVariable("tri", 3.1);

        WHEN("Variables with same value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three > tri}", root);

            THEN("Result of: three > tri") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variables with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${two > three}", root);

            THEN("Result of: two > three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${2.1 > three}", root);

            THEN("Result of: 2.1 > three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("Variable and literal with different value are compared") {
            Teng::Error_t err;
            auto result = g(err, "${three > 2.1}", root);

            THEN("Result of: three > 2.1") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }
    }
}

SCENARIO(
    "Logic OR operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("nula", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables with same value") {
            Teng::Error_t err;
            auto result = g(err, "${zero || nula}", root);

            THEN("Result of: zero || nula") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to variables with different value") {
            Teng::Error_t err;
            auto result = g(err, "${three || zero}", root);

            THEN("Result of: three || zero") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.3");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${0 || three}", root);

            THEN("Result of: 0 || three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.3");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${three || 0}", root);

            THEN("Result of: three || 0") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.3");
            }
        }
    }
}

SCENARIO(
    "Logic AND operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("nula", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables with same value") {
            Teng::Error_t err;
            auto result = g(err, "${zero && nula}", root);

            THEN("Result of: zero && nula") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to variables with different value") {
            Teng::Error_t err;
            auto result = g(err, "${three && zero}", root);

            THEN("Result of: three && zero") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${0.0 && three}", root);

            THEN("Result of: 0.0 && three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${three && 2.3}", root);

            THEN("Result of: three && 2.3") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "2.3");
            }
        }
    }
}

SCENARIO(
    "Bit OR operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero | three}", root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 7},
                    "Runtime: The left operand of | numeric operator is a "
                    "real but an integer is expected"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Bit XOR operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero ^ three}", root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 7},
                    "Runtime: The left operand of ^ numeric operator is a "
                    "real but an integer is expected"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "Bit AND operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero & three}", root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 7},
                    "Runtime: The left operand of & numeric operator is a "
                    "real but an integer is expected"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The PLUS operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);
        root.addVariable("minus_three", -3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero + three}", root);

            THEN("Result of: zero + three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.3");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 + three}", root);

            THEN("Result of: 2.2 + three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "5.5");
            }
        }

        WHEN("The operator is applied to negative variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 + minus_three}", root);

            THEN("Result of: 2.2 + minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-1.1");
            }
        }
    }
}

SCENARIO(
    "The MINUS operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);
        root.addVariable("minus_three", -3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero - three}", root);

            THEN("Result of: zero - three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-3.3");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 - three}", root);

            THEN("Result of: 2.2 - three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-1.1");
            }
        }

        WHEN("The operator is applied to negative variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 - minus_three}", root);

            THEN("Result of: 2.2 - minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "5.5");
            }
        }
    }
}

SCENARIO(
    "The CONCAT operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero ++ three}", root);

            THEN("Result is concatenation of stringified real numbers") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.03.3");
            }
        }
    }
}

SCENARIO(
    "The MUL operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.3);
        root.addVariable("minus_three", -3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero * three}", root);

            THEN("Result of: zero * three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to negative variables") {
            Teng::Error_t err;
            auto result = g(err, "${minus_three * minus_three}", root);

            THEN("Result of: minus_three * minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "10.89");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 * three}", root);

            THEN("Result of: 2.2 * three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "7.26");
            }
        }

        WHEN("The operator is applied to negative variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 * minus_three}", root);

            THEN("Result of: 2.2 * minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-7.26");
            }
        }
    }
}

SCENARIO(
    "The DIV operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("two", 2.2);
        root.addVariable("three", 3.3);
        root.addVariable("minus_three", -3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${three / two}", root);

            THEN("Result of: three / two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1.5");
            }
        }

        WHEN("The operator is applied to negative variables") {
            Teng::Error_t err;
            auto result = g(err, "${minus_three / three}", root);

            THEN("Result of: minus_three / three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-1.0");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 / three}", root);

            THEN("Result of: 2.2 / three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.666667");
            }
        }

        WHEN("The literal is diveded by zero") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 / zero}", root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 6},
                    "Runtime: Right operand of / division operator is zero"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The MOD operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0);
        root.addVariable("two", 2);
        root.addVariable("three", 3.3);
        root.addVariable("minus_three", -3.3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${three % two}", root);

            THEN("Result of: three % two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The operator is applied to negative variables") {
            Teng::Error_t err;
            auto result = g(err, "${minus_three % two}", root);

            THEN("Result of: minus_three % two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-1");
            }
        }

        WHEN("The operator is applied to variable and literal") {
            Teng::Error_t err;
            auto result = g(err, "${4.2 % two}", root);

            THEN("Result of: 4.2 % two") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }

        WHEN("The literal is diveded by zero") {
            Teng::Error_t err;
            auto result = g(err, "${2.2 % zero}", root);

            THEN("Result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 6},
                    "Runtime: Right operand of % division operator is zero"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The REPEAT operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", "0.0");
        root.addVariable("three", 3);

        WHEN("The operator is applied to variables") {
            Teng::Error_t err;
            auto result = g(err, "${zero ** three}", root);

            THEN("Result of: zero ** three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.00.00.0");
            }
        }
    }
}

SCENARIO(
    "The NOT operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.1);

        WHEN("The operator is applied to zero") {
            Teng::Error_t err;
            auto result = g(err, "${!zero}", root);

            THEN("Result of: !zero") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1");
            }
        }

        WHEN("The operator is applied to three") {
            Teng::Error_t err;
            auto result = g(err, "${!three}", root);

            THEN("Result of: !three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

SCENARIO(
    "The BIT_NOT operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);

        WHEN("The operator is applied to zero") {
            Teng::Error_t err;
            auto result = g(err, "${~zero}", root);

            THEN("Result of: ~zero") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Operand of bit ~ operator is not int"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

SCENARIO(
    "The unary MINUS operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.1);
        root.addVariable("minus_three", -3.1);

        WHEN("The operator is applied to zero") {
            Teng::Error_t err;
            auto result = g(err, "${-zero}", root);

            THEN("Result of: -zero") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-0.0");
            }
        }

        WHEN("The operator is applied to positive real number") {
            Teng::Error_t err;
            auto result = g(err, "${-three}", root);

            THEN("Result of: -three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-3.1");
            }
        }

        WHEN("The operator is applied to negative real number") {
            Teng::Error_t err;
            auto result = g(err, "${-minus_three}", root);

            THEN("Result of: -minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.1");
            }
        }

        WHEN("The operator is multiply applied to real number") {
            Teng::Error_t err;
            auto result = g(err, "${--three}", root);

            THEN("Result of: --three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.1");
            }
        }

        WHEN("The operator is used in complex expression") {
            Teng::Error_t err;
            auto result = g(err, "${4.1 * --three}", root);

            THEN("Result of: 4.1 * --three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "12.71");
            }
        }
    }
}

SCENARIO(
    "The unary PLUS operator for real numbers",
    "[numeric][expr][real]"
) {
    GIVEN("Some variables in root frag set to real numbers") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0.0);
        root.addVariable("three", 3.1);
        root.addVariable("minus_three", -3.1);

        WHEN("The operator is applied to zero") {
            Teng::Error_t err;
            auto result = g(err, "${+zero}", root);

            THEN("Result of: +zero") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0.0");
            }
        }

        WHEN("The operator is applied to positive real number") {
            Teng::Error_t err;
            auto result = g(err, "${+three}", root);

            THEN("Result of: +three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.1");
            }
        }

        WHEN("The operator is applied to negative real number") {
            Teng::Error_t err;
            auto result = g(err, "${+minus_three}", root);

            THEN("Result of: +minus_three") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "-3.1");
            }
        }

        WHEN("The operator is multiply applied to real number") {
            Teng::Error_t err;
            auto result = g(err, "${+(+three)}", root);

            THEN("Result of: +(+three)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "3.1");
            }
        }

        WHEN("The operator is used in complex expression") {
            Teng::Error_t err;
            auto result = g(err, "${4.1 * +(+three)}", root);

            THEN("Result of: 4.1 * +(+three)") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "12.71");
            }
        }
    }
}

SCENARIO(
    "Fuzzer problems in expression with real numbers",
    "[real][fuzzer]"
) {
    GIVEN("Comparison of real number with string") {
        Teng::Fragment_t root;
        std::string t = "${1.1e20 =~ 1.1}";

        WHEN("The expression is evaluated") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("Memory leak is fixed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "0");
            }
        }
    }
}

