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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-19  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-14  (burlog)
 *             Cleaned.
 */

#include <sstream>
#include <fstream>
#include <algorithm>

#ifdef DEBUG
#include <iostream>
#endif /* DEBUG */

#include "aux.h"
#include "logging.h"
#include "platform.h"
#include "dictionary.h"

namespace Teng {
namespace {

/** Stripts whitespace characters.
 */
string_view_t strip(const string_view_t &str) {
    const char *begin = str.begin();
    const char *end = str.end();
    while ((begin < end) && isspace(*begin)) ++begin;
    while ((begin < end) && isspace(*(end - 1))) --end;
    return {begin, end};
}

/** Stripts whitespace characters from left.
 */
string_view_t lstrip(const string_view_t &str) {
    const char *begin = str.begin();
    const char *end = str.end();
    while ((begin < end) && isspace(*begin)) ++begin;
    return {begin, end};
}

/**
 * @short Extracts line from string.
 *
 * Removes terminating <LF> and (optional) preceding <CR>.
 *
 * @return line and position of terminating <LF> in input string
 */
std::pair<string_view_t, const char *>
getLine(
    Error_t &err,
    const Pos_t &pos,
    const string_view_t &str,
    const char *iline
) {
    // search for next newline
    auto eline = std::find(iline, str.end(), '\n');

    // no new line found
    if (eline == str.end()) {
        Pos_t eol_pos = pos;
        eol_pos.advanceColumn(eline - iline + 1);
        logWarning(err, eol_pos, "No newline at end of file");
        return  {{iline, eline}, eline};
    }

    // trim '\r' if any
    if ((iline != str.begin()) && (*(iline - 1) == '\r'))
        return {{iline, eline - 1}, eline};

    // done
    return  {{iline, eline}, eline};
}

/** Returns true if given interval contains whitespaces only.
 */
bool contains_spaces_only(const char *ivalue, const char *evalue) {
    for (; ivalue != evalue; ++ivalue)
        if (!isspace(*ivalue))
            return false;
    return true;
}

/** The dict format parser.
 */
struct DictParser_t {
    /** C'tor.
     */
    DictParser_t(
        Dictionary_t *dict,
        Error_t &err,
        SourceList_t &sources,
        const FilesystemInterface_t *filesystem,
        bool &expandVars,
        string_view_t filename
    ): dict(dict), err(err), sources(sources), filesystem(filesystem), pos(),
       expandVars(expandVars), include_level(0)
    {load_file(filename, Pos_t(/*base level, no include reference*/));}

    /** C'tor: for included dicts.
     */
    DictParser_t(
        DictParser_t &parent,
        string_view_t filename,
        const Pos_t &incl_pos
    ): dict(parent.dict), err(parent.err), sources(parent.sources),
       filesystem(parent.filesystem), expandVars(parent.expandVars),
       include_level(parent.include_level + 1)
    {load_file(filename, incl_pos);}

    void load_file(string_view_t filename, const Pos_t &incl_pos);

    /** Starts parsing the loaded data.
     */
    template <typename type1_t, typename type2_t>
    void parse(type1_t new_directive, type2_t new_entry);

    /** Prepares new directive and call insertion callback.
     */
    template <typename type1_t, typename type2_t>
    void process_directive(
        type1_t new_directive,
        type2_t new_entry,
        string_view_t line
    );

    /** Prepares new dict entry and call insertion callback.
     */
    template <typename type_t>
    std::string *process_entry(type_t new_entry, string_view_t line);

    /** Parses value and expands any variables.
     */
    std::string parse_entry_value(string_view_t value, Pos_t pos);

    /** Expands variables (if any) in given string.
     */
    std::string expand_value(string_view_t value, Pos_t value_pos);

    /** Opens file and process it.
     */
    template <typename type1_t, typename type2_t>
    void include_file(
        type1_t new_directive,
        type2_t new_entry,
        string_view_t filename,
        Pos_t incl_pos
    );

