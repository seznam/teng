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
 * $Id: tengdictionary.h,v 1.3 2005-11-20 11:11:41 vasek Exp $
 *
 * DESCRIPTION
 * Teng dictionary.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (vasek)
 *             Created.
 */


#ifndef TENGDICTIONARY_H
#define TENGDICTIONARY_H

#include <map>
#include <iosfwd>
#include <string>
#include <vector>

#include "tengerror.h"
#include "tengsourcelist.h"
#include "tengstringview.h"

namespace Teng {

/**
 * @short Dictionary -- mapping of string to string value.
 *
 * Used for language and parametric dictionaries.
 */
class Dictionary_t {
public:
    /**
     * @short Creates new dictionary object.
     *
     * @param fs_root path of root for locating files
     */
    Dictionary_t(const std::string &fs_root)
        : fs_root(fs_root), level(0), sources(), err(),
          expandValue(false), replaceValue(false)
    {}

    /**
     * @short Destroy dictionary object.
     */
    virtual ~Dictionary_t() = default;

    /**
     * @short Parses dicionary from given file.
     *
     * @param filename name of file to parse
     * @param include_pos the position of include directive if any
     * @return 0 OK !0 error
     */
    int parse(const std::string &filename, const Pos_t &include_pos = {});

    /**
     * @short Adds new entry into dictionary. Doesn't replace existing entry.
     *
     * @param name name of entry
     * @param value value of entry
     */
    virtual void add(const std::string &name, const std::string &value);

    /**
     * @short Searches for key in dictionary.
     *
     * @param key the key
     * @return found value or 0 when key not found
     */
    virtual const std::string *lookup(const std::string &key) const;

    /**
     * @short Check source files for change.
     *
     * @return 0 OK !0 changed
     */
    int isChanged() const {return sources.isChanged();}

    /**
     * @short Dumps dictionary into string. For debugging purposes.
     *
     * @param out output string
     */
    virtual void dump(std::string &out) const;

    /**
     * @short Dumps dictionary into stream. For debugging purposes.
     *
     * @param out output stream
     */
    virtual void dump(std::ostream &out) const;

    /**
     * @short Return source list.
     *
     * @return source list
     */
    const SourceList_t &getSources() const {return sources;}

    /**
     * @short Get error logger.
     *
     * @return error logger
     */
    const Error_t &getErrors() const {return err;}

protected:
    /**
     * @short Parses dictionary from given string. Worker function.
     *
     * @param data the input data.
     * @return 0 OK !0 error
     */
    virtual int parseString(string_view_t data);

    /**
     * @short Parses value line.
     *
     * @param line parsed line
     */
    virtual std::string parseValueLine(string_view_t line);

    /**
     * @short Parses line beginning with identifier.
     *
     * @param line parsed line
     */
    virtual std::string *addIdent(string_view_t line);

    /**
     * @short Parses and processes processing directive.
     *
     * @param directive whole directive string
     * @param param parameter to directive
     *
     * @return 0 OK !0 error
     */
    virtual int processDirective(string_view_t directive, string_view_t param);

    /**
     * @short Parses and processes processing directive.
     *
     * @param directive whole directive string
     * @param param parameter to directive
     *
     * @return 0 OK !0 error
     */
    virtual int processDirective(string_view_t line);

    /**
     * @short Adds new entry into dictionary.
     *
     * @param name name of entry
     * @param value value of entry
     *
     * @return pointer to inserted value.
     */
    virtual std::string *add(string_view_t name, string_view_t value);

    /**
     * @short Maximal number of dictionary file inclusion.
     */
    static const unsigned int MAX_RECURSION_LEVEL = 10;

    /**
     * @short Root directory for file lookup.
     */
    std::string fs_root;

    /**
     * @short Current level of recursion. Valid only when parsing.
     */
    unsigned int level;

    /**
     * @short Sources of this dictionary.
     */
    SourceList_t sources;

    /**
     * @short Error logger.
     */
    Error_t err;

    /** Position in source file.
     *
     * Valid only during parse.
     */
    Pos_t pos;

private:
    // don't copy
    Dictionary_t(const Dictionary_t &) = delete;
    Dictionary_t &operator=(const Dictionary_t &) = delete;

    /** Parses bool value and sets it if parsing succeeds.
     *
     * @param name name of entry
     * @param param value of entry
     * @param value value to set
     *
     * @return 0 OK !0 error
     */
    int setBool(string_view_t name, string_view_t param, bool &value);

    /** Includes file given by directive into dict.
     *
     * @param filename name of entry
     *
     * @return 0 OK !0 error
     */
    int includeFile(string_view_t filename);

    /**
     * @short Adds new entry into dictionary.
     *
     * @param name name of entry
     * @param value value of entry
     *
     * @return pointer to inserted value.
     */
    std::string *addImpl(string_view_t name, std::string value);

    /** Parses dicionary from given file. Worker function.
     *
     * @param file file open for reading
     * @param filename the name of the file
     *
     * @return 0 OK !0 error
     */
    int parse(std::ifstream &file, const std::string &filename);

    /** The dictionary itself.
     */
    std::map<std::string, std::string> dict;

    /** Flags whether #{name} is expanded in values.
     *
     * Valid only during parse.
     */
    bool expandValue;

    /** Flags whether replace values of already defined names.
     *
     * Valid only during parse.
     */
    bool replaceValue;
};

} // namespace Teng

#endif // TENGDICTIONARY_H
