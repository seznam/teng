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
 * $Id: tengerror.h,v 1.6 2011-01-19 06:39:45 burlog Exp $
 *
 * DESCRIPTION
 * Teng error handling class.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (vasek)
 *             Created.
 * 2006-06-21  (sten__)
 *             Removed error duplicities.
 */

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include "tengposition.h"
#include "tengerror.h"

namespace Teng {
namespace {

Pos_t to_pos_t(const Error_t::ErrorPos_t &pos) {
    return {
        pos.filename.empty()? nullptr: &pos.filename,
        pos.lineno,
        pos.colno
    };
}

const char *
translate(
    std::vector<std::pair<const void *, char *>> &filenames,
    const std::string *filename
) {
    static const char *empty_filename = "";
    if (!filename)
        return empty_filename;
    if (filename == Pos_t::no_filename())
        return empty_filename;
    for (auto item: filenames)
        if (item.first == filename)
            return item.second;
    filenames.push_back({filename, nullptr});
    filenames.back().second = strndup(filename->c_str(), filename->size());
    return filenames.back().second;
}

} // namespace

Error_t::~Error_t() noexcept {
    for (auto item: filenames)
        free(item.second);
}

std::string Error_t::Entry_t::getLogLine() const {
    std::ostringstream out;
    dump(out);
    out << '\n';
    return out.str();
}

void Error_t::Entry_t::dump(std::ostream &out) const {
    static const char *LS[] = {"Debug", "Warning", "Diag", "Error", "Fatal"};
    out << to_pos_t(pos) << " " << LS[level] << ": " << msg;
}

void Error_t::dump(std::ostream &out) const {
    for (auto &entry: getEntries())
        out << entry << std::endl;
}

std::vector<Error_t::Entry_t> Error_t::getEntries() const {
    using Record_t = decltype(records)::value_type;
    using Message_t = RecordValue_t::Message_t;

    // lambda converting record to entry
    auto make_entry = [] (const Record_t *record, const Message_t &msg) {
        return Entry_t{
            msg.level,
            {record->first.filename, record->first.lineno, record->first.colno},
            msg.text
        };
    };

    // lambda creating message for the number of ignored messages
    auto make_ignored = [] (const Record_t *record) {
        return Entry_t{
            Level_t::WARNING,
            {record->first.filename, record->first.lineno, record->first.colno},
            "The " + std::to_string(record->second.ignored)
            + " other error message(s) for this source code position "
            "have been ignored"
        };
    };

    // appends entry to log with according to source code positions
    auto push_back = [] (auto &&result, Entry_t &&entry) {
        result.push_back(std::move(entry));
        for (auto i = result.size() - 1; i > 0; --i) {
            auto &lhs = result[i - 1];
            auto &rhs = result[i];
            if (lhs.pos.filename == rhs.pos.filename) {
                if (lhs.pos.lineno < rhs.pos.lineno)
                    break;
                if (lhs.pos.lineno == rhs.pos.lineno) {
                    if (lhs.pos.colno < rhs.pos.colno)
                        break;
                    if (lhs.pos.colno == rhs.pos.colno) {
                        if (lhs.level <= rhs.level)
                            break;
                    }
                }
                std::swap(lhs, rhs);
            }
        }
    };

    // prepare list of sorted records
    std::vector<const Record_t *> ordered_records;
    ordered_records.resize(records.size());
    for (auto &record: records)
        ordered_records[record.second.record_order] = &record;

    // generate error entries according records order
    std::vector<Error_t::Entry_t> result;
    result.reserve(records.size() * max_messages_per_pos);
    for (auto *record: ordered_records) {
        // regular entries
        for (auto &msg: record->second.messages)
            push_back(result, make_entry(record, msg));
        if (record->second.ignored > 0)
            push_back(result, make_ignored(record));
    }

    // done
    return result;
}

/** Inserts entry to errors vector according to its position in source code.
 */
void Error_t::append_impl(
    Level_t level,
    const std::string *filename,
    int32_t lineno,
    int32_t colno,
    std::string msg
) {
    RecordKey_t key = {translate(filenames, filename), lineno, colno};

    // if no one error has been recorded for this position, insert new
    auto irecord = records.find(key);
    if (irecord == records.end()) {
        records.emplace(
            std::move(key),
            RecordValue_t{records.size(), 0, {{level, std::move(msg)}}}
        );
        return;
    }

    // ignore messages that are over limit
    auto &rec_value = irecord->second;
    if (rec_value.messages.size() >= max_messages_per_pos) {
        ++rec_value.ignored;
        return;
    }

    // append new message and push diags and warnings to the front
    auto i = rec_value.messages.size();
    do { /*here is at least one item in messages list, see code above*/
        if (rec_value.messages[--i].level >= level)
            break;
    } while (i > 0);

    // insert message only if the last one is not same
    auto iwhere = rec_value.messages.begin() + i;
    if (iwhere->text != msg) {
        // insert after the found place
        iwhere += 1;
        rec_value.messages.insert(iwhere, {level, std::move(msg)});
    }
}

} // namespace Teng

