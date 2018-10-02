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
 * Teng engine -- case expressions.
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
    "The switch/case operator",
    "[expr][case]"
) {
    GIVEN("Some variables") {
        Teng::Fragment_t root;
        root.addVariable("aaa", "(aaa)");
        root.addVariable("bbb", "(bbb)");

        WHEN("The case with branch matching variable value is evaluated") {
            Teng::Error_t err;
            auto t = "${case($aaa, '(aaa)': 'aaa == (aaa)', *: '(xxx)')}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "aaa == (aaa)");
            }
        }

        WHEN("The case with branch no matching variable value is evaluated") {
           Teng::Error_t err;
           auto t = "${case($aaa, '(xxx)': 'aaa != (xxx)', *: '(aaa)')}";
           auto result = g(err, t, root);

           THEN("The right (default) branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "(aaa)");
           }
        }

        WHEN("The case with three branches is evaluated (first branch)") {
           Teng::Error_t err;
           root.addVariable("n", 1);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The first branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "a");
           }
        }

        WHEN("The case with three branches is evaluated (second branch)") {
           Teng::Error_t err;
           root.addVariable("n", 2);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The second branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "b");
           }
        }

        WHEN("The case with three branches is evaluated (third branch)") {
           Teng::Error_t err;
           root.addVariable("n", 3);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c', *: 'z')}";
           auto result = g(err, t, root);

           THEN("The third branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "c");
           }
        }

        WHEN("The case with no default branch is evaluated") {
           Teng::Error_t err;
           root.addVariable("n", 4);
           auto t = "${case(n, 1: 'a', 2: 'b', 3: 'c')}";
           auto result = g(err, t, root);

           THEN("Result is undefined") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "undefined");
           }
        }

        WHEN("The case complex expressions is evaluated") {
           Teng::Error_t err;
           root.addVariable("n", 3);
           root.addVariable("m", 4);
           auto t = "${case(n - m, -1: 'n' ++ 'm', *: '')}";
           auto result = g(err, t, root);

           THEN("The right branch has been choosen") {
               std::vector<Teng::Error_t::Entry_t> errs;
               REQUIRE(err.getEntries() == errs);
               REQUIRE(result == "nm");
           }
        }
    }
}

SCENARIO(
    "The nested switch/case operator",
    "[expr][case]"
) {
    GIVEN("Some variables") {
        Teng::Fragment_t root;
        root.addVariable("aaa", "(aaa)");
        root.addVariable("bbb", "(bbb)");

        WHEN("Nested case in non default option returns non default value") {
            Teng::Error_t err;
            std::string n = "case(aaa, '(aaa)': 'AAA', *: 'yyy')";
            std::string t = "${case(aaa, '(aaa)': " + n + ", *: 'xxx')}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "AAA");
            }
        }

        WHEN("Nested case in non default option returns default value") {
            Teng::Error_t err;
            std::string n = "case(bbb, '(aaa)': 'AAA', *: 'yyy')";
            std::string t = "${case(aaa, '(aaa)': " + n + ", *: 'xxx')}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "yyy");
            }
        }

        WHEN("Nested case in default option returns non default value") {
            Teng::Error_t err;
            std::string n = "case(aaa, '(aaa)': 'AAA', *: 'yyy')";
            std::string t = "${case(bbb, '(aaa)': 'xxx', *: " + n + ")}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "AAA");
            }
        }

        WHEN("Nested case in default option returns default value") {
            Teng::Error_t err;
            std::string n = "case(bbb, '(aaa)': 'AAA', *: 'yyy')";
            std::string t = "${case(bbb, '(aaa)': 'xxx', *: " + n + ")}";
            auto result = g(err, t, root);

            THEN("The right branch has been choosen") {
                std::vector<Teng::Error_t::Entry_t> errs;
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "yyy");
            }
        }
    }
}

