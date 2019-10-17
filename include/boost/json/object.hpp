//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_OBJECT_HPP
#define BOOST_JSON_OBJECT_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace boost {
namespace json {

class value;

//----------------------------------------------------------

/** An associative container of key to JSON value pairs

    This is an associative container that contains key-value
    pairs with unique keys. Search, insertion, and removal of
    elements have average constant-time complexity.
    <br>
    Internally the elements are initially kept in insertion
    order, but this order can be changed manually by specifying
    where new or moved elements go. Elements are also organized
    into buckets. Which bucket an element is placed into depends
    entirely on the hash of its key. This allows fast access to
    individual elements, since once the hash is computed, it
    refers to the exact bucket the element is placed into.

    @par Storage

    All elements stored in the container will use the same storage that
    was used to construct the container, including recursive children
    of those elements.

    @par Satisfies

    @ref object models
        <em>Container</em>,
        <em>ReversibleContainer</em>,
        <em>SequenceContainer</em>, and
        <em>UnorderedAssociativeContainer</em>.
*/
class object
{
public:
    /// The type of keys
    using key_type = string_view;

    /// The type of mapped values
    using mapped_type = value;

    /// The element type
    using value_type =
        std::pair<key_type const, value>;

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference =
        std::pair<key_type const, value&>;

    /// A const reference to an element
    using const_reference =
        std::pair<key_type const, value const&>;

#ifdef GENERATING_DOCUMENTATION
    /** The hash function used for keys

        Objects of this type are used to calculate the
        hash for a key.

        @par Satisfies

        Meets the requirements of __Hash__
    */
    using hasher = __see_below__;

    /** The key comparison function

        Objects of this type are used to compare keys
        for equality.
    */
    using key_equal = __see_below__;

    using pointer = __implementation_defined__;
    using const_pointer = __implementation_defined__;
    using iterator = __implementation_defined__;
    using const_iterator = __implementation_defined__;

    using node_type = __see_below__;

    /** Describes the result of inserting a node handle.

        Objects of this type are returned by overloads
        of @ref insert which accept a @ref node_type
        parameter.
    */
    struct insert_return_type
    {
        /** An iterator pointing to the result of the insertion.

            If @ref inserted is `true`, this will point to the
            newly inserted element. Otherwise it will point to
            the already existing element with the matching key.
        */
        iterator position;

        /** The empty node handle, or the original node handle

            If @ref inserted is `true`, this will be an empty
            node handle. Otherwise, it will have ownership of
            the original node used to perform the insertion.
        */
        node_type node;

        /** Indicates if the insertion was successful.

            This will be `true` if the node was inserted, or
            `false` if a matching key was already in the container.
        */
        bool inserted;
    };

#else
    class hasher;
    class key_equal;
    class pointer;
    class const_pointer;
    class iterator;
    class const_iterator;
    class node_type;
    struct insert_return_type;
#endif

#if 0
    /// A reverse random access iterator to an element
    using reverse_iterator =
        std::reverse_iterator<iterator>;

    /// A const reverse random access iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;
#endif

    /// The value type of initializer lists
    using init_value = std::pair<key_type, value>;

private:
    struct list_hook;
    struct element;
    struct table;
    class undo_range;

    storage_ptr sp_;
    table* tab_ = nullptr;

    static
    float
    max_load_factor() noexcept
    {
        return 1.f;
    }

    template<class T>
    using is_inputit = typename std::enable_if<
        std::is_convertible<typename
            std::iterator_traits<T>::value_type,
            const_reference>::value>::type;

public:
    //------------------------------------------------------

    /** Destroy the container

        The destructor for each element is called, any used
        memory is deallocated, and shared ownership of the
        underlying storage is released.

        @par Complexity

        Linear in @ref size()
    */
    BOOST_JSON_DECL
    ~object();

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
    object() noexcept;

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
    object(
        storage_ptr sp) noexcept;

    /** Construct an empty container

        Storage for indexing the elements by hash is
        allocated for at least `count` elements.
        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The minimum number of elements for
        which space in the index is reserved.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        size_type count,
        storage_ptr sp = default_storage());

    /** Construct with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.
        Storage for indexing the elements by hash is
        allocated for at least `count` elements, or an
        implementation defined amount if this argument
        is omitted.
        The container and all inserted elements will use the
        @ref storage owned by `sp`,
        or the default parameter value returned by
        @ref default_storage() if this argument is omitted.

        @par Constraints

        `std::is_constructible_v<const_reference, std::iterator_traits<InputIt>::value_type>`

        @par Complexity

        Linear in `std::distance(first, last)`

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @param count The minimum number of elements for
        which space in the index is reserved.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    object(
        InputIt first,
        InputIt last,
        size_type count = 0,
        storage_ptr sp = default_storage());

    /** Move constructor

        Construct the container with the contents of `other`
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
    object(object&& other) noexcept;

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

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to move

        @param sp A pointer to the @ref storage to use. The
        container array will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    object(
        object&& other,
        storage_ptr sp);

    /** Pilfer constructor

        Construct the container with the contents of `other`
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
    object(pilfered<object> other) noexcept;

    /** Copy constructor

        Construct the container with a copy of the contents
        of `other.
        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy
    */
    BOOST_JSON_DECL
    object(
        object const& other);

