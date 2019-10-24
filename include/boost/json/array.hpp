//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_ARRAY_HPP
#define BOOST_JSON_ARRAY_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/storage.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>

namespace boost {
namespace json {

class value;

/** A dynamically sized array of JSON values

    This is the type used to represent JSON values of kind array.
    It strives for semantic equivalence to `std::vector<value>`.

    The elements are stored contiguously, which means that elements
    can be accessed not only through iterators, but also using offsets
    to regular pointers to elements. This means that a pointer to an
    element of an @ref array may be passed to any function that expects
    a pointer to an element of an array.

    The storage of the array is handled automatically, being expanded
    and contracted as needed. Arrays usually occupy more space than
    array language constructs, because more memory is allocated to
    handle future growth. This way an array does not need to reallocate
    each time an element is inserted, but only when the additional
    memory is exhausted. The total amount of allocated memory can be
    queried using the @ref capacity function. Extra memory can be
    relinquished by calling @ref shrink_to_fit.

    Reallocations are usually costly operations in terms of performance.
    The @ref reserve function can be used to eliminate reallocations
    if the number of elements is known beforehand.

    The complexity (efficiency) of common operations on arrays is as follows:

    @li Random access - constant *O(1)*
    @li Insertion or removal of elements at the end - amortized constant *O(1)*
    @li Insertion or removal of elements - linear in the distance to the end of
      the array *O(n)*

    @par Storage

    All elements stored in the container will use the same storage that
    was used to construct the container, including recursive children
    of those elements.

    @par Thread Safety

    Non-const member functions may not be called concurrently.

    @par Satisfies

    Meets the requirements of
        <em>Container</em>,
        <em>ContiguousContainer</em>,
        <em>ReversibleContainer</em>, and
        <em>SequenceContainer</em>.
*/
class array
{
    struct table;

    table* tab_ = nullptr;
    storage_ptr sp_;

    struct undo_create;
    struct undo_insert;

public:
    /// The type of each element
    using value_type = value;

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference = value&;

    /// A const reference to an element
    using const_reference = value const&;

    /// A pointer to an element
    using pointer = value*;

    /// A const pointer to an element
    using const_pointer = value const*;

    /// A random access iterator to an element
    using iterator = value*;

    /// A const random access iterator to an element
    using const_iterator = value const*;

    /// A reverse random access iterator to an element
    using reverse_iterator =
        std::reverse_iterator<iterator>;

    /// A const reverse random access iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    //------------------------------------------------------

    /** Destroy the container

        The destructor for each element is called, any used
        memory is deallocated, and shared ownership of the
        underlying storage is released.

        @par Complexity

        Linear in @ref size()
    */
    BOOST_JSON_DECL
    ~array();

    //------------------------------------------------------

    /** Construct an empty container

        The container and all inserted elements will use the
        @ref storage returned by @ref default_storage().

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    array() noexcept;

    /** Construct an empty container

        The container and all inserted elements will use the
        @ref storage owned by `sp`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    explicit
    array(storage_ptr sp) noexcept;

    /** Construct a container with `count` copies of `v`

        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The number of copies to insert.

        @param v The value to be inserted.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    array(
        size_type count,
        value_type const& v,
        storage_ptr sp = default_storage());

    /** Construct a container with `count` null values

        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Complexity

        Linear in `count`

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The number of copies to insert.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    array(
        size_type count,
        storage_ptr sp = default_storage());

    /** Construct a container with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Constraints

        `std::is_constructible_v<value, std::iterator_traits<InputIt>::value_type>`

        @par Complexity

        Linear in `std::distance(first, last)`

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<value,
                typename std::iterator_traits<
                    InputIt>::value_type>::value>::type
    #endif
    >
    array(
        InputIt first, InputIt last,
        storage_ptr sp = default_storage());

    /** Copy constructor

        Constructs the container with a copy of the contents
        of `other`.
        The container and all inserted elements will use the
        @ref storage returned by @ref default_storage().

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy
    */
    BOOST_JSON_DECL
    array(array const& other);

    /** Copy constructor

        Constructs the container with a copy of the contents
        of `other`.
        The container and all inserted elements will use the
        @ref storage owned by `sp`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    array(
        array const& other,
        storage_ptr sp);

    /** Pilfer constructor

        Constructs the container with the contents of `other`
        using pilfer semantics.
        Ownership of the @ref storage is transferred.

        @note

        After construction, the moved-from object may only be
        destroyed.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The container to pilfer

        @see
        
        Pilfering constructors are described in
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">Valueless Variants Considered Harmful</a>, by Peter Dimov.
    */
    BOOST_JSON_DECL
    array(pilfered<array> other) noexcept;

