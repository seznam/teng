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
 * Teng engine -- old tests
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Coverted to catch2.
 */

#include <teng.h>

#ifndef SRC_DIR
#define SRC_DIR "."
#endif /* SRC_DIR */

#define TEST_ROOT SRC_DIR "/src/tests/"

inline std::string g(
    const std::string &templ,
    const Teng::Fragment_t &data = {},
    const std::string &ct = "text/html",
    const std::string &encoding = "utf-8"
) {
    std::string result;
    Teng::StringWriter_t writer(result);

    Teng::Error_t err;
    Teng::Teng_t teng(TEST_ROOT);
    teng.generatePage(templ, "", "", "", ct, encoding, data, writer, err);
    return result;
}

inline std::string g(
    Teng::Error_t &err,
    const std::string &templ,
    const Teng::Fragment_t &data = {},
    const std::string &lang = "",
    const std::string &ct = "text/html",
    const std::string &encoding = "utf-8",
    const std::string &conf_file = "teng.conf"
) {
    std::string result;
    Teng::StringWriter_t writer(result);
    Teng::Teng_t teng(TEST_ROOT);
    teng.generatePage(
        templ,
        TEST_ROOT "dict.txt",
        lang,
        TEST_ROOT + conf_file,
        ct,
        encoding,
        data,
        writer,
        err
    );
    return result;
}

#define ERRLOG_TEST(LHS, RHS)                                                  \
    for (int i = 0; i < std::min(LHS.size(), RHS.size()); ++i) {               \
        INFO("i=" + std::to_string(i));                                        \
        REQUIRE(LHS[i] == RHS[i]);                                             \
    }                                                                          \
    REQUIRE(LHS.size() == RHS.size())                                          \