    /** Copy constructor

        Construct the container with a copy of the contents
        of `other.
        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    object(
        object const& other,
        storage_ptr sp);
       
    /** Construct the container with an initializer list

        Storage for indexing the elements by hash is
        allocated for at least `count` elements, or an
        implementation defined amount if this argument
        is omitted.
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

        @param count The minimum number of elements for
        which space in the index is reserved.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            init_value> init,
        size_type count = 0,
        storage_ptr sp = default_storage());

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

        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to assign from
    */
    BOOST_JSON_DECL
    object&
    operator=(object&& other);

    /** Copy assignment operator

        Replaces the contents with an element-wise copy of `other`.

        @par Complexity

        Linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to copy
    */
    BOOST_JSON_DECL
    object&
    operator=(object const& other);

    /** Assign the contents of an initializer list

        Replaces the contents with the contents of an
        initializer list.

        @par Complexity

        Linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to assign
    */
    BOOST_JSON_DECL
    object&
    operator=(
        std::initializer_list<
            init_value> init);

    /** Return a pointer to the storage associated with the container

        Shared ownership of the @ref storage is propagated by
        the container to all of its children recursively.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    storage_ptr const&
    get_storage() const noexcept;

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
    BOOST_JSON_DECL
    iterator
    begin() noexcept;

    /** Return an iterator to the first element

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    begin() const noexcept;

    /** Return an iterator to the first element

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    cbegin() const noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    iterator
    end() noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    end() const noexcept;

    /** Return an iterator to the element following the last element

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    cend() const noexcept;

#if 0
    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    reverse_iterator
    rbegin() noexcept;

    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_reverse_iterator
    rbegin() const noexcept;

    /** Return a reverse iterator to the first element of the reversed container

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
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
    BOOST_JSON_DECL
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
    BOOST_JSON_DECL
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
    BOOST_JSON_DECL
    const_reverse_iterator
    crend() const noexcept;
#endif

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
    BOOST_JSON_DECL
    bool
    empty() const noexcept;

    /** Return the number of elements in the container

        This returns the number of elements in the container.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
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
    BOOST_JSON_DECL
    size_type
    max_size() const noexcept;

    /** Returns the maximum number of elements the container can support before rehashing.

        This returns the number of elements which may exist in
        the container, after which the container will require a
        rehash. It effectively returns `size() / max_load_factor()`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    size_type
    capacity() const noexcept;

    /** Reserve space for at least the specified number of elements.

        Sets the number of buckets to the number needed to
        accomodate at least `n` elements without exceeding
        the maximum load factor, and rehashes the container;
        i.e. puts the elements into appropriate buckets
        considering that total number of buckets has
        changed.

        @par Complexity

        Average case linear in the size of the container,
        worst case quadratic.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param n The new minimum capacity of the container
    */
    BOOST_JSON_DECL
    void
    reserve(size_type n);

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Clear the contents

        Erases all elements from the container. After this
        call, @ref size() returns zero.
        All references, pointers, or iterators referring
        to contained elements are invalidated. Any past-the-end
        iterators are also invalidated.

        @par Complexity

        Linear in @ref size().
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    template<class P = value_type
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<value_type,
                P&&>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(P&& p);

    template<class P = value_type
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<value_type,
                P&&>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(
        const_iterator pos,
        P&& p);