    /** Move constructor

        Constructs the container with the contents of `other`
        using move semantics. Ownership of the underlying
        memory is transferred.
        The container acquires shared ownership of the
        @ref storage used by `other`.

        @note

        After construction, the moved-from object behaves as
        if newly constructed with its current storage pointer.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The container to move
    */
    BOOST_JSON_DECL
    array(array&& other) noexcept;

    /** Move constructor

        Using `*sp` as the @ref storage for the new container,
        moves all the elements from `other`.

        @li If `*other.get_storage() == *sp`, ownership of the
        underlying memory is transferred in constant time, with
        no possibility of exceptions.
        After construction, the moved-from object behaves as if
        newly constructed with its current @ref storage pointer.

        @li If `*other.get_storage() != *sp`, an element-wise
        copy is performed. In this case, the moved-from container
        is not changed.

        The container and all inserted elements will use the
        specified storage.
        
        @par Complexity

        At most, linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to move

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    array(
        array&& other,
        storage_ptr sp);

    /** Constructs the container with the contents of an initializer list

        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    array(
        std::initializer_list<value> init,
        storage_ptr sp = default_storage());

    //------------------------------------------------------

    /** Copy assignment operator

        Replaces the contents with an element-wise copy of other.

        @par Complexity

        Linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy
    */
    array&
    operator=(array const& other)
    {
        copy(other);
        return *this;
    }

    /** Move assignment operator

        Replaces the contents with those of `other` using move
        semantics (the data in `other` is moved into this container).

        @li If `*other.get_storage() == get_storage()`,
        ownership of the  underlying memory is transferred in
        constant time, with no possibility of exceptions.
        After construction, the moved-from object behaves as if
        newly constructed with its current @ref storage pointer.

        @li If `*other.get_storage() != *sp`, an element-wise
        copy is performed. In this case the moved-from container
        is not modified, and exceptions may be thrown.

        @par Complexity

        At most, linear in `this->size()` plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to assign from
    */
    BOOST_JSON_DECL
    array&
    operator=(array&& other);

    /** Assign the contents of an initializer list

        Replaces the contents with the contents of an
        initializer list.

        @par Complexity

        Linear in `this->size()` plus `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to assign
    */
    array&
    operator=(
        std::initializer_list<value> init)
    {
        assign(init);
        return *this;
    }

    //------------------------------------------------------

    /** Return a pointer to the storage associated with the container

        Shared ownership of the @ref storage is propagated by
        the container to all of its children recursively.

        @par Complexity

        Constant.
    */
    storage_ptr const&
    get_storage() const noexcept
    {
        return sp_;
    }

    //------------------------------------------------------
    //
    // Element access
    //
    //------------------------------------------------------

    /** Access an element, with bounds checking

        Returns a reference to the element specified at
        location `pos`, with bounds checking. If pos is not
        within the range of the container, an exception of
        type `std::out_of_range` is thrown.

        @par Complexity

        Constant.

        @param pos A zero-based index

        @throws std::out_of_range `pos >= size()`
    */
    inline
    reference
    at(size_type pos);

    /** Access an element, with bounds checking

        Returns a reference to the element specified at
        location `pos`, with bounds checking. If pos is not
        within the range of the container, an exception of
        type `std::out_of_range` is thrown.

        @par Complexity

        Constant.

        @param pos A zero-based index

        @throws std::out_of_range `pos >= size()`
    */
    inline
    const_reference
    at(size_type pos) const;

    /** Access an element

        Returns a reference to the element specified at
        location `pos`. No bounds checking is performed.

        @par Precondition
        
        `pos >= size`

        @par Complexity

        Constant.

        @param pos A zero-based index
    */
    inline
    reference
    operator[](size_type pos);

    /** Access an element

        Returns a reference to the element specified at
        location `pos`. No bounds checking is performed.

        @par Precondition

        `pos >= size`

        @par Complexity

        Constant.

        @param pos A zero-based index
    */
    inline
    const_reference
    operator[](size_type pos) const;

