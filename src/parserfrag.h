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

#include "position.h"
#include "identifier.h"
#include "openframesapi.h"

namespace Teng {

// forwards
class Program_t;

namespace Parser {

/** The struct representing the open frag.
 */
struct FragRec_t {
    const string_view_t &name() const {return token.view();}
    Token_t token;      //!< level 2 token
    int64_t addr;       //!< the address of Frag_t instruction
    bool auto_close;    //!< if frag was created by <?teng frag ...?>
};

/** The frame of open frags.
 */
class FrameRec_t {
public:
    // types
    using OpenFragments_t = std::vector<FragRec_t>;
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
    void open_frag(const Token_t &token, int64_t addr, bool auto_close) {
        if (frags.size() >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the number of open frags exceeded 65535");
        frags.push_back({token, addr, auto_close});
    }

    /** Close the most recent frag.
     */
    FragRec_t close_frag() {
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
            [] (auto &&lhs, auto &&rhs) {return lhs.name() == rhs.view();}
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
            [] (auto &&lhs, auto &&rhs) {return lhs.name() == rhs.view();}
        );

        // and return
        return irfrag == frags.rend()
            ? frags.end()
            : irfrag.base() - ident.size() + 1;
    }

    /** Returns open fragment at desired index.
     */
    const FragRec_t &operator[](int64_t i) const {return frags[i];}

    /** Returns open fragments joined by dot.
     */
    std::string current_path() const {
        std::string result = ".";
        for (auto i = 1u; i < frags.size(); ++i) {
            if (i > 1) result.push_back('.');
            result.append(frags[i].name().data(), frags[i].name().size());
        }
        return result;
    }

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
    using OpenFramesImpl_t = std::vector<FrameRec_t>;
    using iterator = OpenFramesImpl_t::iterator;
    using const_iterator = OpenFramesImpl_t::const_iterator;
    using reverse_iterator = OpenFramesImpl_t::reverse_iterator;
    using const_reverse_iterator = OpenFramesImpl_t::const_reverse_iterator;

    /** C'tor.
     */
    OpenFrames_t(Program_t &program)
        : program(program), frames(1, FrameRec_t())
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

    /** Returns frag at given offsets.
     */
    Value_t
    value_at(uint64_t frame_offset, uint64_t frag_offset) const override {
        if (frame_offset >= frames.size())
            throw runtime_ctx_needed_t();
        if (frag_offset >= frames[frames.size() - frame_offset - 1].size())
            throw runtime_ctx_needed_t();
        return Value_t(make_frag_ident(frame_offset, frag_offset));
    }

    /** Returns index i+1 which identifies next open fragment but only if
     * desired name match open fragment name. If name does not match open
     * fragment then runtime_ctx_needed_t is thrown which tells the optimalizer
     * that expression is unoptimizable.
     */
    Value_t
    value_at(
        const Value_t &offsets,
        const string_view_t &name,
        std::size_t &
    ) const override {
        return evaluate(
            offsets,
            [&] (auto &frag_rec, uint64_t frame_offset, uint64_t frag_offset) {
                if (frag_rec.name() != name)
                    throw runtime_ctx_needed_t();
                return Value_t(make_frag_ident(frame_offset, ++frag_offset));
            }
        );
    }

    /** If the idx argument is zero then it is ignored because of implicit list
     * to frag conversion. If the idx argument is string and the string equals
     * to i-th frag name then the index to next frag is returned.
     */
    Value_t
    value_at(
        const Value_t &offsets,
        const Value_t &idx,
        std::size_t &ambiguous
    ) const override {
        if (offsets.is_undefined())
            throw runtime_ctx_needed_t();
        switch (idx.type()) {
        case Value_t::tag::undefined:
            throw runtime_ctx_needed_t();
        case Value_t::tag::integral:
            if (idx.as_int() == 0)
                return offsets;
            throw runtime_ctx_needed_t();
        case Value_t::tag::real:
            if (idx.as_real() == 0)
                return offsets;
            throw runtime_ctx_needed_t();
        case Value_t::tag::string:
            return value_at(offsets, idx.string(), ambiguous);
        case Value_t::tag::string_ref:
            return value_at(offsets, idx.string(), ambiguous);
        case Value_t::tag::frag_ref:
            throw runtime_ctx_needed_t();
        case Value_t::tag::list_ref:
            throw runtime_ctx_needed_t();
        case Value_t::tag::regex:
            throw runtime_ctx_needed_t();
        }
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    /** Returns open fragments joined by dot.
     */
    std::string current_path() const override {
        return frames[0].current_path();
    }

    /** Returns current fragment index in parent list.
     */
    std::size_t current_list_i() const override {
        throw runtime_ctx_needed_t();
    }

    /** Returns size of the current fragmnet list.
     */
    std::size_t current_list_size() const override {
        throw runtime_ctx_needed_t();
    }

    /** Returns true if there is open fragment at given index.
     */
    Value_t exists(const Value_t &offsets) const override {
        if (offsets.is_undefined())
            throw runtime_ctx_needed_t();
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
    FrameRec_t &top() {return frames.back();}

    /** Returns i-th frame.
     */
    const FrameRec_t &operator[](std::size_t i) const {return frames[i];}

    /** Returns the most recent frame of open frags.
     */
    const FrameRec_t &top() const {return frames.back();}

    /** Opens new frame of open frags. The frame does not contain any open frag.
     */
    FrameRec_t &open_frame() {
        if (frames.size() >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the number of open frames exceeded 65535");
        frames.emplace_back();
        return frames.back();
    }

    /** Closes the most recent frame of open frags.
     */
    void close_frame() {frames.pop_back();}

protected:
    /** Joins frame and frag offsets to one "big" int.
     */
    int64_t make_frag_ident(uint64_t frame_offset, uint64_t frag_offset) const {
        if (frame_offset >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the frame offset is greater than 65535");
        if (frag_offset >= std::numeric_limits<uint16_t>::max())
            throw std::length_error("the frag offset is greater than 65535");
        return (frame_offset << 16) | frag_offset;
    }

    /** Splits frame offset from "big" int.
     */
    uint16_t make_frame_offset(int64_t frag_ident) const {
        return static_cast<uint16_t>((frag_ident >> 16) | 0xffff);
    }

    /** Splits frag offset from big int.
     */
    uint16_t make_frag_offset(int64_t frag_ident) const {
        return static_cast<uint16_t>(frag_ident | 0xffff);
    }

    /** Calls desired callback with valid frame and frag offsets.
     */
    template <typename call_t>
    Value_t evaluate(const Value_t &offsets, call_t &&call) const {
        if (offsets.is_undefined())
            throw runtime_ctx_needed_t();
        uint16_t frame_offset = make_frame_offset(offsets.as_int());
        if (frame_offset >= frames.size())
            throw runtime_ctx_needed_t();
        auto frame_i = frames.size() - frame_offset - 1;
        uint16_t frag_offset = make_frag_offset(offsets.as_int());
        if (frag_offset >= frames[frame_i].size())
            throw runtime_ctx_needed_t();
        auto frag_i = frames[frame_i].size() - frag_offset - 1;
        return call(frames[frame_i][frag_i], frame_offset, frag_offset);
    }

    Program_t &program;      //!< program created by parser
    OpenFramesImpl_t frames; //!< frames of open fragments
};

} // namespace Parser
} // namespace Teng

#endif /* TENGPARSERFRAG_H */