    /** Insert elements at the end

        The elements in the range `[first, last)` are
        inserted at the end, in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Precondition

        `first` and `last` are not iterators into `*this`.

        @par Constraints

        `std::is_constructible_v<const_reference, std::iterator_traits<InputIt>::value_type>`

        @par Complexity
        
        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    void
    insert(InputIt first, InputIt last);

    /** Insert elements before the specified location

        The elements in the range `[first, last)` are
        inserted before the element pointed to by `pos`.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Precondition

        `first` and `last` are not iterators into `*this`.

        @par Constraints

        `std::is_constructible_v<const_reference, std::iterator_traits<InputIt>::value_type>`

        @par Complexity
        
        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param pos Iterator before which the new elements will
        be inserted. This may be the @ref end() iterator.

        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    void
    insert(
        const_iterator pos,
        InputIt first, InputIt last);

    /** Insert elements at the end

        The elements in the initializer list are
        inserted at the end, in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Complexity
        
        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    void
    insert(std::initializer_list<
        init_value> init);

    /** Insert elements before the specified location

        The elements in the initializer list are
        inserted before the element pointed to by `pos`.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Complexity
        
        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param pos Iterator before which the new elements will
        be inserted. This may be the @ref end() iterator.

        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    void
    insert(
        const_iterator pos,
        std::initializer_list<
            init_value> init);

    /** Insert a node

        Attempts to inserts the node contained in `nh`:

        @li If `nh.empty()`, does nothing and returns the
        @ref end() iterator. Otherwise,

        @li If `! nh.empty()`, inserts the element owned by
        `nh` into the container, if a matching key equivalent
        to `nh.key()` does not already exist. The element is
        inserted after all the existing elements.

        @par Expects

        `nh.get_storage() == *this->get_storage()`.

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param nh A compatible node handle.

        @returns An @ref insert_return_type describing the result
        of the insertion.
    */
    BOOST_JSON_DECL
    insert_return_type
    insert(node_type&& nh);

    /** Insert a node

        Attempts to inserts the node contained in `nh`:

        @li If `nh.empty()`, does nothing and returns the
        @ref end() iterator. Otherwise,

        @li If `! nh.empty()`, inserts the element owned by
        `nh` into the container, if a matching key equivalent
        to `nh.key()` does not already exist. The element is
        inserted before `pos`.

        @par Expects

        `nh.get_storage() == *this->get_storage()`.

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the new elements will
        be inserted. This may be the @ref end() iterator.

        @param nh A compatible node handle.

        @returns An @ref insert_return_type describing the result
        of the insertion.
    */
    BOOST_JSON_DECL
    insert_return_type
    insert(
        const_iterator pos,
        node_type&& nh);

    /** Insert an element or assign an element if the key already exists

        If the key equivalent to `key` already exists in the
        container. assigns `std::forward<M>(obj)` to the
        `mapped type` corresponding to the key. Otherwise,
        inserts the new value at the end as if by insert,
        constructing it from
        `value_type(key, std::forward<M>(obj))`.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param key The key used for lookup and insertion

        @param obj The value to insert or assign

        @returns A `pair` where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        key_type key, M&& obj);

    /** Insert an element or assign an element if the key already exists

        If the key equivalent to `key` already exists in the
        container. assigns `std::forward<M>(obj)` to the
        `mapped type` corresponding to the key. Otherwise,
        inserts the new value before the element pointed to
        by `pos` as if by insert, constructing it from
        `value_type(key, std::forward<M>(obj))`.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the new elements will
        be inserted. This may be the @ref end() iterator.

        @param key The key used for lookup and insertion

        @param obj The value to insert or assign

        @returns A pair where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        const_iterator pos,
        key_type key,
        M&& obj);

    /** Construct an element in place

        Inserts a new element into the container constructed
        in-place with the given argument if there is no
        element with the key in the container.
        The element is inserted after all the existing
        elements.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param key The key used for lookup and insertion

        @param arg The argument used to construct the value.
        This will be passed as `std::forward<Arg>(arg)` to
        the @ref value constructor.

        @returns A pair where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class Arg>
    std::pair<iterator, bool>
    emplace(key_type key, Arg&& arg);

    /** Construct an element in place

        Inserts a new element into the container constructed
        in-place with the given argument if there is no
        element with the key in the container.
        The element is inserted before `pos`.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear
        in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the new element will
        be inserted. This may be the @ref end() iterator.

        @param key The key used for lookup and insertion

        @param arg The argument used to construct the value.
        This will be passed as `std::forward<Arg>(arg)` to
        the @ref value constructor.

        @returns A pair where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class Arg>
    std::pair<iterator, bool>
    emplace(
        const_iterator pos,
        key_type key, Arg&& arg);

    /** Erase an element

        Remove the element pointed to by `pos`, which must
        be valid and dereferenceable. Thus the @ref end()
        iterator (which is valid but cannot be dereferenced)
        cannot be used as a value for `pos`.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @returns The number of elements removed, which can
        be either 0 or 1.
    */
    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos);
    
    /** Erase a range of elements

        Removes the elements in the range `[first, last)`,
        which must be a valid range in `*this`.
        References and iterators to the erased elements are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity

        Average case linear in `std::distance(first, last)`,
        worst case linear in @ref size().

        @param first The beginning of the range to remove.
        
        @param last The end of the range to remove.
        
        @returns An iterator following the last removed element.
    */
    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    /** Erase an element

        Remove the element which matches `key`, if it exists.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @returns The number of elements removed, which can
        be either 0 or 1.
    */
    BOOST_JSON_DECL
    size_type
    erase(key_type key);

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
    swap(object& other);

