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
#include <boost/core/exchange.hpp>
#include <boost/static_assert.hpp>
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.
*/
class storage
{
    std::atomic<std::size_t> refs_;

    BOOST_JSON_DECL
    void
    addref();

    BOOST_JSON_DECL
    void
    release();

    template<class T>
    friend class basic_storage_ptr;

public:
    static std::size_t constexpr max_align =
        sizeof(max_align_t);

    BOOST_JSON_DECL
    storage();

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

//----------------------------------------------------------

/** Manages a type-erased storage object and options for a set of JSON values.
*/
template<class T>
class basic_storage_ptr
{
    BOOST_STATIC_ASSERT(
        std::is_base_of<storage, T>::value);

    template<class U>
    friend class basic_storage_ptr;

    using count = std::atomic<unsigned>;

    T* t_ = nullptr;

    explicit
    basic_storage_ptr(T* t) noexcept
        : t_(t)
    {
    }

public:
    basic_storage_ptr() = default;

    ~basic_storage_ptr()
    {
        if(t_)
            t_->release();
    }

    basic_storage_ptr(
        basic_storage_ptr&& other) noexcept
        : t_(boost::exchange(other.t_, nullptr))
    {
    }


    basic_storage_ptr(
        basic_storage_ptr const& other) noexcept
        : t_(other.t_)
    {
        if(t_)
            t_->addref();
    }


    template<class U
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<U*, T*>::value &&
            ! std::is_same<U, T>::value
                >::type
    #endif
    >
    basic_storage_ptr(
        basic_storage_ptr<U>&& sp) noexcept
        : t_(boost::exchange(sp.t_, nullptr))
    {
    }

    template<class U
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<U*, T*>::value &&
            ! std::is_same<U, T>::value
                >::type
    #endif
    >
    basic_storage_ptr(
        basic_storage_ptr<U> const& sp) noexcept
        : t_(sp.t_)
    {
        if(t_)
            t_->addref();
    }

    basic_storage_ptr(
        std::nullptr_t) noexcept
    {
    }

    basic_storage_ptr&
    operator=(
        basic_storage_ptr&& other) noexcept
    {
        if(t_)
            t_->release();
        t_ = boost::exchange(other.t_, nullptr);
        return *this;
    }

    basic_storage_ptr&
    operator=(
        basic_storage_ptr const& other) noexcept
    {
        if(other.t_)
            other.t_->addref();
        if(t_)
            t_->release();
        t_ = other.t_;
        return *this;
    }

    explicit
    operator bool() const noexcept
    {
        return t_ != nullptr;
    }

    T*
    get() const noexcept
    {
        return t_;
    }

    T*
    operator->() const noexcept
    {
        return t_;
    }

    T&
    operator*() const noexcept
    {
        return *t_;
    }

    template<class U, class... Args>
    friend
    basic_storage_ptr<U>
    make_storage(Args&&... args);
};

using storage_ptr = basic_storage_ptr<storage>;

BOOST_JSON_DECL
bool
operator==(storage_ptr const& lhs, storage_ptr const& rhs) noexcept;

BOOST_JSON_DECL
bool
operator==(storage* lhs, storage_ptr const& rhs) noexcept;

BOOST_JSON_DECL
bool
operator==(storage_ptr const& lhs, storage* rhs) noexcept;

BOOST_JSON_DECL
bool
operator!=(storage_ptr const& lhs, storage_ptr const& rhs) noexcept;

BOOST_JSON_DECL
bool
operator!=(storage* lhs, storage_ptr const& rhs) noexcept;

BOOST_JSON_DECL
bool
operator!=(storage_ptr const& lhs, storage* rhs) noexcept;

//----------------------------------------------------------

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
