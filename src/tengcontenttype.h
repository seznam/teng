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
 * $Id: tengcontenttype.h,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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

#ifndef _TENGCONTENTTYPE_H
#define _TENGCONTENTTYPE_H

#include <string>
#include <utility>
#include <vector>
#include <map>

using namespace std;

namespace Teng {

/**
 * @short Describes content type.
 *
 * This descriptor is used for escaping/unescaping strings and for
 * commenting pieces of output out.
 */
class ContentType_t {
public:
    /**
     * @short Create new empty descriptor.
     */
    ContentType_t();

    /**
     * @short String used for commenting out line.
     */
    string lineComment;

    /**
     * @short Start of commend and end of comment.
     */
    pair<string, string> blockComment;

    /**
     * @short Add escape mapping into escaping table.
     * @param c character
     * @param escape associated escape sequence
     * @return position in escape list or -1 when escape for given
     *         character already present
     */
    int addEscape(unsigned char c, const string &escape);

    /**
     * @short Compile unescaping automaton from escaping list.
     */
    void compileUnescaper();

    /**
     * @short Escape given string.
     * @param src string to escape
     * @return escaped string
     */
    string escape(const string &src) const;

    /**
     * @short Unescape given string.
     * @param src string to unescape
     * @return unescaped string
     */
    string unescape(const string &src) const;

    /**
     * @short Find content type descriptor for given name.
     * @param name name of content type
     * @return descriptor
     */
    static const ContentType_t* findContentType(const string &name);

    /**
     * @short Lists supported content types.
     * @param supported list of supported content types.
     */
    static void listSupported(vector<pair<string, string> > &supported);

private:
    /**
     * @short List of escape rules.
     */
    vector<pair<unsigned char, string> > escapes;

    /**
     * @short Map of indices to escape list (-1 -> no escape).
     */
    int escapeBitmap[256];

    /**
     * @short Unescaping automaton.
     */
    vector<pair<int, int> > unescaper;

    /**
     * @short Moves to next state of automaton.
     * @param c characted being matched
     * @param state current state
     * @return new +state or -character or 0 (on no match)
     */
    int nextState(unsigned char c, int state) const;
};

} // namespace Teng

#endif // _TENGCONTENTTYPE_H
