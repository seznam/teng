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
 * $Id: tengfragmentstack.h,v 1.7 2006-06-13 10:04:16 vasek Exp $
 *
 * DESCRIPTION
 * Teng open fragments frames stack.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2004-05-10  (vasek)
 *             Created.
 * 2018-07-07  (burlog)
 *             Rewritten.
 */

#ifndef TENGOPENFRAMES_H
#define TENGOPENFRAMES_H

#include <string>

#include "tengerror.h"
#include "tengvalue.h"
#include "tengstructs.h"
#include "tengstringview.h"
#include "tengopenframesapi.h"

namespace Teng {

/** Represents position in some list.
 */
struct ListPos_t {
    explicit operator bool() const {return valid;}
    std::size_t i;    //!< position in list
    std::size_t size; //!< the list size
    bool valid;       //!< true if position points to some list
};

/** Converts negative index (like in python) to standard C array index.
 */
inline int64_t fix_negative_i(int64_t i, std::size_t max_i) {
    if (i >= 0) return i;
    if (-i > max_i) return max_i;
    return max_i + i;
}

/** Returns attribute for desired name, no matter of value it is.
 */
inline Value_t get_attr(const Fragment_t *frag, const string_view_t &name) {
    if (!frag)
        return Value_t();
    auto ivalue = frag->find(name);
    if (ivalue == frag->end())
        return Value_t();
    return Value_t(&ivalue->second);
}

/** Resolves the 'frag' value:
 *
 * tag::frag_ref - this is returned,
 * tag::list_ref - value built from i-th list item is returned,
 * other - nullptr is return.
 */
inline const Fragment_t *get_frag(const Value_t &self) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
        return nullptr;
    case Value_t::tag::frag_ref:
        return self.as_frag_ref().ptr;
    case Value_t::tag::list_ref:
        return (*self.as_list_ref().ptr)[self.as_list_ref().i].fragment();
    }
}

/** Resolves the 'value' of value:
 *
 * tag::frag_ref - this is returned,
 * tag::list_ref - value built from i-th list item is returned,
 * other - nullptr is return.
 */
inline Value_t get_value_at(const Value_t &self) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
        return Value_t();
    case Value_t::tag::frag_ref:
        return self;
    case Value_t::tag::list_ref:
        return Value_t(&(*self.as_list_ref().ptr)[self.as_list_ref().i]);
    }
}

/** Resolves the 'frag' value:
 *
 * tag::frag_ref - this is returned,
 * tag::list_ref of length one - value built 0-th list list item,
 * tag::list_ref of length other - nullptr is returned,
 * other - nullptr is returned.
 *
 * The implicit conversion of one-length-list to frags allows simplyfied dot
 * syntax of runtime variables e.g. "$$a.b.c" instead of e.g. "$$a[0].b[0].c".
 */
inline const Fragment_t *
get_lone_frag(const Value_t &self, std::size_t &ambiguous) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
        return nullptr;
    case Value_t::tag::frag_ref:
        return self.as_frag_ref().ptr;
    case Value_t::tag::list_ref:
        if (self.as_list_ref().ptr->size() == 1)
            return self.as_list_ref().ptr->begin()->fragment();
        ambiguous = self.as_list_ref().ptr->size();
        return nullptr;
    }
}

/** Returns i-th list item if self value contains list and nullptr otherwise.
 */
inline Value_t get_value_at(const Value_t &self, int64_t i) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
    case Value_t::tag::frag_ref:
        return Value_t();
    case Value_t::tag::list_ref:
        i = fix_negative_i(i, self.as_list_ref().ptr->size());
        if (i < self.as_list_ref().ptr->size())
            return Value_t(&(*self.as_list_ref().ptr)[i]);
        return Value_t();
    }
}

/** If the value holds the list then the method increments the list-ref
 * index of and returns true. If index points out of list range then the false
 * value is returned.
 */
inline bool move_to_next_list_item(Value_t &self) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
    case Value_t::tag::frag_ref:
        return false;
    case Value_t::tag::list_ref:
        return ++self.as_list_ref().i < self.as_list_ref().ptr->size();
    }
}

/** Returns index of frag in its parent list or udenfined if value does not
 * contain list.
 */
