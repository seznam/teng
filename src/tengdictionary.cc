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


#include <iostream>
#include <utility>
#include <ctype.h>
#include <errno.h>

#include <stdio.h>

#include "tengdictionary.h"
#include "tengplatform.h"

using namespace std;

using namespace Teng;

int Dictionary_t::parse(const string &filename) {
    level = MAX_RECURSION_LEVEL;
    Error_t::Position_t pos(filename);
    return parse(filename, pos);
}

Dictionary_t::~Dictionary_t() {
}

namespace {
    /**
     * @short Extracts line from string.
     *
     * Removes terminating <LF> and (optional) preceding <CR>.
     *
     * @param str input string
     * @param line found line
     * @param begin start of line
     * @param pos position in current file
     * @param err error logger
     * @return position of terminating <LF> in input string
     */
    string::size_type getLine(const string &str, string &line,
                              string::size_type begin)
    {
        // find <LF>
        string::size_type nl = str.find('\n', begin);
        // remove (optional) preceding<CR>.
        if ((nl > 0) && (nl != string::npos) && (str[nl - 1] == '\r'))
            line = str.substr(begin, nl - begin - 1);
        else line = str.substr(begin, nl - begin);
        // return position of terminating <LF> in input string
        return nl;
    }
}

/**
 * @short Parses value line.
 *
 * @param line parsed line
 * @param value returend value
 * @param pos position in current file
 * @param err error logger
 * @return 0 OK !0 error
 */
int Dictionary_t::parseValueLine(const string &line, string &value,
                                 Error_t::Position_t &pos)
{
    // erase value
    value.erase();
    // strip all whitespaces from begin
    string::const_iterator iline = line.begin();
    while ((iline != line.end()) && isspace(*iline)) {
        if (*iline == '\t') pos.advanceToTab();
        else pos.advanceColumn();
        ++iline;
    }
    // reserve necessary space
    value.reserve(line.end() - iline);
    // indicates quoted line
    bool quoted = false;
    // process whole string char by char
    for (bool first = true; iline != line.end(); ++iline) {
        switch (*iline) {
        case '\\':
            // escape -> interpret next char
            if ((iline + 1) != line.end()) {
                // next char available, advance to it
                ++iline;
                // interpret char
                switch (*iline) {
                case 'n':
                    value.push_back('\n');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'v':
                    value.push_back('\v');
                    break;
                case '\\':
                    value.push_back('\\');
                    break;
                case '"':
                    value.push_back('"');
                    break;
                case '\t':
                    // tabs are not allowed to be escaped
                    err.logError(Error_t::LL_ERROR, pos, "Invalid escape");
                    value.push_back(*(iline - 1));
                    value.push_back(*iline);
                    pos.advanceColumn();
                default:
                    // other chars are not allowed to be escaped
                    err.logError(Error_t::LL_ERROR, pos, "Invalid escape");
                    value.push_back(*(iline - 1));
                    value.push_back(*iline);
                    break;
                }
            } else {
                // escaping of end-of-line is not allowed
                err.logError(Error_t::LL_ERROR, pos,
                             "Escaping EOL not allowed");
            }
            pos.advanceColumn();
            break;
        case '"':
            // quote
            if (first) {
                // first quote => we have quoted line
                quoted = true;
                pos.advanceColumn();
            } else {
                // other quote
                if (quoted) {
                    // close quoted line
                    ++iline;
                    pos.advanceColumn();
                    // run through rest of line and find any non-white char
                    while (iline != line.end()) {
                        if (!isspace(*iline)) {
                            // if found, report it as error
                            err.logError(Error_t::LL_ERROR, pos,
                                         "Text after quoted line");
                            return -1;
                        }
                        if (*iline == '\t') pos.advanceToTab();
                        else pos.advanceColumn();
                        ++iline;
                    }
                    // OK
                    return 0;
                }
                // insert quote to the value
                value.push_back(*iline);
            }
            break;
        case '\t':
            // extra only for tab alignment
            value.push_back(*iline);
            pos.advanceToTab();
            break;
        default:
            // push to value
            value.push_back(*iline);
            pos.advanceColumn();
            break;
        }
        // indicate that we are not at the first character
        first = false;
    }

    if (quoted) {
        // if line is still quoted at its end => quote not closed
        err.logError(Error_t::LL_ERROR, pos, "Missing terminating quote");
        return -1;
    }

    // OK
    return 0;
}

