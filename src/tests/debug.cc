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
 * Teng engine -- debug tools.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created.
 */

#include <teng/teng.h>

#include "catch.hpp"
#include "utils.h"

SCENARIO(
    "The debug fragment",
    "[debug]"
) {
    GIVEN("Some data") {
        Teng::Fragment_t root;
        root.addVariable("zero", 0);
        root.addVariable("pi", 3.14);
        root.addVariable("some_long_utf_string", "ěščřžýáíéééééééíáýžřčšě");
        auto &first = root.addFragment("first");
        auto &second = first.addFragment("second");
        second.addVariable("test", "TEST");
        auto &third = second.addFragment("third");
        root.addFragment("first");
        root.addFragment("first").addVariable("last_first", "LAST");

        WHEN("Generated with debug fragment enabled") {
            Teng::Error_t err;
            std::string t = "<?teng debug?>";
            auto result = g(err, t, root, "teng.debug.conf", "cs");
            auto r = "Template sources:\n"
                     "\n"
                     "Language dictionary sources:\n"
                     "    " TEST_ROOT "dict.cs.txt\n"
                     "\n"
                     "Configuration dictionary sources:\n"
                     "    " TEST_ROOT "teng.debug.conf\n"
                     "\n"
                     "Configuration:\n"
                     "    debug: enabled\n"
                     "    errorfragment: enabled\n"
                     "    logtooutput: disabled\n"
                     "    bytecode: enabled\n"
                     "    watchfiles: enabled\n"
                     "    maxincludedepth: 10\n"
                     "    maxdebugvallength: 40\n"
                     "    format: enabled\n"
                     "    alwaysescape: enabled\n"
                     "    printescape: enabled\n"
                     "    shorttag: enabled\n"
                     "\n"
                     "Application data:\n"
                     "    pi: 3.140000\n"
                     "    some_long_utf_string: 'ěščřžýáíéééééééíáý...'\n"
                     "    zero: 0\n"
                     "\n"
                     "    first[0]:\n"
                     "        second[0]:\n"
                     "            test: 'TEST'\n"
                     "\n"
                     "            third[0]:\n"
                     "\n"
                     "    first[1]:\n"
                     "\n"
                     "    first[2]:\n"
                     "        last_first: 'LAST'\n";

            THEN("The rendered template contains debug") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == r);
            }
        }

        WHEN("Generated with debug fragment disabled") {
            Teng::Error_t err;
            std::string t = "<?teng debug?>";
            auto result = g(err, t, root, "teng.conf", "cs");

            THEN("The rendered template does not contain debug") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "The bytecode fragment",
    "[debug]"
) {
    GIVEN("Some complex template") {
        Teng::Fragment_t root;
        root.addVariable("var", 1);
        std::string t = "${case(var,1,2:'a','3':'b',*:'c')}<?teng bytecode?>";

        WHEN("Generated with bytecode fragment enabled") {
            Teng::Error_t err;
            auto result = g(err, t, root, "teng.debug.conf", "cs");
            auto r = "000 VAR                 &lt;name=var,escape=true,"
                         "frame-offset=0,frag-offset=0&gt;\n"
                     "001 PRG_STACK_PUSH      \n"
                     "002 PRG_STACK_AT        &lt;index=0&gt;\n"
                     "003 VAL                 &lt;value=1,type=integral&gt;\n"
                     "004 EQ                  \n"
                     "005 OR                  &lt;jump=+3&gt;\n"
                     "006 PRG_STACK_AT        &lt;index=0&gt;\n"
                     "007 VAL                 &lt;value=2,type=integral&gt;\n"
                     "008 EQ                  \n"
                     "009 JMP_IF_NOT          &lt;jump=+2&gt;\n"
                     "010 VAL                 &lt;value=a,type=string&gt;\n"
                     "011 JMP                 &lt;jump=+7&gt;\n"
                     "012 PRG_STACK_AT        &lt;index=0&gt;\n"
                     "013 VAL                 &lt;value=3,type=string&gt;\n"
                     "014 EQ                  \n"
                     "015 JMP_IF_NOT          &lt;jump=+2&gt;\n"
                     "016 VAL                 &lt;value=b,type=string&gt;\n"
                     "017 JMP                 &lt;jump=+1&gt;\n"
                     "018 VAL                 &lt;value=c,type=string&gt;\n"
                     "019 PRG_STACK_POP       \n"
                     "020 PRINT               &lt;print_escape=true,unoptimizable=false&gt;\n"
                     "021 BYTECODE_FRAG       \n"
                     "022 HALT                \n";

            THEN("The rendered template contains bytecode") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == (std::string("a") + r));
            }
        }

        WHEN("Generated with bytecode fragment disabled") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("The rendered template does not contain bytecode") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "a");
            }
        }
    }
}

SCENARIO(
    "The error fragment",
    "[debug]"
) {
    GIVEN("Template with error fragment with no errors") {
        Teng::Fragment_t root;
        std::string t = "<?teng frag _error?>${filename}<?teng endfrag?>";

        WHEN("Template is rendered") {
            Teng::Error_t err;
            auto result = g(err, t, root, "teng.debug.conf");

            THEN("Nothing returned") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }

    GIVEN("Template with error fragment") {
        Teng::Fragment_t root;
        root.addVariable("var", 1);
        std::string t = "${missing}<?teng frag _error?>"
                        "${filename}/${line}/${column}/${level}/${message}"
                        "<?teng endfrag?>";

        WHEN("Generated with error fragment enabled") {
            Teng::Error_t err;
            auto result = g(err, t, root, "teng.debug.conf");
            std::string r = "undefined/1/2/1/Runtime: Variable '.missing' "
                            "is undefined [open_frags=., iteration=0/1]";

            THEN("The rendered template contains error fragment data") {
                std::vector<Teng::Error_t::Entry_t> errs {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Variable '.missing' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == r);
            }
        }

        WHEN("Generated with error fragment disabled") {
            Teng::Error_t err;
            auto result = g(err, t, root);

            THEN("The rendered template does not contain error fragment") {
                std::vector<Teng::Error_t::Entry_t> errs {{
                    Teng::Error_t::WARNING,
                    {1, 2},
                    "Runtime: Variable '.missing' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "undefined");
            }
        }
    }

    GIVEN("Template with error fragment referenced by runtime variables") {
        Teng::Fragment_t root;
        root.addVariable("var", 1);
        std::string t = "${var}/${$$._count}/${no}/${$$_error._count}";

        WHEN("Error fragment is referenced by runtime variables") {
            Teng::Error_t err;
            auto result = g(err, t, root, "teng.debug.conf");

            THEN("The rendered template contains error fragment data") {
                std::vector<Teng::Error_t::Entry_t> errs {{
                    Teng::Error_t::WARNING,
                    {1, 22},
                    "Runtime: Variable '.no' is undefined "
                    "[open_frags=., iteration=0/1]"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "1/1/undefined/1");
            }
        }
    }
}

