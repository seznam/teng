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
 * $Id: tengconfiguration.cc,v 1.5 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng configuration dictionary.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2004-09-18  (vasek)
 *             Created.
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "tenglogging.h"
#include "tengconfiguration.h"

namespace Teng {

Configuration_t::Configuration_t(Error_t &err, const std::string &fs_root)
    : Dictionary_t(err, fs_root),
      debug(false), errorFragment(false), logToOutput(false), bytecode(false),
      watchFiles(true), alwaysEscape(true), shortTag(false), format(true),
      maxIncludeDepth(10), maxDebugValLength(40), printEscape(true)
{}

teng_feature
Configuration_t::isEnabled(const string_view_t &name) const {
    auto bool2feature = [] (bool value) {
        return value? teng_feature::enabled: teng_feature::disabled;
    };

    // query known features
    if (name == "debug") return bool2feature(debug);
    if (name == "errorfragment") return bool2feature(errorFragment);
    if (name == "logtooutput") return bool2feature(logToOutput);
    if (name == "bytecode") return bool2feature(bytecode);
    if (name == "watchfiles") return bool2feature(watchFiles);
    if (name == "format") return bool2feature(format);
    if (name == "alwaysescape") return bool2feature(alwaysEscape);
    if (name == "printescape") return bool2feature(printEscape);
    if (name == "shorttag") return bool2feature(shortTag);

    // unknown features
    return teng_feature::unknown;
}

std::ostream &operator<<(std::ostream &o, const Configuration_t &c) {
    auto bool2string = [] (bool value) {return value? "enabled": "disabled";};
    o << "Configuration:" << std::endl
      << "    debug: " << bool2string(c.debug) << std::endl
      << "    errorfragment: " << bool2string(c.errorFragment) << std::endl
      << "    logtooutput: " << bool2string(c.logToOutput) << std::endl
      << "    bytecode: " << bool2string(c.bytecode) << std::endl
      << "    watchfiles: " << bool2string(c.watchFiles) << std::endl
      << "    maxincludedepth: " << c.maxIncludeDepth << std::endl
      << "    maxdebugvallength: " << c.maxDebugValLength << std::endl
      << "    format: " << bool2string(c.format) << std::endl
      << "    alwaysescape: " << bool2string(c.alwaysEscape) << std::endl
      << "    printescape: " << bool2string(c.printEscape) << std::endl
      << "    shorttag: " << bool2string(c.shortTag) << std::endl;
    return o;
}

Configuration_t::error_code
Configuration_t::new_directive(
    const char *name_ptr, std::size_t name_len,
    const char *value_ptr, std::size_t value_len
) {
    string_view_t name = {name_ptr, name_len};
    string_view_t value = {value_ptr, value_len};

    // lambda that converts directive value to number
    auto to_number = [&] (auto &&result) {
        if (!value.empty()) {
            char *end;
            auto depth = strtoul(value.data(), &end, 10);
            if (*end == '\0') {
                result = depth;
                return error_code::none;
            }
        }
        return error_code::invalid_number;
    };

    // the numeric directives
    if (name == "maxincludedepth")
        return to_number(maxIncludeDepth);
    if (name == "maxdebugvallength")
        return to_number(maxDebugValLength);

    // lambda that enables Teng features
    auto enable_feature = [&] (bool enable) {
        auto do_enable = [&] (bool &v) {v = enable; return error_code::none;};
        if (value == "debug") return do_enable(debug);
        if (value == "errorfragment") return do_enable(errorFragment);
        if (value == "logtooutput") return do_enable(logToOutput);
        if (value == "bytecode") return do_enable(bytecode);
        if (value == "watchfiles") return do_enable(watchFiles);
        if (value == "format") return do_enable(format);
        if (value == "alwaysescape") return do_enable(alwaysEscape);
        if (value == "printescape") return do_enable(printEscape);
        if (value == "shorttag") return do_enable(shortTag);
        return enable? error_code::invalid_enable: error_code::invalid_disable;
    };

    // enable/disable Teng features
    return name == "enable"
        ? enable_feature(true)
        : name == "disable"
        ? enable_feature(false)
        : Dictionary_t::new_directive(name_ptr, name_len, value_ptr, value_len);
}

} // namespace Teng

