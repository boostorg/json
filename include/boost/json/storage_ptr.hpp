//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_STORAGE_PTR_HPP
#define BOOST_JSON_STORAGE_PTR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/memory_resource.hpp>
#include <boost/json/detail/counted_resource.hpp>
#include <boost/json/detail/default_resource.hpp>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** Determine if a call to a memory resource's deallocate function is a no-op.

    A user-defined @ref memory_resource can specialize
    this metafunction to indicate to the library that
    calls to the `deallocate` function are not necessary
    or have no effect. The implementation will elide such
    calls when it is safe to do so. By default, the
    implementation assumes that all memory resources
    require calls to deallocate.

    @par Example

    This example specializes the metafuction for `my_resource`,
    to indicate that calls to deallocate are no-ops:

    @code

    // Forward-declaration for a user-defined memory resource
    struct my_resource;

    // It is necessary to specialize the template from
    // inside the namespace in which it is declared:

    namespace boost {
    namespace json {

    template<>
    struct is_deallocate_null< my_resource >
    {
        static constexpr bool value = true;
    };

    } // namespace json
    } // namespace boost

    @endcode

    @see
        @ref memory_resource,
        @ref storage_ptr.
*/
template<class T>
struct is_deallocate_null
{
    /** A bool equal to true if calls to `T::do_deallocate` have no effect.

        The value defaults to `false` for all unspecialized types.
    */
    static constexpr bool value = false;
};

/** Manages a type-erased storage object.

    This container is used to hold an owning or
    non-owning reference to a @ref memory_resource
    object.
*/
class storage_ptr
{
    friend struct detail::counted_resource;
    using counted_resource =
        detail::counted_resource;

    std::uintptr_t i_ = 0;

    static
    memory_resource*
    get_default() noexcept
    {
        BOOST_JSON_REQUIRE_CONST_INIT
        static detail::default_resource impl;
        return &impl;
    }

    counted_resource*
    get_counted() const noexcept
    {
        return reinterpret_cast<
            counted_resource*>(i_ & ~3);
    }

    inline
    void
    addref() const noexcept
    {
        if(is_counted())
            ++get_counted()->refs;
    }

    inline
    void
    release() const noexcept
    {
        if(is_counted())
        {
            auto const p = get_counted();
            if(--p->refs == 0)
                delete p;
        }
    }

    template<class T>
    storage_ptr(
        detail::counted_resource_impl<T>* p) noexcept
        : i_(reinterpret_cast<
            std::uintptr_t>(p) + 1 +
            (is_deallocate_null<T>::value ? 2 : 0))
    {
        BOOST_ASSERT(p);
    }

public:
    /** Destructor.

        This releases the pointed-to memory resource.
        If the memory resource is counted and this is the
        last reference, the memory resource is destroyed.
        If the memory resource does not require deallocation,
        all memory allocated using this memory resource is
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

    /** Default constructor.

        This constructs a storage pointer that refers to
        the default memory resource. The default memory
        resource is not reference counted, uses the global
        allocation functions `operator new` and
        `operator delete` to allocate and deallocate memory,
        and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr() noexcept
        : i_(0)
    {
    }

    /** Construct a pointer to a memory resource.

        The caller is responsible for maintaining the
        lifetime of the pointed-to @ref memory_resource.
        Ownership is not transferred.

        @par Constraints
        @code
        std::is_convertible< T*, memory_resource* >::value == true
        @endcode

        @par Exception Safety

        No-throw guarantee.

        @param p A pointer to the memory resource to use.
    */
    template<class T
#ifndef BOOST_JSON_DOCS
        , class = typename std::enable_if<
            std::is_convertible<T*,
                memory_resource*>::value>::type
#endif
    >
    storage_ptr(T* p) noexcept
        : i_(reinterpret_cast<std::uintptr_t>(
                static_cast<memory_resource *>(p)) +
            (is_deallocate_null<T>::value ? 2 : 0))
    {
        BOOST_ASSERT(p);
    }