    /** Access the first element

        Returns a reference to the first element.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    inline
    reference
    front();

    /** Access the first element

        Returns a reference to the first element.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    inline
    const_reference
    front() const;

    /** Access the last element

        Returns a reference to the last element.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    inline
    reference
    back();

    /** Access the last element

        Returns a reference to the last element.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    inline
    const_reference
    back() const;

    /** Access the underlying array directly

        Returns a pointer to the underlying array serving
        as element storage. The value returned is such that
        the range `[data(), data()+size())` is always a
        valid range, even if the container is empty.

        @par Complexity

        Constant.

        @note

        If `size() == 0`, the function may or may not return
        a null pointer.
    */
    inline
    value_type*
    data() noexcept;

    /** Access the underlying array directly

        Returns a pointer to the underlying array serving
        as element storage. The value returned is such that
        the range `[data(), data()+size())` is always a
        valid range, even if the container is empty.

        @par Complexity

        Constant.

        @note

        If `size() == 0`, the function may or may not return
        a null pointer.
    */
    inline
    value_type const*
    data() const noexcept;

    //------------------------------------------------------
    //
    // Iterators
    //
    //------------------------------------------------------

    /** Return an iterator to the first element

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    iterator
    begin() noexcept;

    /** Return an iterator to the first element

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    begin() const noexcept;

    /** Return an iterator to the first element

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    cbegin() const noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    iterator
    end() noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    end() const noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    cend() const noexcept;

    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    reverse_iterator
    rbegin() noexcept;

    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    rbegin() const noexcept;

    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    crbegin() const noexcept;

    /** Return a reverse iterator to the element following the last element of the reversed container

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    reverse_iterator
    rend() noexcept;

    /** Return a reverse iterator to the element following the last element of the reversed container

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    rend() const noexcept;

    /** Return a reverse iterator to the element following the last element of the reversed container

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    crend() const noexcept;

    //------------------------------------------------------
    //
    // Capacity
    //
    //------------------------------------------------------

    /** Check if the container has no elements

        Returns `true` if there are no elements in the container,
        i.e. @ref size() returns 0.

        @par Complexity

        Constant.
    */
    inline
    bool
    empty() const noexcept;

    /** Return the number of elements in the container

        This returns the number of elements in the container.
        The value returned may be different from the value
        returned from @ref capacity.

        @par Complexity

        Constant.
    */
    inline
    size_type
    size() const noexcept;

    /** Return the maximum number of elements the container can hold

        The maximum is an implementation-defined number dependent
        on system or library implementation. This value is a
        theoretical limit; at runtime, the actual maximum size
        may be less due to resource limits.

        @par Complexity

        Constant.
    */
    inline
    size_type
    max_size() const noexcept;

    /** Return the number of elements that can be held in currently allocated memory

        This number may be larger than the value returned
        by @ref size().

        @par Complexity

        Constant.
    */
    inline
    size_type
    capacity() const noexcept;

    /** Increase the capacity to at least a certain amount

        This increases the capacity of the array to a value
        that is greater than or equal to `new_capacity`. If
        `new_capacity > capacity()`, new memory is allocated.
        Otherwise, the call has no effect. The number of
        elements and therefore the @ref size() of the container
        is not changed.

        @note

        If new memory is allocated, all iterators including any
        past-the-end iterators, and all references to the
        elements are invalidated. Otherwise, no iterators or
        references are invalidated.

        @par Complexity

        At most, linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @throw std::length_error `new_capacity > max_size()`

        @param new_capacity The new capacity of the array.
    */
    BOOST_JSON_DECL
    void
    reserve(size_type new_capacity);

    /** Request the removal of unused capacity

        This performs a non-binding request to reduce @ref capacity()
        to @ref size(). The request may or may not be fulfilled. If
        reallocation occurs, all iterators including any past-the-end
        iterators, and all references to the elements are invalidated.
        Otherwise, no iterators or references are invalidated.

        @par Complexity

        At most, linear in @ref size().

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    shrink_to_fit() noexcept;

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Clear the contents

        Erases all elements from the container. After this
        call, @ref size() returns zero but @ref capacity()
        is unchanged. All references, pointers, or iterators
        referring to contained elements are invalidated. Any
        past-the-end iterators are also invalidated.

        @par Complexity

        Linear in @ref size().
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    /** Insert elements before the specified location

        This inserts a copy of `v` before `pos`.
        If `capacity() < size() + 1`, a reallocation
        occurs first, and all iterators and references
        are invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Constant plus linear in `std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the content will
        be inserted. This may be the @ref end() iterator.

        @param v The value to insert. A copy will be made
        using container's associated @ref storage.

        @return An iterator to the inserted value
    */
    iterator
    insert(
        const_iterator pos,
        value_type const& v)
    {
        return emplace_impl(pos, v);
    }