    Dictionary_t *dict;     //!< dict with yet parsed values
    Error_t &err;           //!< the error log
    SourceList_t &sources;  //!< source files of dictionary entries
    const FilesystemInterface_t *filesystem;   //!< the filesystem
    Pos_t pos;              //!< position in the dict file
    std::string data;       //!< the dict file data
    bool &expandVars;       //!< expand variables in dict values
    uint32_t include_level; //!< the include level
};

void DictParser_t::load_file(string_view_t filename, const Pos_t &incl_pos) {
    auto path = filename.str();
    try {
        data = filesystem->read(path);
        // insert source into source list and setup file position
        pos = {sources.push(filesystem, path).first, 1, 0};
    } catch (const std::exception &e) {
        logError(
            err,
            incl_pos,
            "Error reading file '" + path + "' "
            + "(" + e.what() + ")"
        );
    }
}

template <typename type1_t, typename type2_t>
void DictParser_t::parse(type1_t new_directive, type2_t new_entry) {
    string_view_t line;
    std::string *last_inserted_value = nullptr;
    auto iline = data.data(), eline = data.data() + data.size();

    // increments file pos in d'tor
    struct pos_updater_t {
        ~pos_updater_t() {pos.advanceColumn(line_size);}
        std::size_t line_size;
        Pos_t &pos;
    };

    // process whole input line by line and skip empty lines
    for (; iline != eline; pos.newLine(), ++iline) {
        std::tie(line, iline) = getLine(err, pos, data, iline);
        pos_updater_t pos_updater{line.size(), pos};
        if (line.empty()) continue;

        // what kind of line is it
        switch (line[0]) {
        case '#':
            // skip comments
            last_inserted_value = nullptr;
            continue;

        case '%':
            // it is directive
            process_directive(new_directive, new_entry, line);
            last_inserted_value = nullptr;
            break;

        case ' ': case '\t': case '\v':
            // continuation of previous dict entry value
            if (last_inserted_value) {
                string_view_t value_view = lstrip({line.begin(), line.end()});
                auto spaces = std::distance(line.begin(), value_view.begin());
                Pos_t value_pos = pos;
                pos.advanceColumn(spaces);
                std::string value = parse_entry_value(value_view, value_pos);
                last_inserted_value->push_back(' ');
                last_inserted_value->append(value);
            } else logWarning(err, pos, "No dict entry to concatenate with");
            break;

        case '.': case '_':
        case 'a' ... 'z':
        case 'A' ... 'Z':
            // dict entry
            last_inserted_value = process_entry(new_entry, line);
            break;

        default:
            last_inserted_value = nullptr;
            logWarning(err, pos, "Illegal identifier: line=" + line.str());
        }
    }
}

template <typename type1_t, typename type2_t>
void DictParser_t::process_directive(
    type1_t new_directive,
    type2_t new_entry,
    string_view_t line
) {
    // space splits name and value
    auto iline = line.begin() + 1; // skip leading '%' character
    auto ivalue = std::find_if(iline, line.end(), isspace);

    // prepare directive name and value
    string_view_t name = {iline, ivalue};
    string_view_t value = strip({ivalue, line.end()});
    auto name_pos = pos, value_pos = pos;
    value_pos.advanceColumn(std::distance(iline, value.begin()));

    // call insertion callback
    if (name != "include)") new_directive(name, value, name_pos, value_pos);
    else include_file(new_directive, new_entry, value, value_pos);
}

template <typename type_t>
std::string *DictParser_t::process_entry(type_t new_entry, string_view_t line) {
    // get all valid chars (assumes that first char is not number)
    auto ivalue = line.begin();
    for (auto eline = line.end(); ivalue != eline; ++ivalue)
        if (!(isalnum(*ivalue) || (*ivalue == '_')))
            break;

    // if first non-valid char is not white, report it as error
    if ((ivalue != line.end()) && (!isspace(*ivalue))) {
        Pos_t tmp_pos = pos;
        tmp_pos.advanceColumn(std::distance(line.begin(), ivalue));
        logWarning(err, tmp_pos, "Invalid character in identifier");
        return nullptr;
    }

    // prepare views to entry identifier and entry value
    string_view_t name = {line.begin(), ivalue};
    string_view_t value_view = lstrip({ivalue, line.end()});
    Pos_t value_pos = pos;
    value_pos.advanceColumn(std::distance(line.begin(), value_view.begin()));

    // parse rest of line as value and insert new entry to dict
    return new_entry(name.str(), parse_entry_value(value_view, value_pos));
}

std::string
DictParser_t::parse_entry_value(string_view_t value, Pos_t value_pos) {
    // reserve necessary space
    std::string result;
    result.reserve(value.size());
    auto ivalue = value.begin();

    // creates pos from value_pos and given value iterator
    auto make_pos = [&] (const char *ivalue) {
        Pos_t tmp_pos = value_pos;
        tmp_pos.advanceColumn(std::distance(value.begin(), ivalue));
        return tmp_pos;
    };

    // process whole string char by char
    bool quoted = false;
    for (auto evalue = value.end(); ivalue != evalue; ++ivalue) {
        switch (*ivalue) {

        // escape sequence
        case '\\':
            if (++ivalue == evalue) {
                logWarning(err, make_pos(ivalue), "EOL escaping not allowed");
                result.push_back('\\');
            }
            // escape -> interpret next char
            switch (*ivalue) {
            case 'n':
                result.push_back('\n');
            case 'r':
                result.push_back('\r');
            case 't':
                result.push_back('\t');
            case 'v':
                result.push_back('\v');
            case '\\':
                result.push_back('\\');
            case '"':
                result.push_back('"');
            default:
                // other chars are not allowed to be escaped
                logWarning(err, make_pos(ivalue), "Invalid escape sequence");
                result.push_back(*ivalue);
            }
            break;

        // quote
        case '"':
            if (ivalue == value.begin()) {
                quoted = true;
                break;
            } else if (quoted) {
                if (!contains_spaces_only(++ivalue, evalue)) {
                    logWarning(
                        err,
                        make_pos(ivalue),
                        "Text after closing quote"
                    );
                }
                return expandVars? expand_value(result, value_pos): result;
            }
            result.push_back(*ivalue);
            break;

        // push to value
        default:
            result.push_back(*ivalue);
            break;
        }
    }

    // if value is still quoted => quote not closed
    if (quoted) logWarning(err, make_pos(value.end()), "Missing closing quote");
    return expandVars? expand_value(result, value_pos): result;
}

std::string DictParser_t::expand_value(string_view_t value, Pos_t value_pos) {
    // expand value
    std::string expanded;
    expanded.reserve(value.size());

    // creates pos from value_pos and given value iterator
    auto make_pos = [&] (const char *ivalue) {
        Pos_t tmp_pos = value_pos;
        tmp_pos.advanceColumn(std::distance(value.begin(), ivalue));
        return tmp_pos;
    };

    // expand all variables
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
            logWarning(err, make_pos(value.end()), "Unterminated #{} variable");
            expanded.append(iopen, value.end());
            return expanded;
        }

