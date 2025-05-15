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
 * $Id: tengcontenttype.cc,v 1.7 2008-02-22 07:15:51 burlog Exp $
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

#include <map>
#include <cctype>
#include <iomanip>
#include <cstring>
#include <utility>
#include <algorithm>

#include "contenttype.h"

namespace Teng {

// forward decls of all creators
std::unique_ptr<ContentType_t> htmlCreator();
std::unique_ptr<ContentType_t> shellCreator();
std::unique_ptr<ContentType_t> cCreator();
std::unique_ptr<ContentType_t> qstringCreator();
std::unique_ptr<ContentType_t> jshtmlCreator();
std::unique_ptr<ContentType_t> jsCreator();
std::unique_ptr<ContentType_t> jsonCreator();

namespace {

/**
 * @short Function which creates content type descriptor
 * @return content type descriptor
 */
using Creator_t = std::unique_ptr<ContentType_t> (*)();

/**
 * @short Entry in table of content type descriptor creating
 * functions.
 */
struct CreatorEntry_t {
    const char *name;    //!< name of content type
    const char *alias;   //!< name of content type
    Creator_t creator;   //!< content type descriptor creating function
    const char *comment; //!< comment for this content type
};

// creator array
static CreatorEntry_t creators[] = {
    {
        "text/html", "html", htmlCreator,
        "Hypertext markup language. Same processor as for"
        " 'text/xhtml' and 'text/xml'"
    }, {
        "text/xhtml", "xhtml", htmlCreator,
        "X hypertext markup language. Same processor as for"
        " 'text/xhtml' and 'text/xml'"
    }, {
        "text/xml", "xml", htmlCreator,
        "Extensible markup language. Same processor as for"
        " 'text/xhtml' and 'text/xml'"
    }, {
        "application/x-sh", "x-sh", shellCreator,
        "Common for all types of shell."
    }, {
        "text/csrc", "csrc", cCreator,
        "C/C++ source code"
    }, {
        "quoted-string", "quoted-string", qstringCreator,
        "Generic quoted string with escapes."
    }, {
        "jshtml", "jshtml", jshtmlCreator,
        "Quoted string embeddable into HTML pages."
    }, {
        "application/x-javascript", "js", jsCreator,
        "Javascript language."
    }, {
        "application/json", "json", jsonCreator,
        "Json."
    }, {
        nullptr, nullptr, nullptr, nullptr
    }
};

std::map<std::string, std::unique_ptr<ContentType_t::Descriptor_t>> descriptors;
std::vector<ContentType_t::Descriptor_t *> descriptorIndex;

ContentType_t::Descriptor_t *init_descriptors() {
    using Descriptor_t = ContentType_t::Descriptor_t;
    std::string comment = "Default (text/plain) type.";
    std::unique_ptr<Descriptor_t> descriptor;
    auto create_default = [] {return std::make_unique<ContentType_t>();};

    // create content type descriptor for text/plain
    std::string name = "text/plain";
    std::size_t i = descriptorIndex.size();
    descriptor.reset(new Descriptor_t{create_default(), i, name, comment});
    descriptorIndex.push_back(descriptor.get());
    descriptors.emplace(name, std::move(descriptor));

    // create content type alias descriptor for text/plain
    name = "text";
    i = descriptorIndex.size();
    descriptor.reset(new Descriptor_t{create_default(), i, name, comment});
    descriptorIndex.push_back(descriptor.get());
    descriptors.emplace(name, std::move(descriptor));

    // all the descriptors are pre-created...
    for (CreatorEntry_t *icreator = creators; icreator->name; ++icreator) {
        // shortcuts
        name = icreator->name;
        comment = icreator->comment;
        auto create = icreator->creator;

        // create content type descriptor
        i = descriptorIndex.size();
        descriptor.reset(new Descriptor_t{create(), i, name, comment});
        descriptorIndex.push_back(descriptor.get());
        descriptors.emplace(icreator->name, std::move(descriptor));

        // create content type alias descriptor
        i = descriptorIndex.size();
        descriptor.reset(new Descriptor_t{create(), i, name, comment});
        descriptorIndex.push_back(descriptor.get());
        descriptors.emplace(icreator->alias, std::move(descriptor));
    }

    return descriptorIndex.front();
}

ContentType_t::Descriptor_t *unknown = init_descriptors();

/**
 * @short State in unescaper automaton.
 */
struct UnescaperState_t {
    /**
     * @short Vector of next states.
     */
    using UnescaperStateVector_t = std::vector<UnescaperState_t>;
    using Pairs_t = std::vector<std::pair<int64_t, int64_t>>;

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
    int64_t nextState;