int Dictionary_t::parseIdentLine(const string &line, string &name, string &value,
                                 Error_t::Position_t &pos)
{
    // get all valid chars (assumes that first char is not number)
    string::const_iterator iline = line.begin();
    for (; iline != line.end(); ++iline) {
        if (!(isalnum(*iline) || (*iline == '_')))
            break;
        pos.advanceColumn();
    }
    if (iline != line.end()) {
        // if first non-valid char is not white, report it as error
        if (!isspace(*iline)) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid character in identifier");
            return -1;
        }
    }
    // cut name
    name = line.substr(0, iline - line.begin());
    // parse rest of line as value
    return parseValueLine(line.substr(iline - line.begin()), value, pos);
}

int Dictionary_t::add(const string &name, const string &value) {
    dict.insert(map<string, string>::value_type(make_pair(name, value)));
    return 0;
}

int Dictionary_t::add(const string &name, const string &value,
                      Error_t::Position_t &pos)
{
    // insert new record into table
    if (expandValue) {
        // expand value
        std::string expanded;
        expanded.reserve(value.size());

        for (std::string::size_type index = 0; index != std::string::npos; ) {
            // find name openning
            std::string::size_type open = value.find("#{", index);
            if (open == std::string::npos) {
                expanded.append(value, index, std::string::npos);
                break;
            } else {
                expanded.append(value, index, open - index);
            }

            // find name closing
            std::string::size_type close = value.find("}", open);
            if (close == std::string::npos) {
                err.logError(Error_t::LL_ERROR, pos,
                             "Unterminated #{} directive.");
                expanded.append(value, open, std::string::npos);
                add(name, expanded);
                return -1;
            }

            std::string ename(value, open + 2, close - open - 2);
            index = close + 1;

            // try to find ename in so far read entries
            const string *evalue = lookup(ename);
            if (!evalue) {
                // not found
                err.logError(Error_t::LL_ERROR, pos,
                             "Dictionary item '" + ename + "' not found.");
                expanded.append("#{");
                expanded.append(ename);
                expanded.push_back('}');
            } else {
                // found
                expanded.append(*evalue);
            }
        }

        // just add expanded value
        return add(name, expanded);
    }

    // add value
    return add(name, value);
}

const string* Dictionary_t::lookup(const string &key) const {
    // try to find key
    map<string, string>::const_iterator f = dict.find(key);
    // not found => null
    if (f == dict.end()) return 0;
    // return value
    return &f->second;
}

int Dictionary_t::parseString(const string &data,
                              Error_t::Position_t &pos)
{
    // position of newline
    string::size_type nl = 0;
    // current line
    string line;
    // name of current ident
    string currentName;
    // current value
    string currentValue;
    // identifies that currentName && currentValue are valid
    bool currentValid = false;

    // return value -- default ok
    int ret = 0;
    // forever
    for (;;) {
        // get next line
        nl = getLine(data, line, nl);
        
        // if not comment line, process it
        if (!(line.empty() || (line[0] == '#'))) {
            // get first element on line
            char first = line[0];
            if (first == '%') {
                // process directive
                if (currentValid) {
                    // insert new identifier
                    add(currentName, currentValue, pos);
                    currentValid = false;
                }
                // split directive to name and value
                string::size_type sep = line.find_first_of(" \t\v", 1);
                if (processDirective(line.substr(1, ((sep == string::npos)
                                                     ? sep : (sep - 1))),
                                     ((sep == string::npos)
                                      ? string()
                                      : line.substr(sep + 1)), pos))
                    ret = -1;
            } else if (isspace(first)) {
                // append to previous line
                if (currentValid) {
                    string value;
                    parseValueLine(line, value, pos);
                    currentValue.push_back(' ');
                    currentValue.append(value);
                } else {
                    // no open line
                    err.logError(Error_t::LL_ERROR, pos,
                                 "No line to concatenate with");
                    ret = -1;
                }
            } else if (isalpha(first) || (first == '_')  || (first == '.')) {
                // new identifier
                // insert new identifier
                if (currentValid)
                    add(currentName, currentValue, pos);
                // parse identifier line
                currentValid = !parseIdentLine(line, currentName,
                                               currentValue, pos);
            } else {
                err.logError(Error_t::LL_ERROR, pos, "Illegal identifier");
                ret = -1;
            }
        } else {
            // comment or empty line terminates entry
            if (currentValid)
                add(currentName, currentValue, pos);
            currentValid = false;
        }
        // if not at end advance to next character (after newline)
        if (nl != string::npos) ++nl;
        else {
            // if line is not empty, last char is not newline
            if (!line.empty()) {
                // move to end of line and report error
                pos.setColumn(line.length() + 1);
                err.logError(Error_t::LL_WARNING, pos,
                             "No newline at end of file");
                // error indicator will be returned
                ret = -1;
            }
            break;
        }
        // move to next line
        pos.newLine();
    }

    // if current data are valid, we must insert appropriate record
    if (currentValid)
        add(currentName, currentValue, pos);

    // return error indicator
    return ret;
}

