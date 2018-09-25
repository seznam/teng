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

// TODO(burlog): remove
#include <iostream>

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
    // TODO(burlog): nejaky zpusob, jak hledat bez vytvareni stringu?
    auto ivalue = frag->find(name.str());
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
inline const Fragment_t *get_lone_frag(const Value_t &self) {
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
        : open_frags(1, {Value_t(root), {}, {}})
    {}

    /** Returns true if fragment has been opened.
     */
    bool open_frag(const string_view_t &name) {
        Value_t new_frag = get_attr(get_frag(open_frags.back().frag), name);
        switch (new_frag.type()) {
        case Value_t::tag::frag_ref:
        case Value_t::tag::list_ref:
            open_frags.push_back({std::move(new_frag), Locals_t(), name});
            return true;
        default:
            return false;
        }
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
            ? ListPos_t{0, 1, true} /*root frag*/
            : get_list_pos_impl(open_frags[i].frag);
    }

    /** Returns fragment (or anything else) at given offset.
     */
    Value_t frag(uint16_t frag_offset) const {
        if (frag_offset >= open_frags.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = open_frags.size() - frag_offset - 1;
        return open_frags[i].frag;
    }

protected:
    struct LocalCmp_t: std::less<string_view_t> {struct is_transparent {};};
    using Locals_t = std::map<std::string, Value_t, LocalCmp_t>;
    struct FragRec_t {Value_t frag; Locals_t locals; string_view_t name;};
    std::vector<FragRec_t> open_frags; //!< list of open fragments
};

// TODO(burlog): 
// class ErrorFragmentFrame_t : public FragmentFrame_t {
//     virtual Status_t
//     findVariable(const std::string &name, Parser::Value_t &var) const {
//         // try to match variable names
//         if (name == FILENAME) {
//             var = *errors[index].pos.filename;;
//             return S_OK;
//         } else if (name == LINE) {
//             var = errors[index].pos.lineno;
//             return S_OK;
//         } else if (name == COLUMN) {
//             var = errors[index].pos.colno;
//             return S_OK;
//         } else if (name == LEVEL) {
//             var = static_cast<int>(errors[index].level);
//             return S_OK;
//         } else if (name == MESSAGE) {
//             var = errors[index].msg;
//             return S_OK;
//         }
//
//         // nothing matched, return local variable
//         return findLocalVariable(name, var);
//     }
// const std::string ERROR_FRAG_NAME("_error");
// const std::string FILENAME("filename");
// const std::string LINE("line");
// const std::string COLUMN("column");
// const std::string LEVEL("level");
// const std::string MESSAGE("message");

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

    /** C'tor.
     */
    OpenFrames_t(const FragmentValue_t *root)
        : root(root), frames(1, FrameRec_t(root))
    {}

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

    // *********************************************vvv open fragment frames API

    /** Returns frag at given offsets.
     */
    Value_t frag(uint16_t frame_offset, uint16_t frag_offset) const override {
        if (frame_offset >= frames.size())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        auto i = frames.size() - frame_offset - 1;
        return frames[i].frag(frag_offset);
    }

    /** Returns the value for desired name.
     */
    Value_t frag_attr(const Value_t &arg, string_view_t name) const override {
        return get_attr(get_lone_frag(arg), name);
    }

    /** If the idx argument is numeric and arg is list then idx-th list item is
     * returned. Or if the idx argument is string and arg is convertible to
     * fragment then the value for the name stored in idx.
     */
    Value_t value_at(const Value_t &arg, const Value_t &idx) const override {
        switch (idx.type()) {
        case Value_t::tag::undefined:
            return Value_t();
        case Value_t::tag::integral:
            return get_value_at(arg, idx.as_int());
        case Value_t::tag::real:
            return get_value_at(arg, idx.as_real());
        case Value_t::tag::string:
            return frag_attr(arg, idx.as_string());
        case Value_t::tag::string_ref:
            return frag_attr(arg, idx.as_string_ref());
        case Value_t::tag::frag_ref:
            return Value_t();
        case Value_t::tag::list_ref:
            return Value_t();
        case Value_t::tag::regex:
            return Value_t();
        }
    }

    /** Returns 'representation' of the given value.
     */
    Value_t repr(const Value_t &arg) const override {
        // TODO(burlog): ma repr vubec pak smysl?
        // TODO(burlog): repr a escape?
        return arg;
    }

    /** Returns true if value exists.
     */
    Value_t exists(const Value_t &arg) const override {
        // #<{(|* Implementation of the teng 'exists' operator.
        //  |)}>#
        // Result_t exists(RunCtxPtr_t ctx) {
        //     throw std::runtime_error("not implemented yet");
        //     // auto &instr = ctx->instr->as<Exists_t>();
        //     // auto error_code = ctx->frag_stack.exists(instr.ident);
        //     // return Result_t(!error_code);
        // }
        return Value_t(!arg.is_undefined());
    }

    // *********************************************^^^ open fragment frames API

    // #<{(|* Implementation of the teng 'defined' operator.
    //  |)}>#
    // Result_t defined(RunCtxPtr_t ctx) {
    //     throw std::runtime_error(__PRETTY_FUNCTION__);
    //     // logWarning(*ctx, "The defined() operator is deprecated");
    //     // auto &instr = ctx->instr->as<Defined_t>();
    //     //
    //     // // no such variable and no such fragment found
    //     // auto error_code = ctx->frag_stack.exists(instr.ident);
    //     // if (error_code) return Result_t(false);
    //     //
    //     // // return false if fragment
    //     // Result_t tmp;
    //     // error_code = ctx->frag_stack.findVariable(instr.ident, tmp);
    //     // if (error_code) return Result_t(true);
    //     //
    //     // // ok
    //     // return tmp;
    // }

    // #<{(|* Implementation of the teng 'isempty' operator.
    //  |)}>#
    // Result_t isempty(RunCtxPtr_t ctx) {
    //     // TODO(burlog): make it
    //     throw std::runtime_error("----------!");
    // }

protected:
    const FragmentValue_t *root;;   //!< the fragments tree root
    std::vector<FrameRec_t> frames; //!< the list of open frames
};

} // namespace Teng

#endif /* TENGOPENFRAMES_H */

