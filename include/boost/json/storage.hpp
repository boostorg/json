//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_STORAGE_HPP
#define BOOST_JSON_STORAGE_HPP

#include <boost/json/detail/config.hpp>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.
*/
struct storage
{
    static std::size_t constexpr max_align =
        sizeof(max_align_t);

    virtual
    ~storage() = default;

    virtual
    void
    addref() noexcept = 0;

    virtual
    void
    release() noexcept = 0;

    virtual
    void*
    allocate(
        std::size_t n,
        std::size_t align =
            max_align) = 0;

    virtual
    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t align =
            max_align) noexcept = 0;

    virtual
    bool
    is_equal(
        storage const& other) const noexcept = 0;

    friend
    bool
    operator==(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return &lhs == &rhs || lhs.is_equal(rhs);
    }

    friend
    bool
    operator!=(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

//------------------------------------------------------------------------------

/** Manages a type-erased storage object and options for a set of JSON values.
*/
class storage_ptr
{
    storage* p_ = nullptr;

public:
    storage_ptr() = default;

    BOOST_JSON_DECL
    ~storage_ptr();

    BOOST_JSON_DECL
    storage_ptr(storage_ptr&&) noexcept;

    BOOST_JSON_DECL
    storage_ptr(storage_ptr const&) noexcept;

    BOOST_JSON_DECL
    storage_ptr&
    operator=(storage_ptr&&) noexcept;

    BOOST_JSON_DECL
    storage_ptr&
    operator=(storage_ptr const&) noexcept;

    /** Constructor

        @param p A pointer to a storage object. Ownership of the
        object is transferred; the reference count is not adjusted.
    */
    BOOST_JSON_DECL
    storage_ptr(storage* p) noexcept;

    /** Return ownership of the managed storage object.
    */
    BOOST_JSON_DECL
    storage*
    release() noexcept;

    explicit
    operator bool() const noexcept
    {
        return p_ != nullptr;
    }

    storage*
    get() const noexcept
    {
        return p_;
    }

    storage*
    operator->() const noexcept
    {
        return p_;
    }

    storage&
    operator*() const noexcept
    {
        return *p_;
    }
};

inline
bool
operator==(storage_ptr const& lhs, storage_ptr const& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

inline
bool
operator==(storage* lhs, storage_ptr const& rhs) noexcept
{
    return lhs == rhs.get();
}

inline
bool
operator==(storage_ptr const& lhs, storage* rhs) noexcept
{
    return lhs.get() == rhs;
}

inline
bool
operator!=(storage_ptr const& lhs, storage_ptr const& rhs) noexcept
{
    return lhs.get() != rhs.get();
}

inline
bool
operator!=(storage* lhs, storage_ptr const& rhs) noexcept
{
    return lhs != rhs.get();
}

inline
bool
operator!=(storage_ptr const& lhs, storage* rhs) noexcept
{
    return lhs.get() != rhs;
}

//------------------------------------------------------------------------------

/** Construct a storage pointer to the specified allocator
*/
template<class Allocator>
storage_ptr
make_storage_ptr(Allocator const& a);

/** Return a pointer to the current default storage
*/
BOOST_JSON_DECL
storage_ptr
default_storage();

/** Set the current default storage

    This function may not be called concurrently,
    or concurrent with @ref default_storage.
*/
BOOST_JSON_DECL
void
default_storage(storage_ptr sp);

} // json
} // boost

#include <boost/json/impl/storage.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/storage.ipp>
#endif

#endif
