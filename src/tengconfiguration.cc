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

#include "tengconfiguration.h"

namespace Teng {

Configuration_t::Configuration_t(const FilesystemInterface_t *filesystem)
    : Dictionary_t(filesystem), debug(false), errorFragment(false),
      logToOutput(false), bytecode(false), watchFiles(true),
      alwaysEscape(true), shortTag(false), maxIncludeDepth(10),
      format(true), maxDebugValLength(40)
{}

Configuration_t::~Configuration_t() {
    // no-op
}

namespace {

std::string strip(const std::string &str) {
    std::string::size_type begin = 0;
    std::string::size_type end = str.length();
    while ((begin < end) && isspace(str[begin])) ++begin;
    while ((begin < end) && isspace(str[end - 1])) --end;
    return std::string(str, begin, end - begin);
}

} // namespace

int Configuration_t::processDirective(const std::string &directive,
                                      const std::string &param,
                                      Error_t::Position_t &pos)
{
    // strip argument
    std::string argument(strip(param));

    if (directive == "maxincludedepth") {
        if (argument.empty()) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid value of max-include-depth '"
                         + argument + "'");
            return -1;
        }

        // convert to unsigned int
        char *end;
        unsigned long int depth = strtoul(argument.c_str(), &end, 10);
        if (*end) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid value of max-include-depth '"
                         + argument + "'");
            return -1;
        }

        maxIncludeDepth = depth;
        return 0;
    }

    if (directive == "maxdebugvallength") {
        if (argument.empty()) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid value of max-debug-val-length '"
                         + argument + "'");
            return -1;
        }

        // convert to unsigned int
        char *end;
        unsigned long int len = strtoul(argument.c_str(), &end, 10);
        if (*end) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid value of max-debug-val-length '"
                         + argument + "'");
            return -1;
        }

        maxDebugValLength = len;
        return 0;
    }


    // enable/disable

    bool value = false;
    if (directive == "enable") value = true;
    else if (directive == "disable") value = false;
    else {
        // other directive
        return Dictionary_t::processDirective(directive, param, pos);
    }

    if (argument == "debug") debug = value;
    else if (argument == "errorfragment") errorFragment = value;
    else if (argument == "logtooutput") logToOutput = value;
    else if (argument == "bytecode") bytecode = value;
    else if (argument == "watchfiles") watchFiles = value;
    else if (argument == "format") format = value;
    else if (argument == "alwaysescape") alwaysEscape = value;
    else if (argument == "shorttag") shortTag = value;
    else {
        err.logError(Error_t::LL_ERROR, pos,
                     "Invalid enable/disable argument '" + argument + "'");
        return -1;
    }

    // OK
    return 0;
}

int Configuration_t::isEnabled(const std::string &feature,
                               bool &enabled) const
{
    if (feature == "debug") enabled = debug;
    else if (feature == "errorfragment") enabled = errorFragment;
    else if (feature == "logtooutput") enabled = logToOutput;
    else if (feature == "bytecode") enabled = bytecode;
    else if (feature == "watchfiles") enabled = watchFiles;
    else if (feature == "format") enabled = format;
    else if (feature == "alwaysescape") enabled = alwaysEscape;
    else if (feature == "shortag") enabled = shortTag;
    else return -1;

    // OK
    return 0;
}

namespace {

const char* ENABLED(bool value) {
    return value ? "enabled" : "disabled";
}

} // namespace

std::ostream& operator<<(std::ostream &o, const Configuration_t &c) {
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

