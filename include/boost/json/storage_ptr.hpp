//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STORAGE_PTR_HPP
#define BOOST_JSON_STORAGE_PTR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage.hpp>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** Manages a type-erased storage object and options for a set of JSON values.
*/
class storage_ptr
{
    template<class T>
    friend class scoped_storage;

    storage* p_ = nullptr;

    inline
    void
    release() const noexcept
    {
        if( p_ &&
            p_->counted_ &&
            --p_->refs_ == 0)
            delete p_;
    }

    explicit
    storage_ptr(storage* p) noexcept
        : p_(p)
    {
    }

public:
    /** Construct a default storage pointer

        This constructs a default storage pointer.
        The default storage is not reference counted,
        uses global operator new and delete to obtain
        memory, and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr() = default;

    storage_ptr(std::nullptr_t) noexcept
    {
    }

    /** Destroy a storage pointer.

        This releases the pointed-to storage. If the
        storage is reference counted and this is the
        last reference. the storage object is destroyed.
        If the storage does not require deallocation,
        all memory allocated using this storage is
        invalidated.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    ~storage_ptr()
    {
        release();
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
    storage_ptr(
        storage_ptr&& other) noexcept
        : p_(other.p_)
    {
        other.p_ = nullptr;
    }

    /** Copy construct a storage pointer.

        If `other` points to a valid @ref storage
        object, then this pointer acquires shared
        ownership of the storage. Otherwise, the
        newly constructed pointer is the default
        storage pointer.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr const& other) noexcept
        : p_(other.p_)
    {
        if(p_)
            ++p_->refs_;
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
    inline
    storage_ptr&
    operator=(
        storage_ptr&& other) noexcept;

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
    inline
    storage_ptr&
    operator=(
        storage_ptr const& other) noexcept;

    /** Return a pointer to the storage object.

        If `this` points to a valid storage object,
        it is returned. Otherwise the return value
        is `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    inline
    storage*
    get() const noexcept;

    /** Return a pointer to the storage object.

        If `this` points to a valid storage object,
        it is returned. Otherwise the return value
        is `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage*
    operator->() const noexcept
    {
        return get();
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
    storage&
    operator*() const noexcept
    {
        return *get();
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
    storage_ptr
    make_storage(Args&&... args);
};

#ifndef GENERATING_DOCUMENTATION
inline
bool
operator==(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

inline
bool
operator!=(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() != rhs.get();
}
#endif

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
        t_.counted_ = false;
    }

    T*
    get() noexcept
    {
        return &t_;
    }

    T*
    operator->() noexcept
    {
        return get();
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
inline
storage_ptr
default_storage() noexcept
{
    return {};
}

} // json
} // boost

#include <boost/json/impl/storage_ptr.hpp>

#endif