    template<class T>
    storage_ptr(
        polymorphic_allocator<T> const& alloc) noexcept
        : i_(reinterpret_cast<std::uintptr_t>(
            alloc.resource()))
    {
    }

    /** Move constructor.

        After construction, the moved-from object
        will point to the default memory resource.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr&& other) noexcept
        : i_(detail::exchange(
            other.i_, 0))
    {
    }

    /** Copy constructor.

        This function acquires shared ownership of
        the memory resource pointed to by `other`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr const& other) noexcept
        : i_(other.i_)
    {
        addref();
    }

    /** Move assignment.

        Shared ownership of the memory resource owned by `*this`
        is released, and shared ownership of the memory resource
        owned by `other` is acquired by move construction.
        After construction, the moved-from object will
        point to the default memory resource.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to move.
    */
    storage_ptr&
    operator=(
        storage_ptr&& other) noexcept
    {
        release();
        i_ = detail::exchange(
            other.i_, 0);
        return *this;
    }

    /** Copy assignment.

        Shared ownership of the memory resource owned by `*this`
        is released, and shared ownership of the memory resource
        owned by `other` is acquired by copy construction.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to copy.
    */
    storage_ptr&
    operator=(
        storage_ptr const& other) noexcept
    {
        other.addref();
        release();
        i_ = other.i_;
        return *this;
    }

    /** Return `true` if the storage pointer has shared ownership of the memory resource.
    */
    bool
    is_counted() const noexcept
    {
        return (i_ & 1) != 0;
    }

    /** Return `true` if the memory resource does not require deallocate to be called.
    */
    bool
    deallocate_is_null() const noexcept
    {
        return (i_ & 2) != 0;
    }

    /** Return `true` if ownership of the memory resource is not shared and deallocate is null.
    */
    bool
    is_not_counted_and_deallocate_is_null() const noexcept
    {
        return (i_ & 3) == 2;
    }

    /** Return a pointer to the memory resource.

        This function returns a pointer to the
        @ref memory_resource being pointed to.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    memory_resource*
    get() const noexcept
    {
        if(i_ != 0)
            return reinterpret_cast<
                memory_resource*>(i_ & ~3);
        return get_default();
    }

    /** Return a pointer to the memory resource.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    memory_resource*
    operator->() const noexcept
    {
        return get();
    }

    /** Return a reference to the memory resource.

        @par Precondition

        `this` points to a valid memory resource.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    memory_resource&
    operator*() const noexcept
    {
        return *get();
    }

    template<class U, class... Args>
    friend
    storage_ptr
    make_counted_resource(Args&&... args);
};

/** Create a new, counted memory resource and return a pointer to it.

    This functions similarly to `make_shared`.

    @par Mandates
    @code
    std::is_base_of< memory_resource, T >::value == true
    @endcode

    @par Complexity

    Same as `T(std::forward<Args>(args)...)`.

    @par Exception Safety

    Strong guarantee.

    @param args Parameters forwarded to the constructor of `T`.

    @tparam T the concrete type of the memory resource to create.
*/
template<class T, class... Args>
storage_ptr
make_counted_resource(Args&&... args)
{
    // If this generates an error, it means that
    // `T` is not a memory resource.
    BOOST_STATIC_ASSERT(
        std::is_base_of<
            memory_resource, T>::value);
    return storage_ptr(new 
        detail::counted_resource_impl<T>(
            std::forward<Args>(args)...));
}

/** Return true if lhs equals rhs.

    This function returns `true` if the @ref memory_resource
    objects pointed to by `lhs` and `rhs` have the
    same address.
*/
inline
bool
operator==(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

/** Return true if lhs does not equal rhs.

    This function returns `true` if the @ref memory_resource
    objects pointed to by `lhs` and `rhs` have different
    addresses.
*/
inline
bool
operator!=(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() != rhs.get();
}

} // json
} // boost

#endif