        // compose dict value key
        string_view_t key = {iopen + 2, iclose};
        ivalue = iclose + 1;

        // try to find key in so far parsed entries
        if (const std::string *dict_value = dict->lookup(key)) {
            expanded.append(*dict_value);

        } else {
            expanded.append("#{");
            expanded.append(key.begin(), key.end());
            expanded.push_back('}');
            logWarning(
                err,
                make_pos(iopen),
                "Dict item '" + key + "' not found"
            );
        }
    }
    return expanded;
}

template <typename type1_t, typename type2_t>
void DictParser_t::include_file(
    type1_t new_directive,
    type2_t new_entry,
    string_view_t filename,
    Pos_t incl_pos
) {
    // include other source
    if (include_level > 10) {
        auto level = std::to_string(include_level);
        logError(err, incl_pos, "Too many includes: " + level);
        return;
    }

    // strip trailing whitespaces
    auto filename_view = strip(filename);
    auto spaces = std::distance(filename.begin(), filename_view.begin());
    incl_pos.advanceColumn(spaces);

    // check filename
    if (!filename_view.empty()) {
        DictParser_t parser(*this, filename, incl_pos);
        parser.parse(new_directive, new_entry);
    } else logWarning(err, incl_pos, "Missing filename to include");
}

} // namespace

