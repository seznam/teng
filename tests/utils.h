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

#include <teng/teng.h>

#include "catch2/catch_approx.hpp"

#ifndef SRC_DIR
#define SRC_DIR "."
#endif /* SRC_DIR */

#define TEST_ROOT SRC_DIR "/tests/"

using Catch::Approx;

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
    Teng::Teng_t::GenPageArgs_t args;
    args.contentType = ct;
    args.encoding = encoding;
    args.templateString = templ;
    teng.generatePage(args, data, writer, err);
    return result;
}

inline std::string g(
    Teng::Error_t &err,
    const std::string &templ,
    const Teng::Fragment_t &data = {},
    const std::string &params = "teng.conf",
    const std::string &lang = "",
    const std::string &ct = "text/html",
    const std::string &encoding = "utf-8"
) {
    std::string result;
    Teng::StringWriter_t writer(result);
    Teng::Teng_t teng(TEST_ROOT);
    Teng::Teng_t::GenPageArgs_t args;
    args.contentType = ct;
    args.encoding = encoding;
    args.templateString = templ;
    args.paramsFilename = TEST_ROOT + params;
    args.lang = lang;
    args.dictFilename = TEST_ROOT "dict.txt";
    teng.generatePage(args, data, writer, err);
    return result;
}

inline std::string gFromFile(
    Teng::Error_t &err,
    const std::string &filename,
    const Teng::Fragment_t &data = {},
    const std::string &params = "teng.conf",
    const std::string &lang = "",
    const std::string &ct = "text/html",
    const std::string &encoding = "utf-8"
) {
    std::string result;
    Teng::StringWriter_t writer(result);
    Teng::Teng_t teng(TEST_ROOT);
    Teng::Teng_t::GenPageArgs_t args;
    args.contentType = ct;
    args.encoding = encoding;
    args.templateFilename = filename;
    args.paramsFilename = TEST_ROOT + params;
    args.lang = lang;
    args.dictFilename = TEST_ROOT "dict.txt";
    teng.generatePage(args, data, writer, err);
    return result;
}

#define ERRLOG_TEST(LHS, RHS)                                                  \
    for (uint64_t i = 0; i < std::min(LHS.size(), RHS.size()); ++i) {          \
        INFO("i=" + std::to_string(i));                                        \
        REQUIRE(LHS[i] == RHS[i]);                                             \
    }                                                                          \
    REQUIRE(LHS.size() == RHS.size())                                          \

