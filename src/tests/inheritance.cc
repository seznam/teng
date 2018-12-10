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
 * Teng engine -- test of teng variables.
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
    "Generating base template without overrides",
    "[inheritance]"
) {
    GIVEN("Template without any overrides") {
        Teng::Fragment_t root;

        WHEN("The template is generated") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "before"
                  "<?define block body?>"
                  "some-data"
                  "<?enddefine block?>"
                  "after"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The define directive are ignored") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "beforesome-dataafter");
            }
        }
    }
}

SCENARIO(
    "Simple template inheritance",
    "[inheritance]"
) {
    GIVEN("Base template with two extending blocks") {
        Teng::Fragment_t root;

        WHEN("The overriden blocks are simple text") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?>"
                  "        overriden"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        <meta>"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The override blocks are used") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head><meta></head>"
                                  "<body>overriden</body></html>");
            }
        }

        WHEN("The overriden blocks are expressions") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?>"
                  "        <?expr'overr'?>${'iden'}"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        %{'<meta>'}"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The override blocks are used") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head><meta></head>"
                                  "<body>overriden</body></html>");
            }
        }

        WHEN("The only one block is overriden") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block head?>"
                  "        <meta>"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base block is used for not overriden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head><meta></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The overriding block contains escaped text") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?><\\?aaa?\\><?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The overriden block is used") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body><?aaa?></body></html>");
            }
        }
    }
}

SCENARIO(
    "Calling base implementation of overriden block",
    "[inheritance]"
) {
    GIVEN("Base template with two extending blocks") {
        Teng::Fragment_t root;

        WHEN("The both overriden block call base implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?>"
                  "        ==<?super block?>=="
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        ==<?super block?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementation is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>====</head>"
                                  "<body>==base==</body></html>");
            }
        }

        WHEN("The one of overriden block call base implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?>"
                  "        ==<?super block?>=="
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        ==--=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementation is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==--==</head>"
                                  "<body>==base==</body></html>");
            }
        }

        WHEN("The base implementation is called more than once") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block body?>"
                  "        ==<?super block?>=="
                  "        ==<?super block?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementation is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>==base====base==</body></html>");
            }
        }
    }
}

SCENARIO(
    "The chain of extending templates",
    "[inheritance]"
) {
    GIVEN("The middle template overrides no one block") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-empty.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override one block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-empty.html'?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level override is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override both blocks") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-empty.html'?>"
                  "    <?override block body?>"
                  "        bbbb"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level overrides are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>bbbb</body></html>");
            }
        }

        WHEN("The override block calls super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-empty.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>====</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The both override blocks call super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-empty.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "    <?override block body?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>====</head>"
                                  "<body>==base==</body></html>");
            }
        }
    }

    GIVEN("The middle template overrides one block") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-one.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>head</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override one block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-one.html'?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level override is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override both blocks") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-one.html'?>"
                  "    <?override block body?>"
                  "        bbbb"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level overrides are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>bbbb</body></html>");
            }
        }

        WHEN("The override block calls super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-one.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The both override blocks call super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-one.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "    <?override block body?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>==base==</body></html>");
            }
        }
    }

    GIVEN("The middle template overrides both blocks") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-two.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>head</head>"
                                  "<body>body</body></html>");
            }
        }

        WHEN("The top-level override one block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-two.html'?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level override is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>body</body></html>");
            }
        }

        WHEN("The top-level override both blocks") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-two.html'?>"
                  "    <?override block body?>"
                  "        bbbb"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level overrides are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>bbbb</body></html>");
            }
        }

        WHEN("The override block calls super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-two.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>body</body></html>");
            }
        }

        WHEN("The both override blocks call super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-two.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "    <?override block body?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>==body==</body></html>");
            }
        }
    }

    GIVEN("The middle template overrides both blocks and one calls super") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-super.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>head</head>"
                                  "<body>--base--</body></html>");
            }
        }

        WHEN("The top-level override one block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-super.html'?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level override is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>--base--</body></html>");
            }
        }

        WHEN("The top-level override both blocks") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-super.html'?>"
                  "    <?override block body?>"
                  "        bbbb"
                  "    <?endoverride block?>"
                  "    <?override block head?>"
                  "        hhhh"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The top-level overrides are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>hhhh</head>"
                                  "<body>bbbb</body></html>");
            }
        }

        WHEN("The override block calls super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-super.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>--base--</body></html>");
            }
        }

        WHEN("The both override blocks call super implementation") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-super.html'?>"
                  "    <?override block head?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "    <?override block body?>"
                  "        ==<?super?>=="
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The direct predescor is called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>==head==</head>"
                                  "<body>==--base--==</body></html>");
            }
        }
    }
}