    /** Insert elements before the specified location

        This inserts `v` before `pos` via move-construction.
        If `capacity() < size() + 1`, a reallocation occurs
        first, and all iterators and references are
        invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Constant plus linear in `std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the content will
        be inserted. This may be the @ref end() iterator.

        @param v The value to insert. Ownership of the
        value will be transferred via move construction,
        using the container's associated @ref storage.

        @return An iterator to the inserted value
    */
    iterator
    insert(
        const_iterator pos,
        value_type&& v)
    {
        return emplace_impl(
            pos, std::move(v));
    }

    /** Insert elements before the specified location

        This inserts `count` copies of `v` before `pos`.
        If `capacity() < size() + count`, a reallocation
        occurs first, and all iterators and references are
        invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Linear in `count + std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the content will
        be inserted. This may be the @ref end() iterator.

        @param count The number of copies to insert.
        
        @param v The value to insert. Copies will be made
        using container's associated @ref storage.

        @return An iterator to the first inserted value,
        or `pos` if `count == 0`.
    */
    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator pos,
        size_type count,
        value_type const& v);

    /** Insert elements before the specified location

        The elements in the range `[first, last)` are
        inserted in order.
        If `capacity() < size() + std::distance(first, last)`,
        a reallocation occurs first, and all iterators and
        references are invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Precondition

        `first` and `last` are not iterators into `*this`.

        @par Constraints

        `not std::is_convertible_v<InputIt, value>`

        @par Mandates

        `std::is_constructible_v<value, std::iterator_traits<InputIt>::value_type>`

        @par Complexity
        
        Linear in `std::distance(first, last) + std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the content will
        be inserted. This may be the @ref end() iterator.
        
        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.

        @return An iterator to the first inserted value, or
        `pos` if `first == last`.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<value,
                typename std::iterator_traits<
                    InputIt>::value_type>::value>::type
    #endif
    >
    iterator
    insert(
        const_iterator pos,
        InputIt first, InputIt last);

    /** Insert elements before the specified location

        The elements in the initializer list `init` are
        inserted in order.
        If `capacity() < size() + init.size()`,
        a reallocation occurs first, and all iterators and
        references are invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Linear in `init.size() + std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the content will
        be inserted. This may be the @ref end() iterator.
        
        @param init The initializer list to insert

        @return An iterator to the first inserted value, or
        `pos` if `init.size() == 0`.
    */
    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator pos,
        std::initializer_list<value> init);

    /** Insert a constructed element in-place

        Inserts a new element into the container directly before
        `pos`. The element is constructed using placement-new
        with the parameter `std::forward<Arg>(arg)`.
        If `capacity() < size() + 1`,
        a reallocation occurs first, and all iterators and
        references are invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Constant plus linear in `std::distance(pos, end())`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the element will
        be inserted. This may be the @ref end() iterator.
        
        @param arg The argument to forward to the @ref value
        constructor.

        @return An iterator to the inserted element
    */
    template<class Arg>
    iterator
    emplace(
        const_iterator pos,
        Arg&& arg);

