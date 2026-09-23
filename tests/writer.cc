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
 * Teng engine -- writers tests.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2026-09-23  (burlog)
 *             Created.
 */

#include <teng/teng.h>

#include "catch2/catch_test_macros.hpp"
#include "utils.h"

namespace {

/** Generates the template to the given file and returns the error log.
 */
std::vector<Teng::Error_t::Entry_t>
gToFile(const std::string &filename, const std::string &templ) {
    Teng::Error_t err;
    Teng::Fragment_t root;
    Teng::FileWriter_t writer(filename);
    Teng::Teng_t teng(TEST_ROOT);
    Teng::Teng_t::GenPageArgs_t args;
    args.templateString = templ;
    teng.generatePage(args, root, writer, err);
    return err.getEntries();
}

} // namespace

SCENARIO(
    "Generating page to file that can't be opened",
    "[writer]"
) {
    GIVEN("The path to file in non existent directory") {
        std::string path = "/there-is-no-such-dir/out.html";

        WHEN("Non empty template is generated to it") {
            auto errs = gToFile(path, "some text");

            THEN("The reason is reported in error log just once") {
                REQUIRE(errs.size() == 1);
                REQUIRE(errs[0].level == Teng::Error_t::FATAL);
                REQUIRE(errs[0].msg.compare(0, 16, "Cannot open file") == 0);
            }
        }

        WHEN("Empty template is generated to it") {
            auto errs = gToFile(path, "");

            THEN("The reason is reported in error log by flush") {
                REQUIRE(errs.size() == 1);
                REQUIRE(errs[0].level == Teng::Error_t::FATAL);
                REQUIRE(errs[0].msg.compare(0, 16, "Cannot open file") == 0);
            }
        }
    }
}

SCENARIO(
    "Generating page to invalid file handle",
    "[writer]"
) {
    GIVEN("The null file handle") {
        FILE *file = nullptr;

        WHEN("Template is generated to it") {
            Teng::Error_t err;
            Teng::Fragment_t root;
            Teng::FileWriter_t writer(file);
            Teng::Teng_t teng(TEST_ROOT);
            Teng::Teng_t::GenPageArgs_t args;
            args.templateString = "some text";
            teng.generatePage(args, root, writer, err);
            auto errs = err.getEntries();

            THEN("The reason is reported in error log") {
                REQUIRE(errs.size() == 1);
                REQUIRE(errs[0].level == Teng::Error_t::FATAL);
                REQUIRE(errs[0].msg == "Got invalid file handle (nullptr)");
            }
        }
    }
}