    /** Extract a node

        If the container has an element that matches `key`,
        unlinks the node that contains that element from
        the container and returns a node handle that owns
        it. Otherwise, returns an empty node handle.
        <br>
        Extracting a node invalidates only the iterators
        to the extracted element, and preserves the relative
        order of the elements that are not erased. Pointers
        and references to the extracted element remain valid,
        but cannot be used while element is owned by a node
        handle: they become usable if the element is inserted
        into a container.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @param pos The key of the element to find
    */
    BOOST_JSON_DECL
    node_type
    extract(const_iterator pos);

    /** Extract a node

        If the container has an element that matches `key`,
        unlinks the node that contains that element from
        the container and returns a node handle that owns
        it. Otherwise, returns an empty node handle.
        <br>
        Extracting a node invalidates only the iterators
        to the extracted element, and preserves the relative
        order of the elements that are not erased. Pointers
        and references to the extracted element remain valid,
        but cannot be used while element is owned by a node
        handle: they become usable if the element is inserted
        into a container.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    node_type
    extract(key_type key);

#if 0
    // TODO
    BOOST_JSON_DECL
    void
    merge(object& source);

    BOOST_JSON_DECL
    void
    merge(object&& source);
#endif

    //------------------------------------------------------
    //
    // Lookup
    //
    //------------------------------------------------------

    /** Access the specified element, with bounds checking.

        Returns a reference to the mapped value of the element
        that matches `key`, otherwise throws.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @throws std::out_of_range if no such element exists.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    value&
    at(key_type key);

    /** Access the specified element, with bounds checking.

        Returns a constant reference to the mapped value of
        the element that matches `key`, otherwise throws.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @throws std::out_of_range if no such element exists.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    value const&
    at(key_type key) const;

    /** Access or insert the specified element

        Returns a reference to the value that is mapped
        to a key equivalent to key, performing an insertion
        of a null value if such key does not already exist.
        <br>
        If an insertion occurs and results in a rehashing of
        the container, all iterators are invalidated. Otherwise
        iterators are not affected. References are not
        invalidated. Rehashing occurs only if the new
        number of elements is greater than
        @ref capacity().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        If an exception is thrown by any operation, the
        insertion has no effect.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    value&
    operator[](key_type key);
    
    /** Count the number of elements with a specific key

        This function returns the count of the number of
        elements match `key`. The only possible return values
        are 0 and 1.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    size_type
    count(key_type key) const noexcept;

    /** Find an element with a specific key

        This function returns an iterator to the element
        matching `key` if it exists, otherwise returns
        @ref end().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    iterator
    find(key_type key) noexcept;

    /** Find an element with a specific key

        This function returns a constant iterator to
        the element matching `key` if it exists,
        otherwise returns @ref end().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    const_iterator
    find(key_type key) const noexcept;

    /** Check if the container contains an element with a specific key

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    bool
    contains(key_type key) const noexcept;

    //------------------------------------------------------
    //
    // Observers
    //
    //------------------------------------------------------

    /** Returns the function used to hash the keys

        The returned function object has this equivalent
        signature:
        @code
        struct hasher
        {
            std::size_t operator()(string_view) const noexcept;
        };
        @endcode

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    hasher
    hash_function() const noexcept;

    /** Returns the function that compares keys for equality.

        The returned function object has this equivalent
        signature:
        @code
        struct key_equal
        {
            bool operator()(string_view, string_view) const noexcept;
        };
        @endcode

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    key_equal
    key_eq() const noexcept;

private:
    struct construct_base;

    inline
    static
    size_type
    constrain_hash(
        std::size_t hash,
        size_type bucket_count) noexcept;

    inline
    size_type
    bucket(key_type key) const;

    inline
    void
    rehash(size_type bucket_count);

    inline
    void
    remove(element* e);

    template<class Arg>
    element*
    allocate(
        key_type key,
        Arg&& arg);

    template<class InputIt>
    void
    insert_range(
        const_iterator pos,
        InputIt first,
        InputIt last,
        size_type size_hint);

    BOOST_JSON_DECL
    element*
    allocate_impl(
        key_type key,
        construct_base const& place_new);

    BOOST_JSON_DECL
    element*
    allocate(std::pair<
        string_view, value const&> const& p);

    BOOST_JSON_DECL
    auto
    find_impl(key_type key) const noexcept ->
        std::pair<element*, std::size_t>;

    BOOST_JSON_DECL
    void
    insert(
        const_iterator pos,
        std::size_t hash,
        element* e);
};

/** Swap this container with the contents of another container.
*/
BOOST_JSON_DECL
void
swap(object& lhs, object& rhs);

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// headers for this file are at the bottom of value.hpp

#endif