    /** Erases elements from the container

        The element at `pos` is removed.

        @par Complexity

        Constant plus linear in `std::distance(pos, end())`

        @par Exception Safety

        No-throw guarantee.

        @param pos Iterator to the element to remove

        @return Iterator following the last removed element.
        If the iterator `pos` refers to the last element,
        the @ref end() iterator is returned.
    */
    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos) noexcept;

    /** Erases elements from the container

        The elements in the range `[first, last)` are removed.

        @par Complexity

        Linear in `std::distance(first, last) + std::distance(pos, end())`

        @par Exception Safety

        No-throw guarantee.

        @param first An iterator pointing to the first
        element to erase, or pointing to the end of the range.

        @param last An iterator pointing to one past the
        last element to erase, or pointing to the end of the
        range.

        @return Iterator following the last removed element.
        If the iterator `pos` refers to the last element,
        the @ref end() iterator is returned.
    */
    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last) noexcept;

    /** Add an element to the end.

        This appends a copy of `v` to the container's
        elements.
        If `capacity() < size() + 1`, a reallocation
        occurs first, and all iterators and references
        are invalidated. Any past-the-end iterators are
        always invalidated.

        @par Complexity
        
        Amortized constant.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param v The value to insert. A copy will be made
        using container's associated @ref storage.
    */
    void
    push_back(value_type const& v)
    {
        emplace_impl(end(), v);
    }

    /** Add an element to the end.

        This appends `v` to the container's elements via
        move-construction.
        If `capacity() < size() + 1`, a reallocation
        occurs first, and all iterators and references
        are invalidated. Any past-the-end iterators are
        always invalidated.

        @par Complexity
        
        Amortized constant.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param v The value to insert. Ownership of the
        value will be transferred via move construction,
        using the container's associated @ref storage.
    */
    void
    push_back(value_type&& v)
    {
        emplace_impl(end(), std::move(v));
    }

    /** Append a constructed element in-place

        Appends a new element to the end of the container's
        list of elements.
        The element is constructed using placement-new
        with the parameter `std::forward<Arg>(arg)`.
        If `capacity() < size() + 1`,
        a reallocation occurs first, and all iterators and
        references are invalidated.
        Otherwise, only the iterators and references from
        the insertion point forward are invalidated. All
        past-the-end iterators are also invalidated.

        @par Complexity
        
        Amortized constant.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param arg The argument to forward to the @ref value
        constructor.

        @return A reference to the inserted element
    */
    template<class Arg>
    reference
    emplace_back(Arg&& arg);

    /** Remove the last element

        The last element of the container is erased.

        @par Precondition

        `not empty()`

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    pop_back() noexcept;

    /** Change the number of elements stored

        Resizes the container to contain `count` elements.
        If `capacity() < size() + count`, a reallocation
        occurs first, and all iterators and references
        are invalidated. Any past-the-end iterators are
        always invalidated.

        @li If `size() > count`, the container is reduced
        to its first `count` elements.

        @li If `size() < count`, additional null values
        are appended.
        
        @par Complexity

        Linear in `abs(size() - count)`, plus the cost of
        reallocation if @ref capacity() is less than `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The new size of the container.
    */
    BOOST_JSON_DECL
    void
    resize(size_type count);

    /** Change the number of elements stored

        Resizes the container to contain `count` elements.
        If `capacity() < size() + count`, a reallocation
        occurs first, and all iterators and references
        are invalidated. Any past-the-end iterators are
        always invalidated.

        @li If `size() > count`, the container is reduced
        to its first `count` elements.

        @li If `size() < count`, additional copies of `v`
        are appended.
        
        @par Complexity

        Linear in `abs(size() - count)`, plus the cost of
        reallocation if @ref capacity() is less than `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The new size of the container.

        @param v The @ref value to copy into the new elements.
    */
    BOOST_JSON_DECL
    void
    resize(
        size_type count,
        value_type const& v);

    /** Swap the contents

        Exchanges the contents of this container with another
        container. Ownership of the respective @ref storage
        objects is not transferred.

        @li If `*other.get_storage() == *sp`, ownership of the
        underlying memory is swapped in constant time, with
        no possibility of exceptions. All iterators and
        references remain valid.

        @li If `*other.get_storage() != *sp`, the contents are
        logically swapped by making copies, which can throw.
        In this case all iterators and references are invalidated.

        @par Complexity

        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to swap with
    */
    BOOST_JSON_DECL
    void
    swap(array& other);

private:
    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    template<class InputIt>
    array(
        InputIt first, InputIt last,
        storage_ptr sp,
        std::input_iterator_tag);

    template<class InputIt>
    array(
        InputIt first, InputIt last,
        storage_ptr sp,
        std::forward_iterator_tag);

    template<class InputIt>
    iterator
    insert(
        const_iterator pos,
        InputIt first, InputIt last,
        std::input_iterator_tag);

    template<class InputIt>
    iterator
    insert(
        const_iterator pos,
        InputIt first, InputIt last,
        std::forward_iterator_tag);

    template<class Arg>
    iterator
    emplace_impl(
        const_iterator pos,
        Arg&& arg);

    BOOST_JSON_DECL
    void
    copy(array const& other);

    BOOST_JSON_DECL
    void
    assign(std::initializer_list<value> init);

    inline
    storage_ptr
    release_storage() noexcept;

    friend class value;
};

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// headers for this file are at the bottom of value.hpp

#endif
