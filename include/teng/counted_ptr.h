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
 * $Id: tengvalue.h,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng variables type.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             reference counting ptr
 */

#include <memory>

#ifndef TENGCOUNTED_PTR_H
#define TENGCOUNTED_PTR_H

namespace Teng {

/** It's similar to shared_ptr but does not provide any thread safety and can
 * release the held pointer.
 */
template <typename type_t>
class counted_ptr {
public:
    /** C'tor: copy.
     */
    counted_ptr(const counted_ptr &other) noexcept
        : ptr(other.ptr), refs(other.refs)
    {++*refs;}

    /** C'tor: move.
     */
    counted_ptr(counted_ptr &&other) noexcept
        : ptr(std::exchange(other.ptr, nullptr)),
          refs(std::exchange(other.refs, nullptr))
    {}

    /** Assigment: copy.
     */
    counted_ptr &operator=(const counted_ptr &other) noexcept {
        if (this != &other) {
            reset();
            new (this) counted_ptr(other);
        }
        return *this;
    }

    /** Assigment: move.
     */
    counted_ptr &operator=(counted_ptr &&other) noexcept {
        if (this != &other) {
            reset();
            new (this) counted_ptr(std::move(other));
        }
        return *this;
    }

    /** D'tor.
     */
    ~counted_ptr() noexcept {reset();}

    /** Dispose the pointer.
     */
    void reset() noexcept {if (refs && (--*refs == 0)) delete ptr;}

    /** Returns pointer to held object.
     */
    type_t *operator->() const {return ptr;}

    /** Returns reference to held object.
     */
    type_t &operator*() const {return *ptr;}

    /** Returns true if caller is exclusive owner of the resource.
     */
    bool unique() const {return *refs == 1;}

    /** Returns the number of references.
     */
    std::size_t use_count() const {return *refs;}

    /** Returns pointer to held object.
     */
    type_t *get() const {return ptr;}

protected:
    template <typename fact_type_t, typename... args_t>
    friend counted_ptr<fact_type_t> make_counted(args_t &&...);

    // types
    using refs_t = std::size_t;

    /** C'tor: taking ownership.
     */
    counted_ptr(type_t *ptr, refs_t *refs) noexcept
        : ptr(ptr), refs(refs)
    {}

    type_t *ptr; //!< pointer to the resource
    refs_t *refs; //!< pointer to the shared reference counter
};

/** Factory function for counted ptr.
 */
template <typename type_t, typename... args_t>
counted_ptr<type_t> make_counted(args_t &&...args) {
    struct memory_t {type_t value; std::size_t refs;};
    auto *memory = new memory_t{{std::forward<args_t>(args)...}, 1};
    return counted_ptr<type_t>(&memory->value, &memory->refs);
}

/** Returns true if both pointers points to same object.
 */
template <typename lhs_t, typename rhs_t>
bool operator==(const counted_ptr<lhs_t> &lhs, const counted_ptr<rhs_t> &rhs) {
    return lhs.get() == rhs.get();
}

/** Returns true if pointers doesn't point to same object.
 */
template <typename lhs_t, typename rhs_t>
bool operator!=(const counted_ptr<lhs_t> &lhs, const counted_ptr<rhs_t> &rhs) {
    return lhs.get() != rhs.get();
}

} // namespace Teng

#endif /* TENGCOUNTED_PTR_H */