inline ListPos_t get_list_pos_impl(const Value_t &self) {
    switch (self.type()) {
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::regex:
    case Value_t::tag::frag_ref:
        return {0, 0, false};
    case Value_t::tag::list_ref:
        return {self.as_list_ref().i, self.as_list_ref().ptr->size(), true};
    }
}

/** The frame of open frags.
 */
struct FrameRec_t {
    FrameRec_t(const FragmentValue_t *root)
        : open_frags()
    {open_frags.emplace_back(root);}

    /** Returns true if fragment has been opened.
     */
    bool open_frag(const string_view_t &name) {
        Value_t new_frag = get_attr(get_frag(open_frags.back().frag), name);
        switch (new_frag.type()) {
        case Value_t::tag::frag_ref:
        case Value_t::tag::list_ref:
            open_frags.emplace_back(name, std::move(new_frag));
            return true;
        default:
            return false;
        }
    }

    /** Returns true if fragment
     */
    bool open_error_frag(FragmentList_t &&errors) {
        open_frags.emplace_back(std::move(errors));
        return true;
    }

    /** Stores error fragment in current open fragment. This is a hack that
     * ensures lifetime of the error frag until the current fragment is
     * closed.
     */
    void store_error_frag(FragmentList_t &&errors) {
        if (open_frags.back().error_frag)
            throw std::runtime_error(__PRETTY_FUNCTION__);
        open_frags.back().error_frag
            = std::make_unique<FragmentList_t>(std::move(errors));
    }

    /** Returns value with reference to error fragment stored in current open
     * fragment.
     */
    Value_t current_error_frag() const {
        return Value_t(open_frags.back().error_frag.get());
    }

    /** Returns true if next fragment has been opened.
     */
    bool next_frag() {
        if (move_to_next_list_item(open_frags.back().frag))
            return true;
        open_frags.pop_back();
        return false;
    }

    /** Appends name part to path if VarDesc_t contains it.
     */
    template <typename VarDesc_t>
    auto append_frag_name(std::string &result, const VarDesc_t *var) const
    -> decltype(static_cast<void>(var->name.data())) {
        result.push_back('.');
        result.append(var->name.data(), var->name.size());
    }

    /** Fallback for VarDesc_t without name.
     */
    void append_frag_name(std::string &result, ...) const {
        // ensure root frag dot
        if (result.empty())
            result.push_back('.');
    }

    /** Returns the path of the desired variable.
     */
    template <typename VarDesc_t>
    std::string path(const VarDesc_t &var) const {
        if (var.frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);

        // the open fragments part
        std::string result;
        uint16_t frag_i = open_frags.size() - var.frag_offset;
        for (uint16_t i = 1/*skip root frag*/; i < frag_i; ++i) {
            auto &frag_name = open_frags[i].name;
            result.push_back('.');
            result.append(frag_name.data(), frag_name.size());
        }

        // the variable part
        append_frag_name(result, &var);
        return result;
    }

    /** Returns the value of the desired variable or an undefined value.
     */
    template <typename VarDesc_t>
    Value_t get_var(const VarDesc_t &var) const {
        if (var.frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);

        // local variables overrides
        auto i = open_frags.size() - var.frag_offset - 1;
        if (auto *local_var = find_local(i, var.name))
            return *local_var;

        // regular variables
        return get_attr(get_frag(open_frags[i].frag), var.name);
    }

    /** Sets the value of the desired variable.
     */
    template <typename VarDesc_t>
    bool set_var(const VarDesc_t &var, Value_t &&value) {
        if (var.frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);

        // local values can't override fragment values
        auto i = open_frags.size() - var.frag_offset - 1;
        if (get_attr(get_frag(open_frags[i].frag), var.name))
            return false;

        // insert value
        auto &locals = open_frags[i].locals;
        auto ilocal_var = locals.find(var.name);
        if (ilocal_var == locals.end())
            locals.emplace(var.name, std::move(value));
        else ilocal_var->second = std::move(value);
        return true;
    }

