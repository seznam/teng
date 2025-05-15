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
 * $Id: tengwriter.cc,v 1.2 2006-01-27 14:03:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng writer -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 */


#include "util.h"
#include "logging.h"
#include "teng/writer.h"

#include <iostream>

namespace Teng {

StringWriter_t::StringWriter_t(std::string &str)
    : str(str)
{}

int StringWriter_t::write(const std::string &str) {
    this->str.append(str);
    return 0;
}

int StringWriter_t::write(const char *str) {
    this->str.append(str);
    return 0;
}

int StringWriter_t::write(const char *str, std::size_t size) {
    this->str.append(str, size);
    return 0;
}

int StringWriter_t::write(const std::string &, StringSpan_t interval) {
    this->str.append(interval.first, interval.second);
    return 0;
}

FileWriter_t::FileWriter_t(const std::string &filename)
    : file(fopen(filename.c_str(), "w")), borrowed(false)
{
    if (!file && err) {
        logFatal(
            *err,
            "Cannot open file '" + filename + "' (" + strerr(errno) + ")"
        );
    }
}

FileWriter_t::FileWriter_t(FILE *file)
    : Writer_t(), file(file), borrowed(true)
{
    if (!file && err)
        logFatal(*err, "Got invalid file handle (nullptr)");
}

FileWriter_t::~FileWriter_t() {
    if (!borrowed && file)
        fclose(file);
}

int FileWriter_t::write(const std::string &str) {
    if (!file) return -1;
    fwrite(str.data(), 1, str.length(), file);
    if (feof(file) || ferror(file)) {
        logFatal(*err, "Error writing to output (" + strerr(errno) + ")");
        return -1;
    }
    return 0;
}

int FileWriter_t::write(const char *str) {
    return write(str, strlen(str));
}

int FileWriter_t::write(const char *str, std::size_t size) {
    if (!file) return -1;
    fwrite(str, 1, size, file);
    if (feof(file) || ferror(file)) {
        logFatal(*err, "Error writing to output (" + strerr(errno) + ")");
        return -1;
    }
    return 0;
}

int FileWriter_t::write(const std::string &str, StringSpan_t interval) {
    const char *cstr = str.data() + std::distance(str.begin(), interval.first);
    size_t len = std::distance(interval.first, interval.second);
    fwrite(cstr, 1, len, file);
    if (feof(file) || ferror(file)) {
        logFatal(*err, "Error writing to output (" + strerr(errno) + ")");
        return -1;
    }
    return 0;
}

int FileWriter_t::flush() {
    if (!file) return -1;
    return (fflush(file) ? -1 : 0);
}

} // namespace Teng

