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

BOOST_JSON_NS_BEGIN

/** Return true if a memory resource's deallocate function has no effect.

    This metafunction may be specialized to indicate to
    the library that calls to the `deallocate` function of
    a @ref memory_resource have no effect. The implementation
    will elide such calls when it is safe to do so. By default,
    the implementation assumes that all memory resources
    require a call to `deallocate` for each memory region
    obtained by calling `allocate`.

    @par Example

    This example specializes the metafuction for `my_resource`,
    to indicate that calls to deallocate have no effect:

    @code

    // Forward-declaration for a user-defined memory resource
    struct my_resource;

    // It is necessary to specialize the template from
    // inside the namespace in which it is declared:

    namespace boost {
    namespace json {

    template<>
    struct is_deallocate_trivial< my_resource >
    {
        static constexpr bool value = true;
    };

    } // namespace json
    } // namespace boost

    @endcode

    It is usually not necessary for users to check this trait.
    Instead, they can call @ref storage_ptr::is_deallocate_trivial
    to determine if the pointed-to memory resource has a trivial
    deallocate function.

    @see
        @ref memory_resource,
        @ref storage_ptr
*/
template<class T>
struct is_deallocate_trivial
{
    /** A bool equal to true if calls to `T::do_deallocate` have no effect.

        The primary template sets `value` to false.
    */
    static constexpr bool value = false;
};

/** A smart pointer to a @ref memory_resource

    This container is used to hold a pointer to a
    memory resource. The pointed-to resource is
    always valid; default-constructed pointers
    use the default memory resource, which calls
    into the standard global system heap.
    Depending on the means of construction, the
    ownership will be either:

    @li Non-owning, when constructing from a raw
    pointer to @ref memory_resource or from a
    @ref polymorphic_allocator. In this case the
    caller is responsible for ensuring that the
    lifetime of the memory resource extends until
    there are no more calls to allocate or
    deallocate.

    @li Owning, when constructing using the function
    @ref make_counted_resource. In this case 
    ownership is shared; the lifetime of the memory
    resource extends until the last copy of the
    @ref storage_ptr is destroyed.

    @par Examples

    These statements create a memory resource on the
    stack and construct a pointer from it without
    taking ownership:

    @code

    // Create our memory resource on the stack
    monotonic_resource mr;

    // Construct a non-owning pointer to the resource
    storage_ptr sp( &mr );

    @endcode

    This function creates a pointer to a memory
    resource using shared ownership and returns it.
    The lifetime of the memory resource extends until
    the last copy of the pointer is destroyed:

    @code

    // Create a counted memory resource and return it
    storage_ptr make_storage()
    {
        return make_counted_resource< monotonic_resource >();
    }

    @endcode

    @see
        @ref make_counted_resource,
        @ref memory_resource,
        @ref polymorphic_allocator
*/
class storage_ptr
{
#ifndef BOOST_JSON_DOCS
    // VFALCO doc toolchain shows this when it shouldn't
    friend struct detail::counted_resource;
#endif
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

    void
    addref() const noexcept
    {
        if(is_counted())
            ++get_counted()->refs;
    }

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
            (json::is_deallocate_trivial<T>::value ? 2 : 0))
    {
        BOOST_ASSERT(p);
    }

public:
    /** Destructor

        If the pointer has shared ownership of the
        resource, the shared ownership is released.
        If this is the last owned copy, the memory
        resource is destroyed.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    ~storage_ptr()
    {
        release();
    }

    /** Constructor

        This constructs a non-owning pointer that refers
        to the default memory resource, which uses the
        standard global system heap to allocate and
        free memory.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr() noexcept
        : i_(0)
    {
    }

    /** Constructor

        This constructs a non-owning pointer that
        points to the memory resource `r`.
        The caller is responsible for maintaining the
        lifetime of the pointed-to @ref memory_resource.

        @par Constraints
        @code
        std::is_convertible< T*, memory_resource* >::value == true
        @endcode

        @par Preconditions
        @code
        r != nullptr
        @endcode

        @par Exception Safety

        No-throw guarantee.

        @param r A pointer to the memory resource to use.
        This may not be null.
    */
    template<class T
#ifndef BOOST_JSON_DOCS
        , class = typename std::enable_if<
            std::is_convertible<T*,
                memory_resource*>::value>::type