    /** Returns local variable of desired name or nullptr.
     */
    const Value_t *find_local(uint16_t i, const string_view_t &name) const {
        auto ilocal_var = open_frags[i].locals.find(name);
        if (ilocal_var == open_frags[i].locals.end())
            return nullptr;
        return &ilocal_var->second;
    }

    /** Returns index of desired frag in fragment list.
     */
    template <typename VarDesc_t>
    ListPos_t get_list_pos(const VarDesc_t &var) const {
        if (var.frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = open_frags.size() - var.frag_offset - 1;
        return i == 0
            ? ListPos_t{0, 1, true} /*root frag (backward compatibility)*/
            : get_list_pos_impl(open_frags[i].frag);
    }

    /** Returns fragment (or anything else) at given offset.
     */
    Value_t value_at(uint16_t frag_offset) const {
        if (frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = open_frags.size() - frag_offset - 1;
        return Value_t(get_value_at(open_frags[i].frag));
    }

    /** Returns open fragments joined by dot.
     */
    std::string current_path() const {
        std::string result = ".";
        for (auto i = 1u; i < open_frags.size(); ++i) {
            if (i > 1) result.push_back('.');
            result.append(open_frags[i].name.data(), open_frags[i].name.size());
        }
        return result;
    }

    /** Returns current fragment index in parent list.
     */
    std::size_t current_list_i() const {
        return open_frags.size() == 1
            ? 0 // root fragment
            : get_list_pos_impl(open_frags.back().frag).i;
    }

    /** Returns size of the current fragmnet list.
     */
    std::size_t current_list_size() const {
        return open_frags.size() == 1
            ? 1 // root fragment
            : get_list_pos_impl(open_frags.back().frag).size;
    }

protected:
    // storage for local variables
    struct LocalCmp_t: std::less<string_view_t> {struct is_transparent {};};
    using Locals_t = std::map<std::string, Value_t, LocalCmp_t>;

    /** Record for open fragment.
     */
    struct FragRec_t {
        /** C'tor: for root frag.
         */
        FragRec_t(const FragmentValue_t *root)
            : frag(root), locals(), name(), error_frag()
        {}

        /** C'tor: for regular fragments.
         */
        FragRec_t(const string_view_t &name, Value_t frag)
            : frag(std::move(frag)), locals(), name(name), error_frag()
        {}

        /** C'tor: for error frag.
         */
        FragRec_t(FragmentList_t &&errors)
            : frag(), locals(), name("_error"),
              error_frag(std::make_unique<FragmentList_t>(std::move(errors)))
        {frag = Value_t(error_frag.get());}

        // shortucts
        using FragListPtr_t = std::unique_ptr<FragmentList_t>;

        Value_t frag;             //!< the current open frag
        Locals_t locals;          //!< local variables of frag
        string_view_t name;       //!< current frag name
        FragListPtr_t error_frag; //!< holds frag data from Error_t::getFrags
    };

    std::vector<FragRec_t> open_frags; //!< list of open fragments
};

/** This class represents runtime stack of open fragments by Teng frag
 * directive runtime. This is 2D structure because you can open root fragment
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
    // don't copy
    OpenFrames_t(const OpenFrames_t &) = delete;
    OpenFrames_t &operator=(const OpenFrames_t &) = delete;

    /** Used to indetified the variable for cases where the instruction does
     * not contain offsets.
     */
    struct VarDesc_t {
        VarDesc_t(const string_view_t &name): name(name) {}
        string_view_t name;
        uint32_t frame_offset = 0;
        uint32_t frag_offset = 0;
    };

    /** C'tor.
     */
    OpenFrames_t(const FragmentValue_t *root)
        : root(root), frames()
    {frames.emplace_back(root);}

    /** Returns the root fragment.
     */
    const FragmentValue_t *root_frag() const {return root;}

    /** Opens new frame.
     */
    void open_frame() {frames.emplace_back(root);}

    /** Close the most recent frame.
     */
    void close_frame() {frames.pop_back();}

    /** Returns true if fragment has been opened.
     */
    bool open_frag(const string_view_t &name) {
        return frames.back().open_frag(name);
    }

    /** Returns true if error fragment has been opened.
     */
    bool open_error_frag(FragmentList_t &&errors) {
        return frames.back().open_error_frag(std::move(errors));
    }

    /** Stores error fragment in current open fragment.
     */
    void store_error_frag(FragmentList_t &&errors) {
        frames.back().store_error_frag(std::move(errors));
    }

    /** Returns value with reference to error fragment stored in current open
     * fragment.
     */
    Value_t current_error_frag() const {
        return frames.back().current_error_frag();
    }

    /** Returns true if next fragment has been opened.
     */
    bool next_frag() {
        return frames.back().next_frag();
    }

    /** Returns path of desired variable.
     */
    template <typename VarDesc_t>
    std::string path(const VarDesc_t &var) const {
        if (var.frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - var.frame_offset - 1;
        return frames[i].path(var);
    }

    /** Returns the value of the desired variable or an undefined value.
     */
    template <typename VarDesc_t>
    Value_t get_var(const VarDesc_t &var) const {
        if (var.frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - var.frame_offset - 1;
        return frames[i].get_var(var);
    }

    /** Returns the value of the desired variable or an undefined value.
     */
    Value_t get_var(const VarDesc_t &var) const {
        return get_var<VarDesc_t>(var);
    }

    /** Sets the value of the desired variable.
     */
    template <typename VarDesc_t>
    bool set_var(const VarDesc_t &var, Value_t &&value) {
        if (var.frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - var.frame_offset - 1;
        return frames[i].set_var(var, std::move(value));
    }

    /** Returns index of desired frag in fragment list and list size.
     */
    template <typename VarDesc_t>
    ListPos_t get_list_pos(const VarDesc_t &var) const {
        if (var.frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - var.frame_offset - 1;
        return frames[i].get_list_pos(var);
    }

    /** Returns index of desired frag in fragment list and list size.
     */
    ListPos_t get_list_pos(const VarDesc_t &var) const {
        return get_list_pos<VarDesc_t>(var);
    }

    // *********************************************vvv open fragment frames API

    /** Returns frag at given offsets.
     */
    Value_t
    value_at(
        uint16_t frame_offset,
        uint16_t frag_offset
    ) const override {
        if (frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - frame_offset - 1;
        return frames[i].value_at(frag_offset);
    }

    /** Returns the value for desired name.
     */
    Value_t
    value_at(
        const Value_t &arg,
        const string_view_t &name,
        std::size_t &ambiguous
    ) const override {return get_attr(get_lone_frag(arg, ambiguous), name);}

    /** If the idx argument is numeric and arg is list then idx-th list item is
     * returned. Or if the idx argument is string and arg is convertible to
     * fragment then the value for the name stored in idx.
     */
    Value_t
    value_at(
        const Value_t &arg,
        const Value_t &idx,
        std::size_t &ambiguous
    ) const override {
        switch (idx.type()) {
        case Value_t::tag::undefined:
            return Value_t();
        case Value_t::tag::integral:
            return get_value_at(arg, idx.as_int());
        case Value_t::tag::real:
            return get_value_at(arg, idx.as_real());
        case Value_t::tag::string:
            return value_at(arg, idx.as_string(), ambiguous);
        case Value_t::tag::string_ref:
            return value_at(arg, idx.as_string_ref(), ambiguous);
        case Value_t::tag::frag_ref:
            return Value_t();
        case Value_t::tag::list_ref:
            return Value_t();
        case Value_t::tag::regex:
            return Value_t();
        }
    }

    /** Returns open fragments joined by dot.
     */
    std::string current_path() const override {
        return frames[0].current_path();
    }

    /** Returns current fragment index in parent list.
     */
    std::size_t current_list_i() const override {
        return frames[0].current_list_i();
    }

    /** Returns size of the current fragmnet list.
     */
    std::size_t current_list_size() const override {
        return frames[0].current_list_size();
    }

    /** Returns true if value exists.
     */
    Value_t exists(const Value_t &arg) const override {
        return Value_t(!arg.is_undefined());
    }

    // *********************************************^^^ open fragment frames API

protected:
    const FragmentValue_t *root;    //!< the fragments tree root
    std::vector<FrameRec_t> frames; //!< the list of open frames
};

} // namespace Teng

#endif /* TENGOPENFRAMES_H */

