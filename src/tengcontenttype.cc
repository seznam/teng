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
 * $Id: tengcontenttype.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Teng language descriptor -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-24  (vasek)
 *             Created.
 */

#include <string.h>
#include <utility>
#include <algorithm>
#include <ctype.h>

#include "tengcontenttype.h"

using namespace std;

using namespace Teng;

static map<string, ContentType_t*> *contentTypes = 0;
static ContentType_t* unknown = 0;

ContentType_t::ContentType_t()
    : lineComment(), blockComment(), escapes(),
      unescaper()
{
    // set escape bitmap to all -1 (character not escaped)
    int *end = escapeBitmap + 256;
    int *i = escapeBitmap;
    while (i != end) *i++ = -1;
    // create empty automaton
    unescaper.push_back(pair<int, int>(0, 0));
}

int ContentType_t::addEscape(unsigned char c, const string &escape) {
    // if escape already present in bitmap make it error
    if (escapeBitmap[c] != -1) return -1;
    // add escape entry
    escapes.push_back(pair<unsigned char, string>(c, escape));
    // update entry in escape bitmap
    return escapeBitmap[c] = escapes.size() - 1;
}

string ContentType_t::escape(const string &src) const {
    // output string
    string dest;
    dest.reserve(src.length());

    // run through input string
    for (string::const_iterator isrc = src.begin();
         isrc != src.end(); ++isrc) {
        // find entry in bitmap
        int pos = escapeBitmap[static_cast<unsigned char>(*isrc)];
        // if position is negative pass source character to output
        if (pos < 0) dest.push_back(*isrc);
        // else append escape sequence
        else dest.append(escapes[pos].second);
    }
    // return output
    return dest;
}

string ContentType_t::unescape(const string &src) const {
    // output string
    string dest;
    dest.reserve(src.length());

    // run through input string
    for (string::const_iterator isrc = src.begin();
         isrc != src.end(); ) {
        // help iterator
        string::const_iterator bsrc = isrc;
        // index in automaton
        int state = 0;
        // run through remaining characters
        for (; bsrc != src.end(); ++bsrc) {
            // move to next state
            state = nextState(*bsrc, state);
            // we stop here if state is not positive
            if (state <= 0) break;
        }

        // check final state
        if (state < 0) {
            // state is negative => it's negated character!
            dest.push_back(-state);
            // move after so far eaten escape sequence
            isrc = bsrc + 1;
        } else {
            // sequence not matcher pass input verbatim to output
            dest.push_back(*isrc++);
        }
    }
    // return output
    return dest;
}

int ContentType_t::nextState(unsigned char c, int state) const {
    // stop when state outside automaton 
    if ((static_cast<unsigned int>(state) >= unescaper.size()) ||
        (state < 0)) return 0;

    // iterator to state
    vector<pair<int, int> >::const_iterator i = unescaper.begin() + state;

    // find rule to move to next state
    for (; i->first > 0; ++i)
        if (i->first == c)
            return i->second;
    // no rule => stop
    return 0;
}

/**
 * @short State in unescaper automaton.
 */
struct UnescaperState_t {
    /**
     * @short Vector of next states.
     */
    typedef vector<UnescaperState_t> UnescaperStateVector_t;

    /**
     * @short Create new state.
     * @param rule matched character
     */
    UnescaperState_t(int rule = 0)
        : rule(rule), nextState(0)
    {}
    
    /**
     * @short Matched character.
     */
    int rule;

    /**
     * @short Next state of automaton (positive) or replacement
     *        character (negative) or stop state (zero).
     */
    int nextState;
    
    /**
     * @short Next states.
     */
    UnescaperStateVector_t nextStates;
   
    /**
     * @short Add new next state.
     * @param o rule for new state
     * @return added state
     */
    UnescaperState_t& add(int o) {
        // try to find existing state
        for (UnescaperStateVector_t::iterator i = nextStates.begin();
             i != nextStates.end(); ++i) {
            // rule found => return it
            if (i->rule == o) return *i;
        }
        // no state found, create it
        nextStates.push_back(UnescaperState_t(o));
        // return it
        return nextStates.back();
    }

    /**
     * @short Linearize automaton tree.
     */
    void linearize(vector<pair<int, int> > &unescaper) const {
        // remember link holders
        vector<int> referrers;
        // run throgh next states and write rule for each
        for (UnescaperStateVector_t::const_iterator i = nextStates.begin();
             i != nextStates.end(); ++i) {
            // remember position of link to next state
            referrers.push_back(unescaper.size());
            // create state
            unescaper.push_back(pair<int, int>(i->rule, -i->nextState));
        }
        // put STOP indicator when we have any next state
        if (nextState == 0)
            unescaper.push_back(pair<int, int>(0, 0));
        // run through all rules
        vector<int>::const_iterator ireferrers = referrers.begin();
        for (UnescaperStateVector_t::const_iterator i = nextStates.begin();
             i != nextStates.end(); ++i, ++ireferrers) {
            if (!i->nextState) {
                // remember start of next state
                int pos = unescaper.size();
                // linearize this state
                i->linearize(unescaper);
                // assign link to sub state
                unescaper[*ireferrers].second = pos;
            }
        }
    }

};

