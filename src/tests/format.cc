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
 * Teng engine -- formatting tests.
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
    "Change formatting of text",
    "[format]"
) {
    GIVEN("Format block that change formatting to NOWHITE") {
        auto t = " { \t <?teng format space='nowhite'?>"
                 " a  b  c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces in formated block are discarted") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == " { \t abc \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to ONESPACE") {
        auto t = " { \t <?teng format space='onespace'?>"
                 " a  b  c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces in formated block are merged") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == " { \t  a b c  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to STRIPLINES") {
        auto t = " { \t <?teng format space='striplines'?>"
                 " a \n b \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("No leading and trailing whitespaces") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == " { \t  a\nb\nc  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to JOINLINES") {
        auto t = " { \t <?teng format space='joinlines'?>"
                 " a \n b \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Spaces and newlines are merged") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == " { \t  a b c  \t } ");
            }
        }
    }

    GIVEN("Format block that change formatting to NOWHITELINES") {
        auto t = " { \t <?teng format space='nowhitelines'?>"
                 " a \n\n b \n  \n c <?teng endformat?> \t } ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Lines that contains whitespaces only are removed") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == " { \t  a \n b \n c  \t } ");
            }
        }
    }
}

SCENARIO(
    "Unclosed format directive",
    "[format]"
) {
    GIVEN("Template with unclosed format directive") {
        auto t = "{<?teng format space='onespace'?>  }  ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The format is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The closing directive of this <?teng format?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 38},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{ } ");
            }
        }
    }

    GIVEN("Template with unclosed format directive wrapping closed one") {
        auto t = "<?teng format space='nospace'?>  (  "
                 "{<?teng format space='onespace'?>  }  <?teng endformat?>"
                 "  )  ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The format is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng format?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 97},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "({ } )");
            }
        }
    }

    GIVEN("Template with closed and next unclosed format directive") {
        auto t = "<?teng format space='nospace'?>  (  <?teng endformat?>  )  "
                 "{<?teng format space='onespace'?>  }  ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The format is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 60},
                    "The closing directive of this <?teng format?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 97},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "(  )  { } ");
            }
        }
    }

    GIVEN("Template with two unclosed format blocks") {
        auto t = "<?teng format space='nospace'?>  (    )  "
                 "{<?teng format space='onespace'?>  }  ";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The format is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng format?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 42},
                    "The closing directive of this <?teng format?> directive "
                    "is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 79},
                    "Unexpected token: name=<EOF>, view="
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "(){ } ");
            }
        }
    }
}

SCENARIO(
    "The invalid open format directive",
    "[format]"
) {
    GIVEN("Template with open format directive without options") {
        auto t = "#<?teng format?>  &  <?teng endformat?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Default space formatting is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "The <?teng format?> directive must contain at least one "
                    "option in name=value format (e.g. space='nospace'); "
                    "ignoring this directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 14},
                    "Unexpected token: name=END, view=?>"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }

    GIVEN("Template with open format directive with error before options") {
        auto t = "#<?teng format 1 space='nospace'?>  &  <?teng endformat?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Default space formatting is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid excessive tokens in <?teng format?> directive "
                    "found; ignoring this directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }

    GIVEN("Template with open format directive with error after options") {
        auto t = "#<?teng format space='nospace' 1?>  &  <?teng endformat?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Default space formatting is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid excessive tokens in <?teng format?> directive "
                    "found; ignoring this directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 31},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }

    GIVEN("Template with open format directive with option of invalid type") {
        auto t = "#<?teng format space=1?>  &  <?teng endformat?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Default space formatting is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Formatting block has no effect; "
                    "option 'space' is not string"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }

    GIVEN("Template with open format directive with invalid options") {
        auto t = "#<?teng format 1+1?>  &  <?teng endformat?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("Default space formatting is preserved") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 1},
                    "Invalid excessive tokens in <?teng format?> directive "
                    "found; ignoring this directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 15},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#  &  #");
            }
        }
    }
}

SCENARIO(
    "The invalid close format directive",
    "[format]"
) {
    GIVEN("Template with close format directive with excessive tokens") {
        auto t = "#<?teng format space='nospace'?>  &  <?teng endformat 1?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The space formatting is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 37},
                    "Ignoring invalid excessive tokens in <?teng endformat?> "
                    "directive"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 54},
                    "Unexpected token: name=DEC_INT, view=1"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#&#");
            }
        }
    }

    GIVEN("Template with open format directive with options") {
        auto t = "#<?teng format space='nospace'?>  &  <?teng endformat a=1?>#";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("The space formatting is applied") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 54},
                    "This directive doesn't accept any options; ignoring them"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "#&#");
            }
        }
    }
}

SCENARIO(
    "The unopened format directive",
    "[format]"
) {
    GIVEN("Template with directive closing not opened format") {
        auto t = "{<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endformat?> directive closes unopened "
                    "format block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}");
            }
        }
    }

    GIVEN("Template with two directives closing not opened format") {
        auto t = "{<?teng endformat?>}{<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endformat?> directive closes unopened "
                    "format block"
                }, {
                    Teng::Error_t::WARNING,
                    {1, 21},
                    "The <?teng endformat?> directive closes unopened "
                    "format block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}{}");
            }
        }
    }

    GIVEN("Template with format after directive closing not opened format") {
        auto t = "{<?teng endformat?>}"
                 "{  <?teng format space='nospace'?>  <?teng endformat?>  }";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 1},
                    "The <?teng endformat?> directive closes unopened "
                    "format block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{}{    }");
            }
        }
    }

    GIVEN("Template with format before directive closing not opened format") {
         auto t = "{  <?teng format space='nospace'?>  <?teng endformat?>  }"
                  "{<?teng endformat?>}";

        WHEN("Generated with none data") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            auto result = g(err, t, root);

            THEN("It contains text from template") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 58},
                    "The <?teng endformat?> directive closes unopened "
                    "format block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "{    }{}");
            }
        }
    }
}

