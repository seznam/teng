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
 * $Id: tengdictionary.cc,v 1.6 2006-01-27 14:04:41 vasek Exp $
 *
 * DESCRIPTION
 * Teng dictionary -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-19  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 */

#include <array>
#include <cstdio>
#include <utility>
#include <cctype>
#include <cerrno>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "tenglogging.h"
#include "tengdictionary.h"
#include "tengplatform.h"
#include "tengaux.h"

namespace Teng {
namespace {

/** Compose absolute filename.
 */
std::string absfile(const std::string &fs_root, const std::string &filename) {
    return !fs_root.empty() && !filename.empty() && !ISROOT(filename)
         ? fs_root + "/" + filename
         : filename;
}

/**
 * @short Extracts line from string.
 *
 * Removes terminating <LF> and (optional) preceding <CR>.
 *
 * @param str input string
 * @param offset pointer to the first char of current line
 *
 * @return line and position of terminating <LF> in input string
 */
std::pair<string_view_t, const char *>
getLine(Error_t &err, Pos_t &pos, const string_view_t &str, const char *iline) {
    // search for next newline
    auto eline = std::find(iline, str.end(), '\n');

    // no new line found
    if (eline == str.end()) {
        pos.advanceColumn(eline - iline + 1);
        logWarning(err, pos, "No newline at end of file");
        return  {{iline, eline}, eline};
    }

    // trim '\r' if any
    if ((iline != str.begin()) && (*(iline - 1) == '\r'))
        return {{iline, eline - 1}, eline};

    // done
    return  {{iline, eline}, eline};
}

/** Expands escape characters.
 */
std::array<char, 3>
expand_escape(
    Error_t &err,
    Pos_t &pos,
    const char *ivalue,
    const char *evalue
) {
    // escaping of end-of-line is not allowed
    if (++ivalue == evalue) {
        logError(err, pos, "Escaping EOL not allowed");
        pos.advanceColumn();
        return {{'\\', '\0', '\0'}};
    }

    // escape -> interpret next char
    pos.advanceColumn(2);
    switch (*ivalue) {
    case 'n':
        return {{'\n', '\0', '\0'}};
    case 'r':
        return {{'\r', '\0', '\0'}};
    case 't':
        return {{'\t', '\0', '\0'}};
    case 'v':
        return {{'\v', '\0', '\0'}};
    case '\\':
        return {{'\\', '\0', '\0'}};
    case '"':
        return {{'\"', '\0', '\0'}};
    default:
        // other chars are not allowed to be escaped
        logError(err, pos, "Invalid escape character");
        return {{'\\', *ivalue, '\0'}};
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Warns if tail contains no whitespaces.
 */
void
warn_if_tail_is_not_empty(
    Error_t &err,
    Pos_t &pos,
    const char *ivalue,
    const char *evalue
) {
    for (; ivalue != evalue; ++ivalue) {
        if (!isspace(*ivalue)) {
            logError(err, pos, "Text after quoted line");
            return;
        }
        pos.advance(*ivalue);
    }
}

} // namespace

int
Dictionary_t::setBool(string_view_t name, string_view_t param, bool &value) {
    if (param == "yes") {
        value = true;
        return 0;
    }
    if (param == "no") {
        value = false;
        return 0;
    }
    logError(
        err,
        pos,
        "Invalid bool value of bool " + name.str()
        + " directive '" + param.str() + "'"
    );
    return -1;
}

int Dictionary_t::includeFile(string_view_t filename) {
    // include other source
    if (!level) {
        logError(err, pos, "Too many includes");
        return -1;
    }

    // strip trailing whitespaces
    auto efilename = filename.end();
    for (; efilename > filename.begin(); --efilename)
        if (!isspace(*(efilename - 1)))
            break;
    filename = {filename.begin(), efilename};

    // check filename
    if (!filename) {
        logError(err, pos, "Missing file to include");
        return -1;
    }

    // include it
    --level;
    int ret = parse(filename.str(), pos);
    ++level;
    return ret;
}

std::string Dictionary_t::parseValueLine(string_view_t value) {
    // trim all leading whitespaces
    auto ivalue = value.begin();
    for (auto evalue = value.end(); ivalue != evalue; ++ivalue) {
        if (isspace(*ivalue)) pos.advance(*ivalue);
        else break;
    }

    // reserve necessary space
    std::string result;
    result.reserve(value.end() - ivalue);

    // if value is still quoted at its end => quote not closed
    struct quoted_t {
        ~quoted_t() {if (v) logError(err, pos, "Missing terminating quote");}
        quoted_t &operator=(bool nv) {v = nv; return *this;}
        explicit operator bool() const {return v;}
        bool v; Error_t &err; Pos_t &pos;
    } quoted{false, err, pos};

    // process whole string char by char
    for (bool first = true; ivalue != value.end(); ++ivalue) {
        switch (*ivalue) {
        case '\\':
            // escape sequence
            result.append(expand_escape(err, pos, ivalue, value.end()).data());
            break;
        case '"':
            // quote
            pos.advanceColumn();
            if (first) {
                quoted = true;
            } else {
                if (quoted) {
                    warn_if_tail_is_not_empty(err, pos, ++ivalue, value.end());
                    return result;
                }
                result.push_back(*ivalue);
            }
            break;
        default:
            // push to value
            result.push_back(*ivalue);
            pos.advance(*ivalue);
            break;
        }
        // indicate that we are not at the first character
        first = false;
    }

    // done
    return result;
}

std::string *Dictionary_t::addIdent(string_view_t line) {
    // get all valid chars (assumes that first char is not number)
    auto ivalue = line.begin();
    for (auto eline = line.end(); ivalue != eline; ++ivalue)
        if (!(isalnum(*ivalue) || (*ivalue == '_')))
            break;
    pos.advanceColumn(ivalue - line.begin());

    // if first non-valid char is not white, report it as error
    if ((ivalue != line.end()) && (!isspace(*ivalue))) {
        logError(err, pos, "Invalid character in identifier");
        return nullptr;
    }

    // parse rest of line as value
    string_view_t name = {line.begin(), ivalue};
    std::string value = parseValueLine({ivalue, line.end()});

    // add ident
    return addImpl(name, std::move(value));
}

void Dictionary_t::add(const std::string &name, const std::string &value) {
    addImpl(name, std::move(value));
}

std::string *Dictionary_t::addImpl(string_view_t name, std::string value) {
    return replaceValue
        ? &(dict[name.str()] = std::move(value))
        : &dict.emplace(name.str(), std::move(value)).first->second;
}

std::string *Dictionary_t::add(string_view_t name, string_view_t value) {
    // insert new record into table immediately if expansion is disabled
    if (!expandValue) return addImpl(name, {value.begin(), value.end()});

    // expand value
    std::string expanded;
    expanded.reserve(value.size());

    // expand all directives
    for (const char *ivalue = value.begin(); ivalue != value.end();) {
        // find name openning
        static const std::string O = "#{";
        auto iopen = std::search(ivalue, value.end(), O.begin(), O.end());
        if (iopen == value.end()) {
            expanded.append(ivalue, value.end());
            break;
        }
        expanded.append(ivalue, iopen);

        // find name closing
        auto iclose = std::find(iopen, value.end(), '}');
        if (iclose == value.end()) {
            logError(err, pos, "Unterminated #{} directive.");
            expanded.append(iopen, value.end());
            return addImpl(name, expanded);
        }

        // compose dict value key
        std::string key = {iopen + 2, iclose - 1};
        ivalue = iclose + 1;

        // try to find key in so far read entries
        if (const std::string *dict_value = lookup(key)) {
            expanded.append(*dict_value);

        } else {
            expanded.append("#{");
            expanded.append(key);
            expanded.push_back('}');
            logError(err, pos, "Dictionary item '" + key + "' not found");
        }
    }

    // just add expanded value
    return add(name, expanded);
}

int
Dictionary_t::processDirective(string_view_t directive, string_view_t param) {
    if (directive == "include")
        return includeFile(param);
    else if (directive == "expand")
        return setBool(directive, param, expandValue);
    else if (directive == "replace")
        return setBool(directive, param, replaceValue);
    logError(err, pos, "Unknown procesing directive");
    return -1;
}

int Dictionary_t::processDirective(string_view_t line) {
    // search for value
    auto ivalue = std::find_if(line.begin(), line.end(), isspace);
    pos.advanceColumn(ivalue - line.begin());

    // trim all leading whitespaces
    for (auto evalue = line.end(); ivalue != evalue; ++ivalue) {
        if (isspace(*ivalue)) pos.advance(*ivalue);
        else break;
    }

    // process directive
    return processDirective({line.begin(), ivalue}, {ivalue, line.end()});
}

const std::string *Dictionary_t::lookup(const std::string &key) const {
    auto ientry = dict.find(key);
    return ientry == dict.end()
         ? (key == "_tld"? &get_tld(): nullptr)
         : &ientry->second;
}

int Dictionary_t::parseString(string_view_t data) {
    // return value -- default ok
    int ret = 0;

    // process whole input line by line
    string_view_t line;
    auto iline = data.begin(), eline = data.end();
    std::string *last_inserted_value = nullptr;
    for (; iline != eline; pos.newLine(), ++iline) {

        // get next line and skip empty lines
        std::tie(line, iline) = getLine(err, pos, data, iline);
        if (!line) continue;

        // what kind of line is it
        switch (line[0]) {
        case '#':
            // skip comments
            last_inserted_value = nullptr;
            continue;

        case '%':
            // it is directive
            if (processDirective({line.begin() + 1, line.end()}))
                ret = -1;
            last_inserted_value = nullptr;
            break;

        case ' ': case '\t': case '\v':
            // continuation
            if (last_inserted_value) {
                std::string value = parseValueLine(line);
                last_inserted_value->push_back(' ');
                last_inserted_value->append(value);
            } else {
                logError(err, pos, "No line to concatenate with");
                ret = -1;
            }
            break;

        case '.': case '_':
        case 'a' ... 'z':
        case 'A' ... 'Z':
            last_inserted_value = addIdent(line);
            break;

        default:
            last_inserted_value = nullptr;
            logError(err, pos, "Illegal identifier: line=" + line.str());
            ret = -1;
        }
    }
    return ret;
}

int Dictionary_t::parse(const std::string &filename, const Pos_t &include_pos) {
    // if relative path => prepend root
    std::string path = absfile(fs_root, filename);

    // insert source into source list
    auto *source_path = sources.push(path, include_pos, err).first;

    // reset pos and parse new file
    pos = {source_path};
    std::ifstream file(filename);
    return parse(file, filename);
}

int Dictionary_t::parse(std::ifstream &file, const std::string &filename) {
    try {
        // restoring file stream exception settings
        struct exception_restorer_t {
            exception_restorer_t(std::ifstream *file)
                : file(file), saved_exceptions(file->exceptions())
            {file->exceptions(std::ios::failbit | std::ios::badbit);}
            ~exception_restorer_t() {file->exceptions(saved_exceptions);}
            std::ifstream *file;
            std::ios::iostate saved_exceptions;
        } restorer(&file);

        // read whole file into memory
        std::string str;
        file.seekg(0, std::ios::end);
        str.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&str[0], str.size());

        // parse loaded string
        return parseString(str);

    } catch (const std::system_error &e) {
        std::string sys = std::string("(") + e.code().message() + ")";
        logError(err, pos, "Error reading file '" + filename + "' " + sys);
        return -1;
    }
}

void Dictionary_t::dump(std::string &out) const {
    std::ostringstream os;
    dump(os);
    out.append(os.str());
}

void Dictionary_t::dump(std::ostream &out) const {
    for (auto &item: dict) {
        out << item.first << ": |" << item.second
            << "|\n----------------------------------------\n";
    }
}

} // namespace Teng