void ContentType_t::compileUnescaper() {
    // destroy current automaton
    unescaper.clear();

    // start state of automaton
    UnescaperState_t root;

    // run through escape definitions
    for (vector<pair<unsigned char, string> >::const_iterator
             iescapes = escapes.begin();
         iescapes != escapes.end(); ++iescapes) {
        // start state
        UnescaperState_t *state = &root;
        // escape sequence
        const string &str = iescapes->second;
        // run through escape sequence
        for (string::const_iterator istr = str.begin();
             istr != str.end(); ++istr) {
            // add next state
            state = &state->add(*istr);
        }
        // assign unescaped character as final rule
        state->nextState = iescapes->first;
    }
    // linearize automaton into vector
    root.linearize(unescaper);
}

/**
 * @short Function which creates content type descriptor
 * @return content type descriptor
 */
typedef ContentType_t* (*Creator_t)();

/**
 * @short Entry in table of content type descriptor creating
 * functions.
 */
struct CreatorEntry_t {
    /**
     * @short name of content type
     */
    const char *name;

    /**
     * @short content type descriptor creating function
     */
    Creator_t creator;

    /**
     * @short comment for this content type
     */
    const char *comment;
};

/**
 * @short Create descriptor of HTML/XHTML/XML content type.
 * @return HTML descriptor
 */
ContentType_t* htmlCreator() {
    // create HTML descriptor
    ContentType_t *html = new ContentType_t();
    // HTML has only block comments
    html->blockComment = pair<string, string>("<!--", "-->");
    // and has these (necessary) escapes
    html->addEscape('&', "&amp;");
    html->addEscape('<', "&lt;");
    html->addEscape('>', "&gt;");
    html->addEscape('"', "&quot;");
    // compile unescaping automaton
    html->compileUnescaper();
    // return descriptor
    return html;
}

/**
 * @short Create descriptor of shell.
 * @return shell descriptor
 */
ContentType_t* shellCreator() {
    // create SHELL descriptor
    ContentType_t *shell = new ContentType_t();
    // SHELL has only line comment
    shell->lineComment = "#";
    // return descriptor
    return shell;
}

/**
 * @short Create descriptor of C language.
 * @return C descriptor
 */
ContentType_t* cCreator() {
    // create C descriptor
    ContentType_t *c = new ContentType_t();
    // C has only block comments
    c->blockComment = pair<string, string>("/*", "*/");
    // return descriptor
    return c;
}

static CreatorEntry_t creators[] = {
    { "text/html", htmlCreator,
      "Hypertext markup language. Same processor as for"
      " 'text/xhtml' and 'text/xml'" },
    { "text/xhtml", htmlCreator,
      "X hypertext markup language. Same processor as for"
      " 'text/xhtml' and 'text/xml'" },
    { "text/xml", htmlCreator,
      "Extensible markup language. Same processor as for"
      " 'text/xhtml' and 'text/xml'" },
    { "application/x-sh", shellCreator,
      "Common for all types of shell." },
    { "text/csrc", cCreator,
      "C/C++ source code" },
    { 0, 0 }
};

const ContentType_t*
ContentType_t::findContentType(const string &_name) {
    // make name lower
    string name = _name;
    transform(name.begin(), name.end(), name.begin(),
              tolower);

    // if no cache create it
    if (!contentTypes) contentTypes = new map<string, ContentType_t*>();
    // if no unknown content type place holder create it
    if (!unknown) unknown = new ContentType_t();
    // try to find cached content type descriptor
    map<string, ContentType_t*>::const_iterator
        fcontentTypes = contentTypes->find(name);
    // if no content descriptor found
    if (fcontentTypes == contentTypes->end()) {
        // run through creator table and try to find appropriate
        // creator
        for (CreatorEntry_t *icreators = creators;
             icreators->name; ++icreators) {
            // if creator found
            if (icreators->name == name) {
                // create content type descriptor
                ContentType_t* contentType = icreators->creator();
                // remember it in cache for future use
                contentTypes->insert(pair<string, ContentType_t*>
                                  (name, contentType));
                // return it to the caller
                return contentType;
            }
        }
        // content type not known
        // return global desriptor for unknown types
        return unknown;
    }

    // return cached entry
    return fcontentTypes->second;
};

void ContentType_t::listSupported(vector<pair<string, string> > &supported) {
    for (CreatorEntry_t *icreators = creators;
         icreators->name; ++icreators)
        supported.push_back(make_pair(icreators->name,
                                      (icreators->comment ? icreators->comment
                                      : string())));
};