#endif
    >
    storage_ptr(T* r) noexcept
        : i_(reinterpret_cast<std::uintptr_t>(
                static_cast<memory_resource *>(r)) +
            (json::is_deallocate_trivial<T>::value ? 2 : 0))
    {
        BOOST_ASSERT(r);
    }

    /** Constructor

        This constructs a non-owning pointer that
        points to the same memory resource as `alloc`,
        obtained by calling `alloc.resource()`.
        The caller is responsible for maintaining the
        lifetime of the pointed-to @ref memory_resource.

        @par Constraints
        @code
        std::is_convertible< T*, memory_resource* >::value == true
        @endcode

        @par Exception Safety

        No-throw guarantee.

        @param alloc A @ref polymorphic_allocator to
        construct from.
    */
    template<class T>
    storage_ptr(
        polymorphic_allocator<T> const& alloc) noexcept
        : i_(reinterpret_cast<std::uintptr_t>(
            alloc.resource()))
    {
    }

    /** Move constructor

        This function constructs a pointer that
        points to the same memory resource as `other`,
        with the same ownership:

        @li If `other` is non-owning, then `*this`
        will be be non-owning.

        @li If `other` has shared ownership, then
        ownership will be transferred to `*this`.

        After construction, `other` will point
        to the default memory resource.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The pointer to construct from.
    */
    storage_ptr(
        storage_ptr&& other) noexcept
        : i_(detail::exchange(
            other.i_, 0))
    {
    }

    /** Copy constructor

        This function constructs a pointer that
        points to the same memory resource as `other`,
        with the same ownership:

        @li If `other` is non-owning, then `*this`
        will be be non-owning.

        @li If `other` has shared ownership, then
        `*this` will acquire shared ownership.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The pointer to construct from.
    */
    storage_ptr(
        storage_ptr const& other) noexcept
        : i_(other.i_)
    {
        addref();
    }

    /** Move assignment

        This function assigns a pointer that
        points to the same memory resource as `other`,
        with the same ownership:

        @li If `other` is non-owning, then `*this`
        will be be non-owning.

        @li If `other` has shared ownership, then
        ownership will be transferred to `*this`.

        After assignment, `other` will point
        to the default memory resource.
        If `*this` previously had shared ownership,
        it is released before the function returns.

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

        This function assigns a pointer that
        points to the same memory resource as `other`,
        with the same ownership:

        @li If `other` is non-owning, then `*this`
        will be be non-owning.

        @li If `other` has shared ownership, then
        `*this` will acquire shared ownership.

        If `*this` previously had shared ownership,
        it is released before the function returns.

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

        This function returns true for memory resources
        created using @ref make_counted_resource.
    */
    bool
    is_counted() const noexcept
    {
        return (i_ & 1) != 0;
    }

    /** Return `true` if calling `deallocate` on the memory resource has no effect.

        This function is used to determine if the deallocate
        function of the pointed to memory resource is trivial.
        The value of @ref is_deallocate_trivial is evaluated
        and saved when the memory resource is constructed
        and the type is known, before the type is erased.
    */
    bool
    is_deallocate_trivial() const noexcept
    {
        return (i_ & 2) != 0;
    }

    /** Return `true` if ownership of the memory resource is not shared and deallocate is trivial.

        This function is used to determine if calls to deallocate
        can effectively be skipped.

        @par Effects
        Returns `! this->is_counted() && this->is_deallocate_trivial()`
    */
    bool
    is_not_counted_and_deallocate_is_trivial() const noexcept
    {
        return (i_ & 3) == 2;
    }

    /** Return a pointer to the memory resource.

        This function returns a pointer to the
        referenced @ref memory_resource.

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

        This function returns a pointer to the
        referenced @ref memory_resource.

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

        This function returns a reference to the
        pointed-to @ref memory_resource.

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

/** Return shared ownership of a newly created memory resource.

    This function creates a memory resource that uses
    shared ownership. The lifetime of the memory resource
    will be extended until the last @ref storage_ptr which
    points to it is destroyed.

    @par Mandates
    @code
    std::is_base_of< memory_resource, T >::value == true
    @endcode

    @par Complexity
    Same as `T( std::forward<Args>(args)... )`.

    @par Exception Safety
    Strong guarantee.

    @param args Parameters forwarded to the constructor of `T`.

    @tparam T The type of memory resource to create.
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

BOOST_JSON_NS_END

#endif
