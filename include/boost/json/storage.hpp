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

namespace detail {
template<class T>
struct storage_impl;
} // detail

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
    std::atomic<std::size_t> refs_{ 1 };
    std::uint64_t const id_ = 0;
    bool const need_free_ ;
    bool const counted_;

    friend class storage_ptr;

    template<class T>
    friend struct detail::storage_impl;

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
    BOOST_JSON_NODISCARD
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

/** Metafunction to determine if a type meets the requirements of Storage.

    This type alias is `std::true_type` if the type
    `T` satisfies the syntactic requirements of <em>Storage</em>,
    otherwise it is equivalent to `std::false_type`.

    @par Exemplar

    For the following declaration,
    `is_storage<Storage>` is `std::true_type`:

    @code
    struct Storage
    {
        static constexpr std::uint64_t id = 0;
        static_constexpr bool need_free = true;

        void* allocate( std::size_t bytes, std::size_t align );
        void deallocate( void* p, std::size_t bytes, std::size_t align );
    };
    @endcode

    @tparam T The type to check.
*/
#if GENERATING_DOCUMENTATION
template<class T>
using is_storage = __see_below__;
#else
template<class T, class = void>
struct is_storage : std::false_type {};

template<class T>
struct is_storage<T, detail::void_t<decltype(
    T::id,
    T::need_free,
    std::declval<void*&>() =
        std::declval<T&>().allocate(
            std::declval<std::size_t>(),
            std::declval<std::size_t>()),
    std::declval<T&>().deallocate(
            std::declval<void*>(),
            std::declval<std::size_t>(),
            std::declval<std::size_t>())
            ) > > : std::true_type
{
};
#endif

} // json
} // boost

#include <boost/json/impl/storage.hpp>

#endif