int Dictionary_t::parse(const string &infilename,
                        Error_t::Position_t &pos)
{
    // if relative path => prepend root
    string filename = infilename;
    if (!filename.empty() && !ISROOT(filename) && (!root.empty()))
        filename = root + '/' + filename;

    // insert source into source list
    sources.addSource(filename, pos, err);

    // open file
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        // on error, log it
        err.logSyscallError(Error_t::LL_ERROR, pos,
                            "Cannot open file '" + filename + "'");
        return -1;
    }

    // create new positioner
    Error_t::Position_t newPos(filename, 1);
    // parse file
    int status = parse(file, newPos);
    // close file
    fclose(file);
    // return status
    return status;
}

int Dictionary_t::parse(FILE *file, Error_t::Position_t &pos) {
    // loaded string
    string str;
    // read whole file into memory
    while (!(feof(file) || ferror(file))) {
        char buff[1024];
        size_t r = fread(buff, 1, 1024, file);
        if (r)
            str.append(buff, r);
    }

    // on error report it and terminate processing
    if (ferror(file)) {
        err.logSyscallError(Error_t::LL_ERROR, pos, "Error reading file");
        return -1;
    }

    // parse loaded string
    return parseString(str, pos);
}

int Dictionary_t::processDirective(const string &directive,
                                   const string &param,
                                   Error_t::Position_t &pos)
{
    if (directive == "include") {
        // include other source
        if (!level) {
            err.logError(Error_t::LL_ERROR, pos, "Too many includes");
            return -1;
        }
        // cut filename
        string filename(param);
        pos.advanceColumn(8);
        if (filename.empty()) {
            err.logError(Error_t::LL_ERROR, pos, "Missing file to include");
            return -1;
        }
        // strip filename
        string::size_type begin = 0;
        string::size_type end = filename.length();
        while ((begin < end) && isspace(filename[begin]))
            ++begin;
        while ((begin < end) && isspace(filename[end - 1]))
            --end;
        pos.advanceColumn(begin);
        filename = filename.substr(begin, end - begin);
        if (filename.empty()) {
            // report empty filename
            err.logError(Error_t::LL_ERROR, pos, "Missing file to include");
            return -1;
        }
        // decrement recursion level
        --level;
        // parse given file
        int ret = parse(filename, pos);
        // indecrement recursion level
        ++level;
        return ret;
    } else if (directive == "expand") {
        if (param == "yes") {
            expandValue = true;
            return 0;
        } else if (param == "no") {
            expandValue = false;
            return 0;
        } else {
            err.logError(Error_t::LL_ERROR, pos, ("Invalid value of expand '"
                                                  + param + "'."));
            return -1;
        }
    }
    // report unknown processing directive
    err.logError(Error_t::LL_ERROR, pos, "Unknown procesing directive");
    // indicate error
    return -1;
}

int Dictionary_t::dump(string &out) const {
    // dumm all records
    for (map<string, string>::const_iterator i = dict.begin();
         i != dict.end(); ++i) {
        out.append(i->first);
        out.append(": |");
        out.append(i->second);
        out.append("|\n----------------------------------------\n");
    }
    // OK
    return 0;
}