    /**
     * @short Next states.
     */
    UnescaperStateVector_t nextStates;

    /**
     * @short Add new next state.
     * @param o rule for new state
     * @return added state
     */
    UnescaperState_t &add(int o) {
        // try to find existing state
        for (auto &state: nextStates)
            if (state.rule == o)
                return state;

        // no state found, create it
        nextStates.push_back(UnescaperState_t(o));

        // return it
        return nextStates.back();
    }

    /**
     * @short Linearize automaton tree.
     */
    void linearize(Pairs_t &unescaper) const {
        // remember link holders
        std::vector<std::size_t> referrers;

        // run throgh next states and write rule for each
        for (auto &state: nextStates) {
            // remember position of link to next state
            referrers.push_back(unescaper.size());
            unescaper.emplace_back(state.rule, -state.nextState);
        }

        // put STOP indicator when we have any next state
        if (nextState == 0)
            unescaper.emplace_back(0, 0);

        // run through all rules
        auto ireferrers = referrers.begin();
        for (auto &state: nextStates) {
            if (!state.nextState) {
                // remember start of next state
                std::size_t pos = unescaper.size();
                // linearize this state
                state.linearize(unescaper);
                // assign link to sub state
                unescaper[*ireferrers].second = pos;
            }
            ++ireferrers;
        }
    }
};

} // namespace

ContentType_t::ContentType_t()
    : lineComment(), blockComment(), escapes(), unescaper()
{
    // set escape bitmap to all -1 (character not escaped)
    auto *end = escapeBitmap + 256;
    auto *i = escapeBitmap;
    while (i != end) *i++ = -1;

    // create empty automaton
    unescaper.emplace_back(0, 0);
}

int64_t ContentType_t::addEscape(unsigned char c, const std::string &escape) {
    // if escape already present in bitmap make it error
    if (escapeBitmap[c] != -1) return -1;

    // add escape entry
    escapes.emplace_back(c, escape);

    // update entry in escape bitmap
    return escapeBitmap[c] = escapes.size() - 1;
}

std::string ContentType_t::escape(const string_view_t &src) const {
    // output string
    std::string dest;
    dest.reserve(src.size());

    // run through input string
    for (auto ch: src) {
        // find entry in bitmap
        auto pos = escapeBitmap[static_cast<unsigned char>(ch)];
        // if position is negative pass source character to output
        if (pos < 0) dest.push_back(ch);
        // else append escape sequence
        else dest.append(escapes[pos].second);
    }

    // return output
    return dest;
}

std::string ContentType_t::unescape(const string_view_t &src) const {
    // output string
    std::string dest;
    dest.reserve(src.size());

    // run through input string
    for (auto isrc = src.begin(); isrc != src.end();) {
        // help iterator
        auto bsrc = isrc;
        // index in automaton
        int64_t state = 0;
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
            dest.push_back(static_cast<char>(-state));
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

int64_t ContentType_t::nextState(unsigned char c, int64_t state) const {
    // stop when state outside automaton
    if ((std::size_t(state) >= unescaper.size()) || (state < 0))
        return 0;

    // find rule to move to next state
    for (auto i = unescaper.begin() + state; i->first > 0; ++i)
        if (i->first == c)
            return i->second;

    // no rule => stop
    return 0;
}

void ContentType_t::compileUnescaper() {
    // destroy current automaton
    unescaper.clear();

    // start state of automaton
    UnescaperState_t root;

    // run through escape definitions
    for (auto &escape: escapes) {
        // start state
        UnescaperState_t *state = &root;

        // escape sequence
        const std::string &str = escape.second;

        // run through escape sequence
        for (char ch: str)
            state = &state->add(ch);

        // assign unescaped character as final rule
        state->nextState = escape.first;
    }

    // linearize automaton into vector
    root.linearize(unescaper);
}

/** @short Create descriptor of HTML/XHTML/XML content type.
 * @return HTML descriptor
 */
std::unique_ptr<ContentType_t> htmlCreator() {
    // create HTML descriptor
    auto html = std::make_unique<ContentType_t>();

    // HTML has only block comments
    html->blockComment = {"<!--", "-->"};

    // and has these (necessary) escapes
    html->addEscape('&', "&amp;");
    html->addEscape('<', "&lt;");
    html->addEscape('>', "&gt;");
    html->addEscape('"', "&quot;");

    // compile unescaping automaton
    html->compileUnescaper();
    return html;
}

/** @short Create descriptor of shell.
 * @return shell descriptor
 */
std::unique_ptr<ContentType_t> shellCreator() {
    // create SHELL descriptor
    auto shell = std::make_unique<ContentType_t>();

    // SHELL has only line comment
    shell->lineComment = "#";

    // return descriptor
    return shell;
}

/** @short Create descriptor of C language.
 * @return C descriptor
 */
std::unique_ptr<ContentType_t> cCreator() {
    // create C descriptor
    auto c = std::make_unique<ContentType_t>();

    // C has only block comments
    c->blockComment = {"/*", "*/"};

    // return descriptor
    return c;
}

/** @short Create descriptor of quoted string.
 * @return quoted string descriptor
 */
std::unique_ptr<ContentType_t> qstringCreator() {
    // create quoted-string descriptor
    auto qs = std::make_unique<ContentType_t>();

    qs->addEscape('\\', "\\\\");
    qs->addEscape('\n', "\\n");
    qs->addEscape('\r', "\\r");
    qs->addEscape('\a', "\\a");
    qs->addEscape('\0', "\\0");
    qs->addEscape('\v', "\\v");
    qs->addEscape('\'', "\\'");
    qs->addEscape('"', "\\\"");

    // compile unescaping automaton
    qs->compileUnescaper();

    // return descriptor
    return qs;
}

/** @short Create descriptor of quoted string.
 * @return quoted string descriptor
 */
std::unique_ptr<ContentType_t> jshtmlCreator() {
    // create quoted-string descriptor
    auto jshtml = std::make_unique<ContentType_t>();

    jshtml->addEscape('\\', "\\\\");
    jshtml->addEscape('\n', "\\n");
    jshtml->addEscape('\r', "\\r");
    jshtml->addEscape('\a', "\\a");
    jshtml->addEscape('\0', "\\0");
    jshtml->addEscape('\v', "\\v");
    jshtml->addEscape('\'', "\\'");
    jshtml->addEscape('"', "\\&quot;");
    jshtml->addEscape('&', "&amp;");
    jshtml->addEscape('<', "&lt;");
    jshtml->addEscape('>', "&gt;");

    // compile unescaping automaton
    jshtml->compileUnescaper();

    // return descriptor
    return jshtml;
}

/** @short Create descriptor of quoted string.
 * @return quoted string descriptor
 */
std::unique_ptr<ContentType_t> jsCreator() {
    // create quoted-string descriptor
    auto js = std::make_unique<ContentType_t>();

    js->addEscape('\\', "\\\\");
    js->addEscape('\n', "\\n");
    js->addEscape('\r', "\\r");
    js->addEscape('\a', "\\a");
    js->addEscape('\0', "\\0");
    js->addEscape('\v', "\\v");
    js->addEscape('\'', "\\'");
    js->addEscape('"', "\\\"");
    js->addEscape('/', "\\/");

    // compile unescaping automaton
    js->compileUnescaper();

    // return descriptor
    return js;
}

std::unique_ptr<ContentType_t> jsonCreator() {
    // create quoted-string descriptor
    auto js = std::make_unique<ContentType_t>();

    js->addEscape('"', "\\\"");
    js->addEscape('\\', "\\\\");
    js->addEscape('\b',"\\b");
    js->addEscape('\f',"\\f");
    js->addEscape('\n',"\\n");
    js->addEscape('\r',"\\r");
    js->addEscape('\t',"\\t");
    js->addEscape('/',"\\/");

    for (unsigned char i = 0; i <= 0x1F; ++i) {
        std::stringstream ss;
        ss << "\\u" << std::hex << std::uppercase
           <<  std::setfill('0') << std::setw(4) << std::hex << i;
        js->addEscape(i, ss.str());
    }

    // compile unescaping automaton
    js->compileUnescaper();

    // return descriptor
    return js;
}

const ContentType_t::Descriptor_t *ContentType_t::getDefault() {
    return unknown;
}

const ContentType_t::Descriptor_t *
ContentType_t::find(const string_view_t &name_view) {
    // make name lower
    std::string name = name_view.str();
    std::transform(name.begin(), name.end(), name.begin(), tolower);

    // try to find cached content type descriptor
    auto idescriptor = descriptors.find(name);
    return idescriptor == descriptors.end()
        ? nullptr
        : idescriptor->second.get();
}

std::vector<std::pair<std::string, std::string>>
ContentType_t::listSupported() {
    std::vector<std::pair<std::string, std::string>> result;
    for (CreatorEntry_t *icreators = creators; icreators->name; ++icreators) {
        auto comment = icreators->comment? icreators->comment: "";
        result.push_back(std::make_pair(icreators->name, comment));
    }
    return result;
};

void Escaper_t::pop() {
    if (!escapers.empty()) escapers.pop();
}

} // namespace Teng
