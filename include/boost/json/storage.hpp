//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STORAGE_HPP
#define BOOST_JSON_STORAGE_HPP

#include <boost/json/detail/config.hpp>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>

#ifdef BOOST_JSON_TRACK_STORAGE
#include <atomic>
#endif

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.
*/
class storage
{
public:
    static std::size_t constexpr max_align =
        sizeof(max_align_t);

    virtual
    ~storage() = default;

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
#ifndef BOOST_JSON_TRACK_STORAGE
using storage_ptr = std::shared_ptr<storage>;

#else
template<class T>
class basic_storage_ptr
{
    template<class U>
    friend class basic_storage_ptr;

    using count = std::atomic<unsigned>;

    T* pv_ = nullptr;
    count* pn_ = nullptr;

    void
    increment() const noexcept;

    void
    decrement() const noexcept;

    explicit
    basic_storage_ptr(T* t) noexcept;

public:
    ~basic_storage_ptr();

    basic_storage_ptr() = default;

    basic_storage_ptr(
        basic_storage_ptr&&) noexcept;

    basic_storage_ptr(
        basic_storage_ptr const&) noexcept;

    template<class U
        ,class = typename std::enable_if<
            std::is_convertible<U*, T*>::value &&
            ! std::is_same<U, T>::value
                >::type
    >
    basic_storage_ptr(
        basic_storage_ptr<U>&& sp) noexcept;

    template<class U
        ,class = typename std::enable_if<
            std::is_convertible<U*, T*>::value &&
            ! std::is_same<U, T>::value
                >::type
    >
    basic_storage_ptr(
        basic_storage_ptr<U> const& sp) noexcept;

    basic_storage_ptr(
        std::nullptr_t) noexcept;

    basic_storage_ptr&
    operator=(
        basic_storage_ptr&&) noexcept;

    basic_storage_ptr&
    operator=(
        basic_storage_ptr const&) noexcept;

    explicit
    operator bool() const noexcept
    {
        return pv_ != nullptr;
    }

    T*
    get() const noexcept
    {
        return pv_;
    }

    T*
    operator->() const noexcept
    {
        return pv_;
    }

    T&
    operator*() const noexcept
    {
        return *pv_;
    }

    template<class U, class... Args>
    friend
    basic_storage_ptr<U>
    make_storage(Args&&... args);
};

using storage_ptr = basic_storage_ptr<storage>;

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

#endif

//------------------------------------------------------------------------------

/** Construct a new storage object
*/
#ifndef BOOST_JSON_TRACK_STORAGE
template<class Storage, class... Args>
std::shared_ptr<Storage>
make_storage(Args&&... args);
#else
template<class Storage, class... Args>
basic_storage_ptr<Storage>
make_storage(Args&&... args);
#endif

/** Construct a storage adaptor for the specified allocator
*/
template<class Allocator>
storage_ptr
make_storage_adaptor(Allocator const& a);

/** Return a pointer to the current default storage
*/
BOOST_JSON_DECL
storage_ptr
default_storage() noexcept;

/** Set the current default storage

    This function may not be called concurrently,
    or concurrent with @ref default_storage.
*/
BOOST_JSON_DECL
void
default_storage(storage_ptr sp) noexcept;

} // json
} // boost

#include <boost/json/impl/storage.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/storage.ipp>
#endif

#endif
