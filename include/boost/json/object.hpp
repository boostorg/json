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

//------------------------------------------------------------------------------

/** An associative container of key to JSON value pairs

    This is an associative container that contains key-value
    pairs with unique keys. Search, insertion, and removal of
    elements have average constant-time complexity.

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

    @ref object meets the requirements of
        <em>Container</em>,
        <em>ReversibleContainer</em>,
        <em>SequenceContainer</em>, and
        <em>UnorderedAssociativeContainer</em>.
*/
class object
{
    struct list_hook;
    struct element;
    struct table;
    class undo_range;

    storage_ptr sp_;
    table* tab_ = nullptr;
    float mf_ = 1.0;

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
    using local_iterator = __implementation_defined__;
    using const_local_iterator = __implementation_defined__;

    using node_type = __see_below__;
    using insert_return_type = __see_below__;

#else
    class hasher;
    class key_equal;
    class pointer;
    class const_pointer;
    class iterator;
    class const_iterator;
    class local_iterator;
    class const_local_iterator;
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

    //--------------------------------------------------------------------------

    /** Destroy the container

        The destructor for each element is called, any used
        memory is deallocated, and shared ownership of the
        underlying storage is released.

        @par Complexity

        Linear in @ref size()
    */
    BOOST_JSON_DECL
    ~object();

    //--------------------------------------------------------------------------

    /** Construct an empty container

        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    object() noexcept;

    /** Construct an empty container

        The container and all inserted elements will use the
        storage pointed to by `sp`.

        @par Complexity

        Constant.

        @param sp A pointer to the @ref storage to use.
        The container will acquire shared ownership of the pointer.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    explicit
    object(
        storage_ptr sp) noexcept;

    /** Construct an empty container

        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Constant.

        @param bucket_count The number of buckets to allocate.

        @par Exception Safety

        Strong guarantee.
    */
    BOOST_JSON_DECL
    explicit
    object(
        size_type bucket_count);

    /** Construct an empty container

        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        storage pointed to by `sp`.

        @par Complexity

        Constant.

        @param bucket_count The number of buckets to allocate.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.

        @par Exception Safety

        Strong guarantee.
    */
    BOOST_JSON_DECL
    object(
        size_type bucket_count,
        storage_ptr sp);

    /** Construct a container with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.
        The container and all inserted elements will use the
        default storage.

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

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
    #endif
    >
    BOOST_JSON_DECL
    object(
        InputIt first,
        InputIt last);

    /** Construct a container with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.
        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        default storage.

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

        @param bucket_count A lower limit on the number of
        buckets to allocate.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
    #endif
    >
    BOOST_JSON_DECL
    object(
        InputIt first,
        InputIt last,
        size_type bucket_count);

    /** Construct a container with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.
        The container and all inserted elements will use the
        storage pointed to by `sp`.

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

        @param sp A pointer to the @ref storage to use. The
        container  will acquire shared ownership of the pointer.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
    #endif
    >
    BOOST_JSON_DECL
    object(
        InputIt first,
        InputIt last,
        storage_ptr sp);

    /** Construct a container with the contents of a range

        The elements in the range `[first, last)` are
        inserted in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.
        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        storage pointed to by `sp`.

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

        @param bucket_count A lower limit on the number of
        buckets to allocate.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
    #endif
    >
    BOOST_JSON_DECL
    object(
        InputIt first,
        InputIt last,
        size_type bucket_count,
        storage_ptr sp);

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

        At most, linear in `other.size()`.

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
    object(pilfered<object> other) noexcept;

    /** Copy constructor

        Constructs the container with a copy of the contents
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

        Constructs the container with a copy of the contents
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

    /** Constructs the container with the contents of an initializer list

        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            init_value> init);

    /** Constructs the container with the contents of an initializer list

        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        default storage.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert

        @param bucket_count A lower limit on the number of
        buckets to allocate.
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            init_value> init,
        size_type bucket_count);

    /** Constructs the container with the contents of an initializer list

        The container and all inserted elements will use the
        @ref storage pointed to by `sp`.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            init_value> init,
        storage_ptr sp);
        
    /** Constructs the container with the contents of an initializer list

        Storage is allocated for at least `bucket_count`
        buckets.
        The container and all inserted elements will use the
        @ref storage pointed to by `sp`.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert

        @param bucket_count A lower limit on the number of
        buckets to allocate.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            init_value> init,
        size_type bucket_count,
        storage_ptr sp);

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

        Constant, or linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to assign from
    */
    BOOST_JSON_DECL
    object&
    operator=(object&& other);

