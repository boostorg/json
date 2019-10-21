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
    addref() noexcept;

    BOOST_JSON_DECL
    void
    release() noexcept;

    template<class T>
    friend class basic_storage_ptr;

public:
    static std::size_t constexpr max_align =
        sizeof(max_align_t);

    BOOST_JSON_DECL
    storage() noexcept;

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
        return &lhs != &rhs && ! lhs.is_equal(rhs);
    }
};

//----------------------------------------------------------

/** Manages a type-erased storage object and options for a set of JSON values.
*/
template<class T>
class basic_storage_ptr
{
    BOOST_JSON_STATIC_ASSERT(
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
    /** Construct an null storage pointer

        This constructs a null storage pointer.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    basic_storage_ptr() = default;

    /** Destroy a storage pointer.

        If `this` is not null, the reference counter
        on the @ref storage object is decrement. When
        the reference count reaches zero, the object
        is destroyed.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    ~basic_storage_ptr()
    {
        if(t_)
            t_->release();
    }

    /** Move construct a storage pointer.

        After construction, the moved-from pointer
        will be null.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    basic_storage_ptr(
        basic_storage_ptr&& other) noexcept
        : t_(other.t_)
    {
        other.t_ = nullptr;
    }

    /** Copy construct a storage pointer.

        If `other` points to a valid @ref storage
        object, then this pointer acquires shared
        ownership of the storage. Otherwise, the
        newly constructed pointer is equal to null.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
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
        : t_(sp.t_)
    {
        sp.t_ = nullptr;
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

    /** Construct an null storage pointer

        This constructs a null storage pointer.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    basic_storage_ptr(
        std::nullptr_t) noexcept
    {
    }

    /** Move assign a storage pointer.

        If `this` points to a valid object, it is
        decremented as if by a call to the destructor.
        After construction, the moved-from pointer
        will be null.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to assign from.
    */
    basic_storage_ptr&
    operator=(
        basic_storage_ptr&& other) noexcept
    {
        if(t_)
            t_->release();
        t_ = other.t_;
        other.t_ = nullptr;
        return *this;
    }

    /** Copy construct a storage pointer.

        If `this` points to a valid object, it is
        decremented as if by a call to the destructor.
        If `other` points to a valid @ref storage
        object, then this pointer acquires shared
        ownership of the storage. Otherwise, the
        newly constructed pointer is equal to null.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to assign from.
    */
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

    /** Return true if this points to a valid storage object.

        This function returns true if @ref get() returns
        a non-null value.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

    */
    explicit
    operator bool() const noexcept
    {
        return t_ != nullptr;
    }

    /** Return a pointer to the storage object.

        If `this` points to a valid storage object,
        it is returned. Otherwise the return value
        is `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    T*
    get() const noexcept
    {
        return t_;
    }

    /** Return a pointer to the storage object.

        If `this` points to a valid storage object,
        it is returned. Otherwise the return value
        is `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    T*
    operator->() const noexcept
    {
        return t_;
    }

    /** Return a reference to the storage object.

        If `this` points to a valid storage object,
        it is returned. Otherwise the behavior is
        undefined.

        @par Precondition

        `this` points to a valid storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    T&
    operator*() const noexcept
    {
        return *t_;
    }

    /** Create a new storage object and return a pointer to it.

        This functions similarly to `make_shared`.

        @par Mandates

        `std::is_base_of_v<storage, U>`

        @par Complexity

        Same as `U(std::forward<Args>(args)...)`.

        @par Exception Safety

        Strong guarantee.

        @param args Parameters forwarded to the constructor of `U`.

        @tparam U the type of the storage object to create.
    */
    template<class U, class... Args>
    friend
    basic_storage_ptr<U>
    make_storage(Args&&... args);
};

/** Compare two storage pointers
*/
template<class T, class U>
bool
operator==(
    basic_storage_ptr<T> const& lhs,
    basic_storage_ptr<U> const& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

/** Compare two storage pointers
*/
template<class T, class U>
bool
operator!=(
    basic_storage_ptr<T> const& lhs,
    basic_storage_ptr<U> const& rhs) noexcept
{
    return lhs.get() != rhs.get();
}

/** Compare two storage pointers
*/
template<class T>
bool
operator==(
    basic_storage_ptr<T> const& lhs,
    std::nullptr_t) noexcept
{
    return lhs.get() == nullptr;
}

/** Compare two storage pointers
*/
template<class T>
bool
operator!=(
    basic_storage_ptr<T> const& lhs,
    std::nullptr_t) noexcept
{
    return lhs.get() != nullptr;
}

/** Compare two storage pointers
*/
template<class T>
bool
operator==(
    std::nullptr_t,
    basic_storage_ptr<T> const& rhs) noexcept
{
    return rhs.get() == nullptr;
}

/** Compare two storage pointers
*/
template<class T>
bool
operator!=(
    std::nullptr_t,
    basic_storage_ptr<T> const& rhs) noexcept
{
    return rhs.get() != nullptr;
}

/// A type-erased storage pointer.
using storage_ptr = basic_storage_ptr<storage>;

//----------------------------------------------------------

/** Return a pointer to the current default storage

    This function returns a pointer to the current default storage.
    This default storage is used when constructing
    any @ref value, @ref object, @ref array, or @ref string
    and the storage is not explicitly specified.

    @par Complexity

    Constant.

    @par Exception Safety

    No-throw guarantee.

    @par Thread Safety

    May not be called concurrently with
    `void default_storage(storage_ptr)`.
*/
BOOST_JSON_DECL
storage_ptr
default_storage() noexcept;

/** Set the current default storage

    This function changes the current default storage pointer.
    This default storage is used when constructing
    any @ref value, @ref object, @ref array, or @ref string
    and the storage is not explicitly specified.

    @par Complexity

    Constant.

    @par Exception Safety

    No-throw guarantee.

    @par Thread Safety

    May not be called concurrently with
    `void default_storage()` or
    `void default_storage(storage_ptr)`.
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
