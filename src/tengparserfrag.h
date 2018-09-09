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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser context.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2018-07-07  (burlog)
 *             Extracted from tengparsercontext.cc.
 */

#ifndef TENGPARSERFRAG_H
#define TENGPARSERFRAG_H

#include <string>
#include <vector>
#include <algorithm>

#include "tengposition.h"
#include "tengidentifier.h"
#include "tengopenframesapi.h"

namespace Teng {

// forwards
class Program_t;

namespace Parser {

/** The struct representing the open frag.
 */
struct OpenFragment_t {
    string_view_t name; //!< the fragment local name
    int32_t addr;       //!< the address of Frag_t instruction
    bool auto_close;    //!< if frag was created by <?teng frag ...?>
};

/** The frame of open frags.
 */
class OpenFrame_t {
public:
    // types
    using OpenFragments_t = std::vector<OpenFragment_t>;
    using const_iterator = OpenFragments_t::const_iterator;

    /** Returns the number of open fragments.
     */
    std::size_t size() const {return frags.size();}

    /** Returns true if there is no open fragments in this frame.
     */
    bool empty() const {return frags.empty();}

    /** Returns const iterator to the first frame of open frags.
     */
    const_iterator begin() const {return frags.begin();}

    /** Returns const iterator one past the last frame of open frags.
     */
    const_iterator end() const {return frags.end();}

    /** Opens new fragment.
     */
    void open_frag(string_view_t name, int32_t addr, bool auto_close) {
        if (frags.size() >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the number of open frags exceeded 65535");
        frags.push_back({name, addr, auto_close});
    }

    /** Close the most recent frag.
     */
    OpenFragment_t close_frag() {
        auto result = frags.back();
        frags.pop_back();
        return result;
    }

    /** Returns true if the list open fragments is prefix of given list of
     * fragment "names". Note, that empty list is prefix of every idents.
     */
    bool is_prefix_of(const Identifier_t &ident) {
        return (size() < ident.size()) && std::equal(
            frags.begin(), frags.end(),
            ident.begin(), ident.begin() + size(),
            [] (auto &&lhs, auto &&rhs) {return lhs.name == rhs;}
        );
    }

    /** Attempts to identify the relative ident in the list of open
     * fragments. The relative list of "names" are searched in backward
     * order in list of open frags and the first match is returned.
     * Note, that the last item of ident is variable name so it has to be
     * omitted from matching. The ident with one or less items is
     * considered as no-match.
     */
    auto resolve_relative(const Identifier_t &ident) {
        // empty ident path is automatically matched (omit var name)
        if (ident.size() <= 1)
            return frags.end();

        // if ident path is longer than opened fragments (omit var name)
        if (frags.size() < (ident.size() - 1))
            return frags.end();

        // search for ident in backward order
        auto irfrag = std::search(
            frags.rbegin(), frags.rend(),
            ++ident.rbegin(), ident.rend(), // omit var name
            [] (auto &&lhs, auto &&rhs) {return lhs.name == rhs;}
        );

        // and return
        return irfrag == frags.rend()
            ? frags.end()
            : irfrag.base() - ident.size() + 1;
    }

    /** Returns open fragment at desired index.
     */
    const OpenFragment_t &operator[](int64_t i) const {return frags[i];}

protected:
    OpenFragments_t frags; //!< the list of open frags
};

/** This class represents stack of open fragments by Teng frag directive during
 * parsing of template. This is 2D structure because you can open root fragment
 * again and again. See this example:
 *
 * <?teng frag frag1?>  <-- base frame [index=0] (it will never be popped out)
 * <?teng frag frag2?>
 * <?teng frag .frag1?> <-- next frame [index=1]
 * ${var}
 * <?teng endfrag?>     <-- popping this frag causes poping current frame
 * <?teng endfrag?>
 * <?teng endfrag?>     <-- base frame is kept
 *
 * where within frag2 is root child frag1 again opened. As long as it is not
 * closed, it is necessary to maintain new path of open fragments.
 */
class OpenFrames_t: public OFFApi_t {
public:
    // types
    using OpenFramesImpl_t = std::vector<OpenFrame_t>;
    using iterator = OpenFramesImpl_t::iterator;
    using const_iterator = OpenFramesImpl_t::const_iterator;
    using reverse_iterator = OpenFramesImpl_t::reverse_iterator;
    using const_reverse_iterator = OpenFramesImpl_t::const_reverse_iterator;

    /** C'tor.
     */
    OpenFrames_t(Program_t &program)
        : program(program), frames(1, OpenFrame_t())
    {}

    /** D'tor.
     */
    ~OpenFrames_t();

    // *********************************************vvv open fragment frames API
    //
    // This API is used by some instructions (Repr_t, ...) during optimization
    // phase to access open fragment frames. For sake of that, the expressions
    // like <?teng frag a?>${exist(a)}<?teng endfrag?> can be optimalized to
    // scalar value without any additional code.
    //
    // We are using indices to open fragments list as fragment identifiers.