const std::string *Dictionary_t::lookup(const string_view_t &key) const {
    auto ientry = entries.find(key);
    return ientry == entries.end()
         ? (key == "_tld"? &get_tld(): nullptr)
         : &ientry->second;
}

void Dictionary_t::dump(std::string &out) const {
    std::ostringstream os;
    dump(os);
    out.append(os.str());
}

void Dictionary_t::dump(std::ostream &out) const {
    for (auto &entry: entries) {
        out << entry.first << ": |" << entry.second
            << "|\n----------------------------------------\n";
    }
}

std::string *
Dictionary_t::new_entry(const std::string &name, const std::string &value) {
    return replaceEntries
        ? &(entries[name] = value)
        : &entries.emplace(name, value).first->second;
}

Dictionary_t::error_code
Dictionary_t::new_directive(
    const char *name_ptr, std::size_t name_len,
    const char *value_ptr, std::size_t value_len
) {
    string_view_t name = {name_ptr, name_len};
    string_view_t value = {value_ptr, value_len};

    // turn feature on or off
    auto enable_feature = [&] (bool &feature_value) {
        if (value == "yes") {feature_value = true; return error_code::none;}
        if (value == "no") {feature_value = false; return error_code::none;}
        if (value == "on") {feature_value = true; return error_code::none;}
        if (value == "off") {feature_value = false; return error_code::none;}
        if (value == "true") {feature_value = true; return error_code::none;}
        if (value == "false") {feature_value = false; return error_code::none;}
        return error_code::invalid_bool;
    };

    // set known features
    if (name == "expand") return enable_feature(expandVars);
    if (name == "replace") return enable_feature(replaceEntries);

    // report failure (unknown directive/feature)
    return error_code::unknown_directive;
}

void Dictionary_t::parse(const std::string &filename) {
    DictParser_t parser{this, err, sources, filesystem.get(), expandVars, filename};
    parser.parse(
        [&] (auto &&name, auto &&value, Pos_t name_pos, Pos_t value_pos) {
            auto result = this->new_directive(
                name.data(), name.size(),
                value.data(), value.size()
            );
            switch (result) {
            case error_code::none:
                break;
            case error_code::invalid_number:
                logWarning(
                    err,
                    value_pos,
                    "Invalid numeric value of " + name
                    + " directive '" + value + "'"
                );
                break;
            case error_code::invalid_bool:
                logWarning(
                    err,
                    value_pos,
                    "Invalid bool value of " + name
                    + " directive '" + value + "'; choose one of "
                    "{yes, no, on, off, true, false}"
                );
                break;
            case error_code::invalid_enable:
                logWarning(
                    err,
                    value_pos,
                    "You can't enable unknown '" + value + "' feature"
                );
                break;
            case error_code::invalid_disable:
                logWarning(
                    err,
                    value_pos,
                    "You can't disable unknown '" + value + "' feature"
                );
                break;
            case error_code::unknown_directive:
                logWarning(err, name_pos, "Unknown procesing directive");
                break;
            }

        }, [&] (const std::string &name, const std::string &value) {
            return new_entry(name, value);
        }
    );
#ifdef DEBUG
    std::cerr << "Dictionary or configuration file " << filename << " parsed."
              << std::endl;
#endif /* DEBUG */
}

} // namespace Teng

