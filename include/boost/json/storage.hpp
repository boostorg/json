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
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.
*/
class storage
{
    std::atomic<std::size_t> refs_;
    //std::size_t refs_;
    unsigned long long id_ = 0;
    bool scoped_ = false;

    BOOST_JSON_DECL
    void
    addref() noexcept;

    BOOST_JSON_DECL
    void
    release() noexcept;

    template<class T>
    friend class basic_storage_ptr;

    template<class T>
    friend class scoped_storage;

public:
    virtual
    ~storage() = default;

    void*
    allocate(
        std::size_t n,
        std::size_t align)
    {
        return do_allocate(n, align);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t align) noexcept
    {
        return do_deallocate(p, n, align);
    }

    bool
    is_scoped() const noexcept
    {
        return scoped_;
    }

    bool
    is_equal(
        storage const& other) const noexcept
    {
        return (this == &other) || (
            this->id_ != 0 &&
            this->id_ == other.id_);
    }

    friend
    bool
    operator==(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return lhs.is_equal(rhs);
    }

    friend
    bool
    operator!=(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return ! lhs.is_equal(rhs);
    }

protected:
    // Choose a unique 64-bit random number from here:
    // https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    BOOST_JSON_DECL
    explicit
    storage(unsigned long long id = 0) noexcept;

    virtual
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) = 0;

    virtual
    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) noexcept = 0;
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

    template<class U>
    friend class scoped_storage;

    T* t_ = nullptr;

    explicit
    basic_storage_ptr(T* t) noexcept
        : t_(t)
    {
    }

public:
    /** Construct a null storage pointer

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

template<class T>
class scoped_storage
{
    T t_;

    BOOST_JSON_STATIC_ASSERT(
        std::is_base_of<storage, T>::value);

public:
    template<class... Args>
    explicit
    scoped_storage(Args&&... args)
        : t_(std::forward<Args>(args)...)
    {
        t_.scoped_ = true;
    }

    operator storage_ptr() noexcept
    {
        return storage_ptr(&t_);
    }
};

//----------------------------------------------------------

/** Return a pointer to the default storage

    This function returns the default storage, which is
    used when constructing a container without explicitly
    specifying the storage. The default storage uses the
    global allocator, equivalent to `std::allocator<char>`.

    @par Complexity

    Constant.

    @par Exception Safety

    No-throw guarantee.

    @par Thread Safety

    May be called concurrently.
*/
BOOST_JSON_DECL
storage_ptr const&
default_storage() noexcept;

} // json
} // boost

#include <boost/json/impl/storage.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/storage.ipp>
#endif

#endif
