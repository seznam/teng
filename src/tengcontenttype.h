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

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <stack>
#include <memory>

#include "tengerror.h"

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

    /** @short Destroy descriptor.
     */
    inline virtual ~ContentType_t() {}

    /** @short String used for commenting out line.
     */
    std::string lineComment;

    /** @short Start of commend and end of comment.
     */
    std::pair<std::string, std::string> blockComment;

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
    virtual std::string escape(const std::string &src) const;

    /** @short Unescape given string.
     * @param src string to unescape
     * @return unescaped string
     */
    virtual std::string unescape(const std::string &src) const;

    /** @short Descriptor of content type.
     */
    struct Descriptor_t {
        Descriptor_t(unsigned int index,
                     const std::string &name,
                     const std::string &description)
            : contentType(std::make_unique<ContentType_t>()),
              index(index), name(name), description(description)
        {}

        template <typename Creator_t>
        Descriptor_t(Creator_t &&creator, unsigned int index)
            : contentType(creator.creator()),
              index(index), name(creator.name), description(creator.comment)
        {}

        std::unique_ptr<ContentType_t> contentType;
        unsigned int index;
        std::string name;
        std::string description;
    };

    /**
     * @short Find content type descriptor for given name.
     * @param name name of content type
     * @param err error log
     * @param failOnError when true return 0 on error;
     *                    otherwise return default (no-op) desriptor
     * @return descriptor od 0 on error
     */
    static const Descriptor_t*
    findContentType(const std::string &name, Error_t &err,
                    const Error_t::Position_t &pos = Error_t::Position_t(),
                    bool failOnError = false);

    static const Descriptor_t* getContentType(unsigned int index);

    /**
     * @short Get default content type.
     * @return default content type descriptor
     */
    static const Descriptor_t* getDefault();

    /**
     * @short Lists supported content types.
     * @param supported list of supported content types.
     */
    static void listSupported(
            std::vector<std::pair<std::string, std::string> > &supported);

private:
    /**
     * @short List of escape rules.
     */
    std::vector<std::pair<unsigned char, std::string> > escapes;

    /**
     * @short Map of indices to escape list (-1 -> no escape).
     */
    int escapeBitmap[256];

    /**
     * @short Unescaping automaton.
     */
    std::vector<std::pair<int, int> > unescaper;

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
    inline Escaper_t(const ContentType_t *ct = 0)
        : escapers()
    {
        topLevel = (ct ? ct : ContentType_t::getDefault()->contentType.get());
        escapers.push(topLevel);
    }

    /** @short Push new content type.
     *
     * @short ct content type
     */
    void push(ContentType_t *ct);

    /** @short Push new content type.
     *
     * @short index index of content type descriptor
     * @short index err error log
     * @short index pos position in source file
     */
    void push(unsigned int index, Error_t &err,
              const Error_t::Position_t &pos = Error_t::Position_t());

    /** @short Pop content type from the top.
     *
     * @short index err error log
     * @short index pos position in source file
     */
    void pop(Error_t &err,
             const Error_t::Position_t &pos = Error_t::Position_t());

    /** @short Escape given string.
     *
     * Uses escaper on the top of the stack.
     *
     * @param src string to escape
     * @return escaped string
     */
    inline std::string escape(const std::string &src) const {
        return escapers.top()->escape(src);
    }

    /** @short Unescape given string.
     *
     * Uses escaper on the top of the stack.
     *
     * @param src string to unescape
     * @return unescaped string
     */
    inline std::string unescape(const std::string &src) const {
        return escapers.top()->unescape(src);
    }

    inline const ContentType_t* getTopLevel() const {
        return topLevel;
    }

private:
    /** @short Stack of un/escaperers.
     */
    std::stack<const ContentType_t*> escapers;

    /** @short Toplevel escapert.
     */
    const ContentType_t *topLevel;
};

} // namespace Teng

#endif // TENGCONTENTTYPE_H