    /** Returns zero which is index to root frag.
     */
    Value_t root_frag() const override {return Value_t(0);}

    /** Returns zero which is index to root frag.
     */
    Value_t this_frag() const override {return Value_t(top().size() - 1);}

    /** Returns index i+1 which identifies next open fragment but only if
     * desired name match open fragment name. If name does not match open
     * fragment then runtime_ctx_needed_t is thrown which tells the optimalizer
     * that expression is unoptimizable.
     */
    Value_t frag_attr(const Value_t &i, string_view_t name) const override {
        if (i.is_undefined())
            throw runtime_ctx_needed_t();
        if (i.as_int() >= top().size())
            throw runtime_ctx_needed_t();
        if (top()[i.as_int()].name != name)
            throw runtime_ctx_needed_t();
        return Value_t(i.as_int() + 1);
    }

    /** If the idx argument is zero then it is ignored because of implicit list
     * to frag conversion. If the idx argument is string and the string equals
     * to i-th frag name then the index to next frag is returned.
     */
    Value_t value_at(const Value_t &i, const Value_t &idx) const override {
        if (i.is_undefined())
            throw runtime_ctx_needed_t();
        switch (idx.type()) {
        case Value_t::tag::undefined:
            throw runtime_ctx_needed_t();
        case Value_t::tag::integral:
            if (idx.as_int() == 0)
                return i;
            throw runtime_ctx_needed_t();
        case Value_t::tag::real:
            if (idx.as_real() == 0)
                return i;
            throw runtime_ctx_needed_t();
        case Value_t::tag::string:
            if (i.as_int() < top().size())
                if (top()[i.as_int()].name == idx.as_string())
                    return Value_t(i.as_int() + 1);
            throw runtime_ctx_needed_t();
        case Value_t::tag::string_ref:
            if (i.as_int() < top().size())
                if (top()[i.as_int()].name == idx.as_string_ref())
                    return Value_t(i.as_int() + 1);
            throw runtime_ctx_needed_t();
        case Value_t::tag::frag_ref:
            throw runtime_ctx_needed_t();
        case Value_t::tag::list_ref:
            throw runtime_ctx_needed_t();
        case Value_t::tag::regex:
            throw runtime_ctx_needed_t();
        }
    }

    /** Returns 'representation' of the open fragments at given index.
     */
    Value_t repr(const Value_t &i) const override {
        if (i.is_undefined()) throw runtime_ctx_needed_t();
        if ((i.as_int() + 1) < top().size())
            return Value_t("$frag-or-list$");
        throw runtime_ctx_needed_t();
    }

    /** Returns true if there is open fragment at given index.
     */
    Value_t exists(const Value_t &i) const override {
        if (i.is_undefined()) throw runtime_ctx_needed_t();
        return Value_t(true);
    }

    // *********************************************^^^ open fragment frames API

    /** Returns the number of frames.
     */
    std::size_t size() const {return frames.size();}

    /** Returns const iterator to the first frame of open frags.
     */
    const_iterator begin() const {return frames.begin();}

    /** Returns const iterator one past the last frame of open frags.
     */
    const_iterator end() const {return frames.end();}

    /** Returns mutable iterator to the first frame of open frags.
     */
    iterator begin() {return frames.begin();}

    /** Returns mutable iterator one past the last frame of open frags.
     */
    iterator end() {return frames.end();}

    /** Returns const reverse iterator to the first frame of open frags.
     */
    const_reverse_iterator rbegin() const {return frames.rbegin();}

    /** Returns const reverse iterator one past the last frame of open frags.
     */
    const_reverse_iterator rend() const {return frames.rend();}

    /** Returns mutable reverse iterator to the first frame of open frags.
     */
    reverse_iterator rbegin() {return frames.rbegin();}

    /** Returns mutable reverse iterator one past the last frame of open frags.
     */
    reverse_iterator rend() {return frames.rend();}

    /** Returns the most recent frame of open frags.
     */
    OpenFrame_t &top() {return frames.back();}

    /** Returns i-th frame.
     */
    const OpenFrame_t &operator[](std::size_t i) const {return frames[i];}

    /** Returns the most recent frame of open frags.
     */
    const OpenFrame_t &top() const {return frames.back();}

    /** Opens new frame of open frags. The frame does not contain any open frag.
     */
    OpenFrame_t &open_frame() {
        if (frames.size() >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the number of open frames exceeded 65535");
        frames.emplace_back();
        return frames.back();
    }

    /** Closes the most recent frame of open frags.
     */
    void close_frame() {frames.pop_back();}

protected:
    Program_t &program;      //!< program created by parser
    OpenFramesImpl_t frames; //!< frames of open fragments
};

} // namespace Parser
} // namespace Teng

#endif /* TENGPARSERFRAG_H */

