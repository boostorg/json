//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STORAGE_HPP
#define BOOST_JSON_STORAGE_HPP

#include <boost/json/config.hpp>
#include <atomic>
#include <cstddef>
#include <stddef.h> // for ::max_align_t

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.

    This interface is modeled similarly to
    `std::pmr::memory_resource` with some notable
    differences:

    @li Instances may be reference counted.

    @li The function @ref is_equal is implemented
    as a non-virtual member which does not require
    RTTI or `typeinfo`.

    @li The function @ref need_free is provided to
    allow the implementation to optionally inform
    callers that calls to deallocate memory are not
    required.
*/
class storage
{
    std::atomic<
        unsigned long long> refs_{ 1 };
    unsigned long long const id_ = 0;
    bool const need_free_ ;
    bool const counted_;

    friend class storage_ptr;

    template<class T>
    friend class scoped_storage;

public:
    virtual
    ~storage() = default;

    /** Returns `true` if calls to `deallocate` are required.
    */
    inline
    bool
    need_free() const noexcept
    {
        return need_free_;
    }

    bool
    is_equal(
        storage const& other) const noexcept
    {
        return (this == &other) || (
            this->id_ != 0 &&
            this->id_ == other.id_);
    }

    /** Allocate memory.

        Allocates storage with for space of at
        least `bytes` octets. The returned storage
        is aligned to the specified alignment is
        supported, and to `alignof(max_align_t)`
        otherwise.

        @throw std::exception if storage of the
        requested size and alignment cannot be
        obtained.
    */
    virtual
    void*
    allocate(
        std::size_t n,
        std::size_t align =
            alignof(max_align_t)) = 0;

    /** Deallocate memory.

        Deallocates the storage pointed to by `p`.

        @par Preconditions

        `p` was returned by a prior call to
        `u.allocate( bytes, align )` where
        `this->is_equal(u)`, and the storage
        `p` points to was not previously
        deallocated.
    */
    virtual
    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t align =
            alignof(max_align_t)) = 0;

    friend
    bool
    operator==(
        storage const& lhs,
        storage const& rhs) noexcept;

    friend
    bool
    operator!=(
        storage const& lhs,
        storage const& rhs) noexcept;

    // Choose a unique 64-bit random number from here:
    // https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    constexpr
    explicit
    storage(
        unsigned long long id,
        bool need_free,
        bool counted) noexcept
        : id_(id)
        , need_free_(need_free || counted)
        , counted_(counted)
    {
    }
};

/** Return true if lhs equals rhs.
*/
inline
bool
operator==(
    storage const& lhs,
    storage const& rhs) noexcept
{
    return lhs.is_equal(rhs);
}

/** Return true if lhs does not equal rhs.
*/
inline
bool
operator!=(
    storage const& lhs,
    storage const& rhs) noexcept
{
    return ! lhs.is_equal(rhs);
}

} // json
} // boost

#endif
