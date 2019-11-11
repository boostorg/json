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
#include <boost/json/detail/assert.hpp>
#include <boost/json/detail/exchange.hpp>
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

    storage* p_;

    inline
    void
    addref() const noexcept
    {
        if(p_->counted_)
            ++p_->refs_;
    }

    inline
    void
    release() const noexcept
    {
        if( p_->counted_ &&
            --p_->refs_ == 0)
            delete p_;
    }

    explicit
    storage_ptr(
        storage* p) noexcept
        : p_(p)
    {
        BOOST_JSON_ASSERT(p);
    }

public:
    /** Default constructor.

        This constructs a default storage pointer.
        The default storage is not reference counted,
        uses global operator new and delete to obtain
        memory, and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    storage_ptr() noexcept;

    /** Construct a pointer to default storage.

        This constructs a default storage pointer.
        The default storage is not reference counted,
        uses global operator new and delete to obtain
        memory, and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr(std::nullptr_t) noexcept
        : storage_ptr()
    {
    }

    /** Destructor.

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

        After construction, the moved-from object
        will point to the default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr&& other) noexcept
        : p_(detail::exchange(
            other.p_,
            storage_ptr().get()))
    {
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
        addref();
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
    storage_ptr&
    operator=(
        storage_ptr&& other) noexcept
    {
        release();
        p_ = detail::exchange(
            other.p_,
            storage_ptr().get());
        return *this;
    }

    /** Copy assignment.

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
    storage_ptr&
    operator=(
        storage_ptr const& other) noexcept
    {
        other.addref();
        release();
        p_ = other.p_;
        return *this;
    }

    /** Return a pointer to the storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage*
    get() const noexcept
    {
        return p_;
    }

    /** Return a pointer to the storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage*
    operator->() const noexcept
    {
        return p_;
    }

    /** Return a reference to the storage object.

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
        return *p_;
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
    struct impl : storage
    {
        T t;

        template<class... Args>
        constexpr
        explicit
        impl(Args&&... args)
            : storage(
                T::id(),
                T::need_free(),
                false)
            , t(std::forward<Args>(args)...)
        {
        }

        void*
        allocate(
            std::size_t n,
            std::size_t align) override
        {
            return t.allocate(n, align);
        }

        void
        deallocate(
            void* p,
            std::size_t n,
            std::size_t align) noexcept override
        {
            t.deallocate(p, n, align);
        }
    };

    impl impl_;

public:
    template<class... Args>
    constexpr
    explicit
    scoped_storage(Args&&... args)
        : impl_(std::forward<Args>(args)...)
    {
    }

    storage*
    get() noexcept
    {
        return &impl_;
    }

    T*
    operator->() noexcept
    {
        return &impl_.t;
    }

    operator storage_ptr() noexcept
    {
        return storage_ptr(&impl_);
    }
};

} // json
} // boost

#include <boost/json/impl/storage_ptr.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/storage_ptr.ipp>
#endif

#endif