SCENARIO(
    "Defining new block in other one",
    "[inheritance]"
) {
    GIVEN("The base template has nested defines") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base-nested-define.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>=TITLE=</head>"
                                  "<body></body></html>");
            }
        }

        WHEN("The top-level override nested block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base-nested-define.html'?>"
                  "    <?override block title?>"
                  "        nice-title"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>=nice-title=</head>"
                                  "<body></body></html>");
            }
        }

        WHEN("The top-level override outer block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base-nested-define.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>that'shead</head>"
                                  "<body></body></html>");
            }
        }

        WHEN("The top-level override outer as well as nested block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base-nested-define.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "    <?override block title?>"
                  "        hidden-title"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>that'shead</head>"
                                  "<body></body></html>");
            }
        }
    }

    GIVEN("The middle template defines new block") {
        Teng::Fragment_t root;

        WHEN("The top-level does not override any block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-define.html'?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The base implementations are called") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head><title>unknown</title></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override nested block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-define.html'?>"
                  "    <?override block title?>"
                  "        nice-title"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head><title>nice-title</title></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override outer block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-define.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>that'shead</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The top-level override outer as well as nested block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='override-define.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "    <?override block title?>"
                  "        hidden-title"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The nested block is overridden") {
                std::vector<Teng::Error_t::Entry_t> errs;
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>that'shead</head>"
                                  "<body>base</body></html>");
            }
        }
    }
}

SCENARIO(
    "Invalid extends blocks",
    "[inheritance]"
) {
    GIVEN("Base template with two extending blocks") {
        Teng::Fragment_t root;

        WHEN("There is nested extends block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?extends file='base.html'?>"
                  "    <?endextends?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 58},
                    "There is already open extends block at (no file):1:26; "
                    "ignoring the extends directive"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("There is nested extends block with override block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?extends file='base.html'?>"
                  "        <?override block head?>"
                  "            that's head"
                  "        <?endoverride block?>"
                  "    <?endextends?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The override block is used for outer extends") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 58},
                    "There is already open extends block at (no file):1:26; "
                    "ignoring the extends directive"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head>that'shead</head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The unclosed extends block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 125},
                    "The unclosed <?teng extends?>; ignoring it"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The unopened end override block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng format?> "
                    "directive is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 30},
                    "The misplaced OVERRIDE_BLOCK token, "
                    "it has to be placed in <?teng extends?> block"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 76},
                    "The misplaced ENDBLOCK_OVERRIDE token, "
                    "it has to be placed in <?teng extends?> block"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 97},
                    "Unexpected token: name=ENDEXTENDS, view=<?endextends"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "that'shead");
            }
        }

        WHEN("The invalid tokens in extends directive") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends abc file='base.html'?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 26},
                    "Invalid or excessive tokens in <?teng extends?>"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 40},
                    "Unexpected token: name=IDENT, view=file"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The invalid numeric value of file options") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file=3?>"
                  "    <?override block head?>"
                  "        that's head"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 26},
                    "Can't extends template; the 'file' value is not string"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

SCENARIO(
    "Invalid override blocks",
    "[inheritance]"
) {
    GIVEN("Base template with two extending blocks") {
        Teng::Fragment_t root;

        WHEN("There is nested override block out of extends") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "<?endextends?>"
                  "<?override block head?>"
                  "<?endoverride block?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 68},
                    "The misplaced OVERRIDE_BLOCK token, "
                    "it has to be placed in <?teng extends?> block"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 91},
                    "The misplaced ENDBLOCK_OVERRIDE token, "
                    "it has to be placed in <?teng extends?> block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The unclosed misplaced override block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "<?endextends?>"
                  "<?override block head?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 68},
                    "The misplaced OVERRIDE_BLOCK token, "
                    "it has to be placed in <?teng extends?> block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("The unopened misplaced end override block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "<?endextends?>"
                  "<?endoverride block?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 68},
                    "The misplaced ENDBLOCK_OVERRIDE token, "
                    "it has to be placed in <?teng extends?> block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }

        WHEN("Invalid override block id") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?extends file='base.html'?>"
                  "    <?override block 3?>"
                  "    <?endoverride block?>"
                  "<?endextends?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 58},
                    "Ignoring override block with invalid block id"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 75},
                    "Unexpected token: name=DEC_INT, view=3"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "<htm><head></head>"
                                  "<body>base</body></html>");
            }
        }
    }
}

SCENARIO(
    "Invalid define blocks",
    "[inheritance]"
) {
    GIVEN("Base template with two extending blocks") {
        Teng::Fragment_t root;

        WHEN("The unclosed misplaced override block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?define block head?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 47},
                    "The <?teng define block?> is not closed"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("The unopened end define block") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?enddefine block?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 0},
                    "The closing directive of this <?teng format?> "
                    "directive is missing"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 26},
                    "Unexpected token: name=ENDBLOCK_DEFINE, "
                    "view=<?enddefine block"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }

        WHEN("Invalid define block id") {
            Teng::Error_t err;
            std::string t
                = "<?format space='nospace'?>"
                  "<?define block 3?>"
                  "<?enddefine block?>"
                  "<?endformat?>";
            auto result = g(err, t, root);

            THEN("The syntax error is reported") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 26},
                    "Ignoring define block with invalid block id"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 41},
                    "Unexpected token: name=DEC_INT, view=3"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
                REQUIRE(result == "");
            }
        }
    }
}

