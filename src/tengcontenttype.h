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
 * $Id: tengcontenttype.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng content type descriptor.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-24  (vasek)
 *             Created.
 * 2003-10-01  (vasek)
 *             We are using mime-types as content type name.
 */

#ifndef TENGCONTENTTYPE_H
#define TENGCONTENTTYPE_H

#include <stack>
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "tengerror.h"
#include "tengstringview.h"

namespace Teng {

/**
 * @short Describes content type.
 *
 * This descriptor is used for escaping/unescaping strings and for
 * commenting pieces of output out.
 */
class ContentType_t {
public:
    /** @short Create new empty descriptor.
     */
    ContentType_t();

    /** @short Add escape mapping into escaping table.
     * @param c character
     * @param escape associated escape sequence
     * @return position in escape list or -1 when escape for given
     *         character already present
     */
    int addEscape(unsigned char c, const std::string &escape);

    /** @short Compile unescaping automaton from escaping list.
     */
    void compileUnescaper();

    /** @short Escape given string.
     * @param src string to escape
     * @return escaped string
     */
    std::string escape(const string_view_t &src) const;

    /** @short Unescape given string.
     * @param src string to unescape
     * @return unescaped string
     */
    std::string unescape(const string_view_t &src) const;

    /** @short Descriptor of content type.
     */
    struct Descriptor_t {
        using ptr_t = std::unique_ptr<ContentType_t>;
        ptr_t contentType;       //!< the content type escaper
        unsigned int index;      //!< index in list of content types
        std::string name;        //!< the content type name
        std::string description; //!< the description text
    };

    /**
     * @short Find content type descriptor for given name.
     * @param name name of content type
     * @return descriptor od 0 on error
     */
    static const Descriptor_t *find(const string_view_t &name_view);

    /**
     * @short Get default content type.
     * @return default content type descriptor
     */
    static const Descriptor_t *getDefault();

    /**
     * @short Lists supported content types.
     * @param supported list of supported content types.
     */
    static std::vector<std::pair<std::string, std::string>> listSupported();

    /** @short String used for commenting out line.
     */
    std::string lineComment;

    /** @short Start of commend and end of comment.
     */
    std::pair<std::string, std::string> blockComment;

private:
    /**
     * @short List of escape rules.
     */
    std::vector<std::pair<unsigned char, std::string>> escapes;

    /**
     * @short Map of indices to escape list (-1 -> no escape).
     */
    int escapeBitmap[256];

    /**
     * @short Unescaping automaton.
     */
    std::vector<std::pair<int, int>> unescaper;

    /**
     * @short Moves to next state of automaton.
     * @param c characted being matched
     * @param state current state
     * @return new +state or -character or 0 (on no match)
     */
    int nextState(unsigned char c, int state) const;
};

class Escaper_t {
public:
    /** @short Creates new escaper with given or default content type.
     *
     * @param ct first content type
     */
    inline Escaper_t(const ContentType_t *ct = nullptr)
        : escapers()
    {
        topLevel = (ct? ct: ContentType_t::getDefault()->contentType.get());
        escapers.push(topLevel);
    }

    /** @short Push new content type.
     *
     * @short ct content type
     */
    void push(const ContentType_t *ct) {escapers.push(ct? ct: escapers.top());}

    /** @short Pop content type from the top.
     */
    void pop();

    /** @short Returns the number of escapers.
     */
    std::size_t size() const {return escapers.size();}

    /** @short Escape given string.
     *
     * Uses escaper on the top of the stack.
     *
     * @param src string to escape
     * @return escaped string
     */
    std::string escape(const string_view_t &src) const {
        return escapers.top()->escape(src);
    }

    /** @short Unescape given string.
     *
     * Uses escaper on the top of the stack.
     *
     * @param src string to unescape
     * @return unescaped string
     */
    std::string unescape(const string_view_t &src) const {
        return escapers.top()->unescape(src);
    }

    /** @short Returns the top level content type.
     */
    const ContentType_t *getTopLevel() const {return topLevel;}

private:
    /** @short Stack of un/escaperers.
     */
    std::stack<const ContentType_t *> escapers;

    /** @short Toplevel escapert.
     */
    const ContentType_t *topLevel;
};

} // namespace Teng

#endif // TENGCONTENTTYPE_H

