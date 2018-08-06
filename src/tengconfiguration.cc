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
namespace {

string_view_t strip(const string_view_t &str) {
    const char *begin = str.begin();
    const char *end = str.end();
    while ((begin < end) && isspace(*begin)) ++begin;
    while ((begin < end) && isspace(*(end - 1))) --end;
    return {begin, end};
}

} // namespace

Configuration_t::Configuration_t(const std::string &root)
    : Dictionary_t(root), debug(false), errorFragment(false),
      logToOutput(false), bytecode(false), watchFiles(true),
      alwaysEscape(true), shortTag(false), format(true),
      maxIncludeDepth(10), maxDebugValLength(40)
{}

int Configuration_t::processDirective(string_view_t directive, string_view_t param) {
    // strip argument
    string_view_t arg = strip(param);

    if (directive == "maxincludedepth") {
        if (arg) {
            char *end;
            auto depth = strtoul(arg.data(), &end, 10);
            if (*end == '\0') {
                maxIncludeDepth = depth;
                return 0;
            }
        }
        logError(err, pos, "Bad maxincludedepth '" + arg.str() + "' value");
        return -1;

    }

    if (directive == "maxdebugvallength") {
        if (arg) {
            char *end;
            auto len = strtoul(arg.data(), &end, 10);
            if (*end == '\0') {
                maxDebugValLength = len;
                return 0;
            }
        }
        logError(err, pos, "Bad maxdebugvallength '" + arg.str() + "' value");
        return -1;
    }

    // enable/disable
    bool value = false;
    if (directive == "enable") value = true;
    else if (directive == "disable") value = false;
    else return Dictionary_t::processDirective(directive, param);

    if (arg == "debug") debug = value;
    else if (arg == "errorfragment") errorFragment = value;
    else if (arg == "logtooutput") logToOutput = value;
    else if (arg == "bytecode") bytecode = value;
    else if (arg == "watchfiles") watchFiles = value;
    else if (arg == "format") format = value;
    else if (arg == "alwaysescape") alwaysEscape = value;
    else if (arg == "shorttag") shortTag = value;
    else {
        logError(err, pos, "Bad enable/disable argument '" + arg.str() + "'");
        return -1;
    }

    // OK
    return 0;
}

int
Configuration_t::isEnabled(const std::string &feature, bool &enabled) const {
    if (feature == "debug") enabled = debug;
    else if (feature == "errorfragment") enabled = errorFragment;
    else if (feature == "logtooutput") enabled = logToOutput;
    else if (feature == "bytecode") enabled = bytecode;
    else if (feature == "watchfiles") enabled = watchFiles;
    else if (feature == "format") enabled = format;
    else if (feature == "alwaysescape") enabled = alwaysEscape;
    else if (feature == "shortag") enabled = shortTag;
    else return -1;
    return 0;
}

std::ostream &operator<<(std::ostream &o, const Configuration_t &c) {
    auto ENABLED = [] (bool value) {return value? "enabled": "disabled";};
    o << "Configuration: " << std::endl
      << "    debug: " << ENABLED(c.debug) << std::endl
      << "    errorfragment: " << ENABLED(c.errorFragment) << std::endl
      << "    logtooutput: " << ENABLED(c.logToOutput) << std::endl
      << "    bytecode: " << ENABLED(c.bytecode) << std::endl
      << "    watchfiles: " << ENABLED(c.watchFiles) << std::endl
      << "    maxincludedepth: " << c.maxIncludeDepth << std::endl
      << "    maxdebugvallength: " << c.maxDebugValLength << std::endl
      << "    format: " << ENABLED(c.format) << std::endl
      << "    alwaysescape: " << ENABLED(c.alwaysEscape) << std::endl
      << "    shorttag: " << ENABLED(c.shortTag) << std::endl;
    return o;
}

} // namespace Teng