    /** Copy assignment operator

        Replaces the contents with an element-wise copy of other.

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

    //--------------------------------------------------------------------------
    //
    // Iterators
    //
    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------------------------------------

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
        The value returned may be different from the value
        returned from @ref capacity.

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

    //--------------------------------------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------------------------------------

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
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
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
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                const_reference>::value>::type
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
        
        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    void
    insert(
        const_iterator pos,
        std::initializer_list<
            init_value> init);

    BOOST_JSON_DECL
    insert_return_type
    insert(node_type&& nh);

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
        constructing it from `value_type(key, std::forward<M>(obj))`.

        If the insertion occurs and results in a rehashing of
        the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements is
        greater than `max_load_factor()*bucket_count()`.

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

        If the insertion occurs and results in a rehashing of
        the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements is
        greater than `max_load_factor()*bucket_count()`.

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param pos Iterator before which the new elements will
        be inserted. This may be the @ref end() iterator.

        @param key The key used for lookup and insertion

        @param obj The value to insert or assign

        @returns A pair `p` where `p.first` is an iterator
        to the existing or inserted element, and `p.second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        const_iterator pos,
        key_type key,
        M&& obj);

    template<class Arg>
    std::pair<iterator, bool>
    emplace(key_type key, Arg&& arg);

    template<class Arg>
    std::pair<iterator, bool>
    emplace(
        const_iterator pos,
        key_type key, Arg&& arg);

    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos);
    
    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    BOOST_JSON_DECL
    size_type
    erase(key_type key);

    BOOST_JSON_DECL
    void
    swap(object& other);

    BOOST_JSON_DECL
    node_type
    extract(const_iterator pos);

    BOOST_JSON_DECL
    node_type
    extract(key_type key);

    BOOST_JSON_DECL
    void
    merge(object& source);

    BOOST_JSON_DECL
    void
    merge(object&& source);

    //--------------------------------------------------------------------------
    //
    // Lookup
    //
    //--------------------------------------------------------------------------
    
    BOOST_JSON_DECL
    value&
    at(key_type key);
    
    BOOST_JSON_DECL
    value const&
    at(key_type key) const;

    BOOST_JSON_DECL
    value&
    operator[](key_type key);
    
    BOOST_JSON_DECL
    value const&
    operator[](key_type key) const;

    BOOST_JSON_DECL
    size_type
    count(key_type key) const noexcept;

    BOOST_JSON_DECL
    iterator
    find(key_type key) noexcept;

    BOOST_JSON_DECL
    const_iterator
    find(key_type key) const noexcept;

    BOOST_JSON_DECL
    bool
    contains(key_type key) const noexcept;

    //--------------------------------------------------------------------------
    //
    // Bucket Interface
    //
    //--------------------------------------------------------------------------

    /** Return an iterator to the beginning of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    local_iterator
    begin(size_type n) noexcept;

    /** Return a constant iterator to the beginning of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    const_local_iterator
    begin(size_type n) const noexcept;

    /** Return a constant iterator to the beginning of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    const_local_iterator
    cbegin(size_type n) noexcept;

    /** Return a iterator to the end of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.
        This element acts as a placeholder, attempting to
        access the pointed-to value results in undefined
        behavior.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    local_iterator
    end(size_type n)  noexcept;

    /** Return a constant iterator to the end of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.
        This element acts as a placeholder, attempting to
        access the pointed-to value results in undefined
        behavior.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    const_local_iterator
    end(size_type n) const noexcept;

    /** Return a constant iterator to the end of the specified bucket

        If the container is empty, the returned iterator
        will be equal to `end(n)`.
        This element acts as a placeholder, attempting to
        access the pointed-to value results in undefined
        behavior.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    const_local_iterator
    cend(size_type n) noexcept;

    /** Returns the number of buckets in the container

        This returns the number of buckets, which may be
        zero if the container is empty.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    size_type
    bucket_count() const noexcept;

    /** Returns the maximum number of buckets in the container

        The value returned is dependent on system or
        library implementation limitations.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    size_type
    max_bucket_count() const noexcept;

    /** Returns the number of elements in a bucket

        This returns the number of elements in the
        bucket with index `n`.

        @par Complexity

        Constant.

        @param n The zero-based index of the bucket to access.
    */
    BOOST_JSON_DECL
    size_type
    bucket_size(size_type n) const noexcept;

    /** Returns the index of the bucket corresponding to a key

        This returns the zero based index of the bucket for
        a matching key. The returned value is valid only for
        instances of the container for which @ref bucket_count()
        returns the same value.
        The behavior is undefined if @ref bucket_count() is zero.

        @param key The value of the key to examine
    */
    BOOST_JSON_DECL
    size_type
    bucket(key_type key) const noexcept;

    //--------------------------------------------------------------------------
    //
    // Hash Policy
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    float
    load_factor() const noexcept;

    BOOST_JSON_DECL
    float
    max_load_factor() const noexcept
    {
        return mf_;
    }

    BOOST_JSON_DECL
    void
    max_load_factor(float ml) noexcept
    {
        mf_ = ml;
    }

    BOOST_JSON_DECL
    void
    rehash(size_type count);

    BOOST_JSON_DECL
    void
    reserve(size_type count);

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    hasher
    hash_function() const;

    BOOST_JSON_DECL
    key_equal
    key_eq() const;

private:
    struct construct_base;

    BOOST_JSON_DECL
    element*
    allocate_impl(
        key_type key,
        construct_base const& place_new);

    BOOST_JSON_DECL
    element*
    allocate(std::pair<
        string_view, value const&> const& p);

    template<class Arg>
    element*
    allocate(
        key_type key,
        Arg&& arg);

    BOOST_JSON_DECL
    static
    size_type
    constrain_hash(
        std::size_t hash,
        size_type bucket_count) noexcept;

    BOOST_JSON_DECL
    std::pair<element*, std::size_t>
    find_impl(key_type key) const noexcept;

    BOOST_JSON_DECL
    void
    insert(
        const_iterator pos,
        std::size_t hash,
        element* e);

    template<class InputIt>
    void
    insert_range(
        const_iterator pos,
        InputIt first,
        InputIt last,
        size_type bucket_count);

    BOOST_JSON_DECL
    void
    remove(element* e);
};

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// headers for this file are at the bottom of value.hpp

#endif