SCENARIO(
    "The nested switch/case operator with more than one option",
    "[expr][case]"
) {
    GIVEN("Complicated nested case expressions") {
        Teng::Error_t err;
        Teng::Fragment_t root;
        std::string t = "${case("
            "n, "
            "1: case(m, 1: 'aa', 2: 'ab', 3,4: 'ac', *: 'az'),"
            "2,3: case(m, 1: 'ba', 2: 'bb', 3,4: 'bc', *: 'bz'),"
            "4: case(m,"
                     "1: 'ca',"
                     "2: 'cb',"
                     "3,4: case(o, 1: 'cca', *: 'ccz'),"
                     "*: 'cz'),"
            "*: 'z')}";

        WHEN("Variable n = 1") {
            root.addVariable("n", 1);

            WHEN("Variable m = 1") {
                root.addVariable("m", 1);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "aa");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "aa");
                    }
                }
            }

            WHEN("Variable m = 2") {
                root.addVariable("m", 2);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ab");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ab");
                    }
                }
            }

            WHEN("Variable m = 3") {
                root.addVariable("m", 3);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ac");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ac");
                    }
                }
            }

            WHEN("Variable m = 4") {
                root.addVariable("m", 4);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ac");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ac");
                    }
                }
            }

            WHEN("Variable m = 5") {
                root.addVariable("m", 5);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "az");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "az");
                    }
                }
            }
        }

        WHEN("Variable n = 2") {
            root.addVariable("n", 2);

            WHEN("Variable m = 1") {
                root.addVariable("m", 1);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ba");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ba");
                    }
                }
            }

            WHEN("Variable m = 2") {
                root.addVariable("m", 2);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bb");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bb");
                    }
                }
            }

            WHEN("Variable m = 3") {
                root.addVariable("m", 3);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }
            }

            WHEN("Variable m = 4") {
                root.addVariable("m", 4);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }
            }

            WHEN("Variable m = 5") {
                root.addVariable("m", 5);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bz");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bz");
                    }
                }
            }
        }

        WHEN("Variable n = 3") {
            root.addVariable("n", 3);

            WHEN("Variable m = 1") {
                root.addVariable("m", 1);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ba");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ba");
                    }
                }
            }

            WHEN("Variable m = 2") {
                root.addVariable("m", 2);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bb");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bb");
                    }
                }
            }

            WHEN("Variable m = 3") {
                root.addVariable("m", 3);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }
            }

            WHEN("Variable m = 4") {
                root.addVariable("m", 4);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bc");
                    }
                }
            }

            WHEN("Variable m = 5") {
                root.addVariable("m", 5);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bz");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "bz");
                    }
                }
            }
        }

        WHEN("Variable n = 4") {
            root.addVariable("n", 4);

            WHEN("Variable m = 1") {
                root.addVariable("m", 1);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ca");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ca");
                    }
                }
            }

            WHEN("Variable m = 2") {
                root.addVariable("m", 2);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cb");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cb");
                    }
                }
            }

            WHEN("Variable m = 3") {
                root.addVariable("m", 3);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cca");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ccz");
                    }
                }
            }

            WHEN("Variable m = 4") {
                root.addVariable("m", 4);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cca");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "ccz");
                    }
                }
            }

            WHEN("Variable m = 5") {
                root.addVariable("m", 5);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cz");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "cz");
                    }
                }
            }
        }

        WHEN("Variable n = 5") {
            root.addVariable("n", 5);

            WHEN("Variable m = 1") {
                root.addVariable("m", 1);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }
            }

            WHEN("Variable m = 2") {
                root.addVariable("m", 2);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }
            }

            WHEN("Variable m = 3") {
                root.addVariable("m", 3);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }
            }

            WHEN("Variable m = 4") {
                root.addVariable("m", 4);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }
            }

            WHEN("Variable m = 5") {
                root.addVariable("m", 5);

                WHEN("Variable o = 1") {
                    root.addVariable("o", 1);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }

                WHEN("Variable o = 2") {
                    root.addVariable("o", 2);

                    THEN("The right branch has been choosen") {
                        auto result = g(err, t, root);
                        std::vector<Teng::Error_t::Entry_t> errs;
                        REQUIRE(err.getEntries() == errs);
                        REQUIRE(result == "z");
                    }
                }
            }
        }
    }
}

SCENARIO(
    "The error in case condition",
    "[expr][casex]"
) {
    GIVEN("Some variables") {
        Teng::Fragment_t root;
        root.addVariable("aaa", "(aaa)");
        root.addVariable("bbb", "(bbb)");

        WHEN("The expression ends immediately after left parentheses") {
            Teng::Error_t err;
            std::string t = "${case(/*}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 7},
                    "Invalid condition in case expression"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 7},
                    "Unterminated comment"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 7},
                    "Unexpected token: name=INV, view=/*}"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }

        WHEN("The case ends immediately after left parentheses") {
            Teng::Error_t err;
            std::string t = "${case(}";
            auto result = g(err, t, root);

            THEN("The result is undefined") {
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Invalid expression, fix it please; replacing whole "
                    "expression with undefined value"
                }, {
                    Teng::Error_t::DIAG,
                    {1, 7},
                    "Missing closing ')' in case expression"
                }, {
                    Teng::Error_t::ERROR,
                    {1, 7},
                    "Unexpected token: name=SHORT_END, view=}"
                }};
                REQUIRE(err.getEntries() == errs);
                REQUIRE(result == "undefined");
            }
        }
    }
}

