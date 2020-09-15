//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_VALUE_HPP
#define BOOST_JSON_VALUE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/array.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/object.hpp>
#include <boost/json/pilfer.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/value_ref.hpp>
#include <boost/json/detail/except.hpp>
#include <boost/json/detail/value.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iosfwd>
#include <new>
#include <type_traits>
#include <utility>

BOOST_JSON_NS_BEGIN

//----------------------------------------------------------

/** The type used to represent any JSON value

    This is a <em>Regular</em> type which works like
    a variant of the basic JSON data types: array,
    object, string, number, boolean, and null.

    @par Thread Safety

    Distinct instances may be accessed concurrently.
    Non-const member functions of a shared instance
    may not be called concurrently with any other
    member functions of that instance.
*/
class value
{
#ifndef BOOST_JSON_DOCS
    // XSL scripts have trouble with private anon unions
    using int64_k = detail::int64_k;
    using uint64_k = detail::uint64_k;
    using double_k = detail::double_k;
    using bool_k = detail::bool_k;
    using null_k = detail::null_k;

    union
    {
        storage_ptr sp_; // must come first
        object      obj_;
        array       arr_;
        string      str_;
        int64_k     i64_;
        uint64_k    u64_;
        double_k    dub_;
        bool_k      bln_;
        null_k      nul_;
    };
#endif

    struct undo;
    struct init_iter;

#ifndef BOOST_JSON_DOCS
    // VFALCO doc toolchain incorrectly treats this as public
    friend struct detail::value_access;
#endif

    // VFALCO Why are these in
    // detail/value.hpp instead of detail/value.ipp?
    inline value(detail::unchecked_object&& uo);
    inline value(detail::unchecked_array&& ua);
    inline value(char** key, std::size_t len, storage_ptr sp);
    inline char const* release_key(std::size_t& len) noexcept;

public:
    /** The type of _Allocator_ returned by @ref get_allocator

        This type is a @ref polymorphic_allocator.
    */
#ifdef BOOST_JSON_DOCS
    // VFALCO doc toolchain renders this incorrectly
    using allocator_type = __see_below__;
#else
    using allocator_type = polymorphic_allocator<value>;
#endif

    /** Destructor.

        The value and all of its contents are destroyed.
        Any dynamically allocated memory that was allocated
        internally is freed.

        @par Complexity
        Constant, or linear in size for array or object.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    ~value();

    /** Default constructor.

        The constructed value is null,
        using the default memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    value() noexcept
        : nul_()
    {
    }

    /** Constructor.

        The constructed value is null,
        using the specified @ref memory_resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    explicit
    value(storage_ptr sp) noexcept
        : nul_(detail::move(sp))
    {
    }

    /** Pilfer constructor.

        The value is constructed by acquiring ownership of
        the contents of `other` using pilfer semantics.

        @note

        After construction, the moved-from value may
        only be destroyed.
        
        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The value to pilfer.

        @see @ref pilfer
        
        Pilfering constructors are described in
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">Valueless Variants Considered Harmful</a>, by Peter Dimov.
    */
    BOOST_JSON_DECL
    value(pilfered<value> other) noexcept;

    /** Copy constructor.

        The value is constructed with a copy of the
        contents of `other`, using the same
        memory resource as `other`.

        @par Complexity
        Linear in the size of `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to copy.
    */
    value(value const& other)
        : value(other, other.storage())
    {
    }

    /** Copy constructor

        The value is constructed with a copy of the
        contents of `other`, using the
        specified memory resource.

        @par Complexity
        Linear in the size of `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to copy.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    BOOST_JSON_DECL
    value(
        value const& other,
        storage_ptr sp);

    /** Move constructor

        The value is constructed by acquiring ownership of
        the contents of `other` and shared ownership of
        `other`'s memory resource.
        
        @note

        After construction, the moved-from value becomes a
        null value with its current storage pointer.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The value to move.
    */
    BOOST_JSON_DECL
    value(value&& other) noexcept;

    /** Move constructor

        The value is constructed with the contents of
        `other` by move semantics, using the specified
        memory resource:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After construction, the moved-from value becomes
        a null value with its current storage pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed if
        `other.is_structured() == true`, which may throw.
        In this case, the moved-from value is not
        changed.
        
        @par Complexity
        Constant or linear in the size of `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to move.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    BOOST_JSON_DECL
    value(
        value&& other,
        storage_ptr sp);

    //------------------------------------------------------

    /** Move assignment.

        The contents of the value are replaced with the
        contents of `other` using move semantics:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After assignment, the moved-from value becomes
        a null with its current storage pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed if
        `other.is_structured() == true`, which may throw.
        In this case, the moved-from value is not
        changed.

        @par Complexity
        Constant, or linear in
        `this->size()` plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to assign from.
    */
    BOOST_JSON_DECL
    value&
    operator=(value&& other);

    /** Copy assignment operator

        The contents of the value are replaced with an
        element-wise copy of the contents of `other`.

        @par Complexity
        Linear in the size of `*this` plus `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to copy.
    */
    BOOST_JSON_DECL
    value& operator=(value const& other);

    //------------------------------------------------------
    //
    // Conversion
    //
    //------------------------------------------------------

    /** Construct an @ref object.

        The value is constructed from `other`, using the
        same memory resource. To transfer ownership, use `std::move`:

        @par Example
        @code
        object obj( {{"a",1}, {"b",2}, {"c"},3}} );

        // transfer ownership
        value jv( std::move(obj) );

        assert( obj.empty() );
        assert( *obj.storage() == *jv.storage() );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The object to construct with.
    */
    value(object other) noexcept
        : obj_(detail::move(other))
    {
    }

    /** Construct an @ref object.

        The value is copy constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        object const& other,
        storage_ptr sp)
        : obj_(
            other,
            detail::move(sp))
    {
    }

    /** Construct an @ref object.

        The value is move constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Constant or linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        object&& other,
        storage_ptr sp)
        : obj_(
            detail::move(other),
            detail::move(sp))
    {
    }

    /** Construct an @ref object.

        This is the fastest way to construct
        an empty object, using the specified
        memory resource. The variable @ref object_kind
        may be passed as the first parameter
        to select this overload:

        @par Example
        @code
        // Construct an empty object

        value jv( object_kind );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.

        @see @ref object_kind
    */
    value(
        object_kind_t,
        storage_ptr sp = {}) noexcept
        : obj_(detail::move(sp))
    {
    }

    /** Construct an @ref array.

        The value is constructed from `other`, using the
        same memory resource. To transfer ownership, use `std::move`:

        @par Example
        @code
        array arr( {1, 2, 3, 4, 5} );

        // transfer ownership
        value jv( std::move(arr) );

        assert( arr.empty() );
        assert( *arr.storage() == *jv.storage() );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The array to construct with.
    */
    value(array other) noexcept
        : arr_(detail::move(other))
    {
    }

    /** Construct an @ref array.

        The value is copy constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The array to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        array const& other,
        storage_ptr sp)
        : arr_(
            other,
            detail::move(sp))
    {
    }

    /** Construct an @ref array.

        The value is move-constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Constant or linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The array to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        array&& other,
        storage_ptr sp)
        : arr_(
            std::move(other),
            detail::move(sp))
    {
    }

    /** Construct an @ref array.

        This is the fastest way to construct
        an empty array, using the specified
        memory resource. The variable @ref array_kind
        may be passed as the first parameter
        to select this overload:

        @par Example
        @code
        // Construct an empty array

        value jv( array_kind );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.

        @see @ref array_kind
    */
    value(
        array_kind_t,
        storage_ptr sp = {}) noexcept
        : arr_(detail::move(sp))
    {
    }

    /** Construct a @ref string.

        The value is constructed from `other`, using the
        same memory resource. To transfer ownership, use `std::move`:

        @par Example
        @code
        string str = "The Boost C++ Library Collection";

        // transfer ownership
        value jv( std::move(str) );

        assert( str.empty() );
        assert( *str.storage() == *jv.storage() );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The string to construct with.
    */
    value(
        string other) noexcept
        : str_(detail::move(other))
    {
    }

    /** Construct a @ref string.

        The value is copy constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The string to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        string const& other,
        storage_ptr sp)
        : str_(
            other,
            detail::move(sp))
    {
    }

    /** Construct a @ref string.

        The value is move constructed from `other`,
        using the specified memory resource.

        @par Complexity
        Constant or linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The string to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        string&& other,
        storage_ptr sp)
        : str_(
            detail::move(other),
            detail::move(sp))
    {
    }

    /** Construct a @ref string.

        The string is constructed with a copy of the
        string view `s`, using the specified memory resource.

        @par Complexity
        Linear in `s.size()`.

        @par Exception Safety  
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The string view to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        string_view s,
        storage_ptr sp = {})
        : str_(s, detail::move(sp))
    {
    }

    /** Construct a @ref string.

        The string is constructed with a copy of the
        null-terminated string `s`, using the specified
        memory resource.

        @par Complexity
        Linear in `std::strlen(s)`.

        @par Exception Safety    
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The null-terminated string to construct
        with.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        char const* s,
        storage_ptr sp = {})
        : str_(s, detail::move(sp))
    {
    }

    /** Construct a @ref string.

        This is the fastest way to construct
        an empty string, using the specified
        memory resource. The variable @ref string_kind
        may be passed as the first parameter
        to select this overload:

        @par Example
        @code
        // Construct an empty string

        value jv( string_kind );
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.

        @see @ref string_kind
    */
    value(
        string_kind_t,
        storage_ptr sp = {}) noexcept
        : str_(detail::move(sp))
    {
    }

    /** Construct a `std::int64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param i The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        short i,
        storage_ptr sp = {}) noexcept
        : i64_(i, detail::move(sp))
    {
    }

    /** Construct a `std::int64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param i The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        int i,
        storage_ptr sp = {}) noexcept
        : i64_(i, detail::move(sp))
    {
    }

    /** Construct a `std::int64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param i The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        long i,
        storage_ptr sp = {}) noexcept
        : i64_(i, detail::move(sp))
    {
    }

    /** Construct a `std::int64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param i The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        long long i,
        storage_ptr sp = {}) noexcept
        : i64_(i, detail::move(sp))
    {
    }

    /** Construct a `std::uint64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param u The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        unsigned short u,
        storage_ptr sp = {}) noexcept
        : u64_(u, detail::move(sp))
    {
    }

    /** Construct a `std::uint64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param u The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        unsigned int u,
        storage_ptr sp = {}) noexcept
        : u64_(u, detail::move(sp))
    {
    }

    /** Construct a `std::uint64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param u The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        unsigned long u,
        storage_ptr sp = {}) noexcept
        : u64_(u, detail::move(sp))
    {
    }

    /** Construct a `std::uint64_t`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param u The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        unsigned long long u,
        storage_ptr sp = {}) noexcept
        : u64_(u, detail::move(sp))
    {
    }

    /** Construct a `double`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param d The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        double d,
        storage_ptr sp = {}) noexcept
        : dub_(d, detail::move(sp))
    {
    }

    /** Construct a `double`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param d The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        long double d,
        storage_ptr sp = {}) noexcept
        : dub_(static_cast<double>(d),
            detail::move(sp))
    {
    }

    /** Construct a bool.

        This constructs a `bool` value using
        the specified memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param b The initial value.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
#ifdef BOOST_JSON_DOCS
    value(
        bool b,
        storage_ptr sp = {}) noexcept;
#else
    template<class Bool
        ,class = typename std::enable_if<
            std::is_same<Bool, bool>::value>::type
    >
    value(
        Bool b,
        storage_ptr sp = {}) noexcept
        : bln_(b, detail::move(sp))
    {
    }
#endif

    /** Construct a null.

        A null value is a monostate.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    value(
        std::nullptr_t,
        storage_ptr sp = {}) noexcept
        : nul_(detail::move(sp))
    {
    }

    /** Construct an object or array

        If the initializer list consists of key/value
        pairs, an @ref object is created. Otherwise
        an @ref array is created. The contents of the 
        initializer list are copied to the newly constructed
        value using the specified memory resource.

        @par Complexity
        Linear in `init.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to construct from. 

        @param sp A pointer to the @ref memory_resource
        to use. The container will acquire shared
        ownership of the memory resource.
    */
    BOOST_JSON_DECL
    value(
        std::initializer_list<value_ref> init,
        storage_ptr sp = {});

    /** Assign an object or array

        If the initializer list consists of key/value
        pairs, the resulting @ref object is assigned.
        Otherwise an @ref array is assigned. The contents
        of the initializer list are moved to `*this`
        using the existing memory resource.

        @par Complexity
        Linear in `init.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to assign from.
    */
    value&
    operator=(
        std::initializer_list<value_ref> init)
    {
        return *this = value(init, storage());
    }

    /** Assignment.

        @par Constraints
        @code
        std::is_constructible< value, T, storage_ptr >::value &&
            ! std::is_same< std::remove_cvref< T >, value >::value
        @endcode

        @par Complexity
        Constant or linear in the size of
        `*this` plus `t`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    template<class T
#ifndef BOOST_JSON_DOCS
        ,class = typename std::enable_if<
            std::is_constructible<
                value, T, storage_ptr>::value &&
            ! std::is_same<detail::remove_cvref<
                T>, value>::value
        >::type
#endif
    >
    value&
    operator=(T&& t);

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Return a reference to an @ref object, changing the kind and replacing the contents.

        The contents are replaced with an empty @ref object
        using the current @ref memory_resource. All
        previously obtained iterators and references
        obtained beforehand are invalidated.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    object&
    emplace_object() noexcept;

    /** Return a reference to an @ref array, changing the kind and replacing the contents.

        The value is replaced with an empty @ref array
        using the current memory resource, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    array&
    emplace_array() noexcept;

    /** Return a reference to a @ref string, changing the kind and replacing the contents.

        The value is replaced with an empty @ref string
        using the current memory resource, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    string&
    emplace_string() noexcept;

    /** Return a reference to a `std::int64_t`, changing the kind and replacing the contents.

        The value is replaced with a `std::int64_t`
        initialized to zero, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    std::int64_t&
    emplace_int64() noexcept;

    /** Return a reference to a `std::uint64_t`, changing the kind and replacing the contents.

        The value is replaced with a `std::uint64_t`
        initialized to zero, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    std::uint64_t&
    emplace_uint64() noexcept;

    /** Return a reference to a `double`, changing the kind and replacing the contents.

        The value is replaced with a `double`
        initialized to zero, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    double&
    emplace_double() noexcept;

    /** Return a reference to a `bool`, changing the kind and replacing the contents.

        The value is replaced with a `bool`
        initialized to `false`, destroying the
        previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    bool&
    emplace_bool() noexcept;

    /** Change the kind to null, discarding the previous contents.

        The value is replaced with a null,
        destroying the previous contents.

        @par Complexity
        Linear in the size of `*this`.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    emplace_null() noexcept;

    /** Swap the contents.

        Exchanges the contents of this value with another
        value. Ownership of the respective @ref memory_resource
        objects is not transferred.

        @li If `*other.storage() == *this->storage()`,
        ownership of the underlying memory is swapped in
        constant time, with no possibility of exceptions.
        All iterators and references remain valid.

        @li If `*other.storage() != *this->storage()`,
        the contents are logically swapped by making copies,
        which can throw. In this case all iterators and
        references are invalidated.

        @par Preconditions
        @code
        &other != this
        @endcode
        
        @par Complexity
        Constant or linear in the sum of the sizes of
        the values.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to swap with.
    */
    BOOST_JSON_DECL
    void
    swap(value& other);

    //------------------------------------------------------
    //
    // Observers
    //
    //------------------------------------------------------

    /** Returns the kind of this JSON value.

        This function returns the discriminating
        enumeration constant of type @ref json::kind
        corresponding to the underlying representation
        stored in the container.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    json::kind
    kind() const noexcept
    {
        return static_cast<json::kind>(
            static_cast<unsigned char>(
                nul_.k) & 0x3f);
    }

    /** Return an @ref array pointer if this is an array, else return `nullptr`

        If `this->kind() == kind::array`, returns a pointer
        to the underlying array. Otherwise, returns `nullptr`.

        @par Example
        The return value can be used in a logical context:
        @code
        if( jv.is_array() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_array() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    array const*
    is_array() const noexcept
    {
        if(kind() == json::kind::array)
            return &arr_;
        return nullptr;
    }

    /** Return an @ref array pointer if this is an array, else return `nullptr`

        If `this->kind() == kind::array`, returns a pointer
        to the underlying array. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_array() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_array() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    array*
    is_array() noexcept
    {
        if(kind() == json::kind::array)
            return &arr_;
        return nullptr;
    }

    /** Return an @ref object pointer if this is an object, else return `nullptr`

        If `this->kind() == kind::object`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_object() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_object() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    object const*
    is_object() const noexcept
    {
        if(kind() == json::kind::object)
            return &obj_;
        return nullptr;
    }

    /** Return an @ref object pointer if this is an object, else return `nullptr`

        If `this->kind() == kind::object`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_object() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_object() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    object*
    is_object() noexcept
    {
        if(kind() == json::kind::object)
            return &obj_;
        return nullptr;
    }

    /** Return a @ref string pointer if this is a string, else return `nullptr`

        If `this->kind() == kind::string`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_string() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_string() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    string const*
    is_string() const noexcept
    {
        if(kind() == json::kind::string)
            return &str_;
        return nullptr;
    }

    /** Return a @ref string pointer if this is a string, else return `nullptr`

        If `this->kind() == kind::string`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_string() )
            return true;
        @endcode

        Or the return value can be used to access the array
        later:
        @code
        if( auto p = jv.is_string() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    string*
    is_string() noexcept
    {
        if(kind() == json::kind::string)
            return &str_;
        return nullptr;
    }

    /** Return an `int64_t` pointer if this is a signed integer, else return `nullptr`

        If `this->kind() == kind::int64`, returns a pointer
        to the underlying integer. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_int64() )
            return true;
        @endcode

        Or the return value can be used to access the integer
        later:
        @code
        if( auto p = jv.is_int64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::int64_t const*
    is_int64() const noexcept
    {
        if(kind() == json::kind::int64)
            return &i64_.i;
        return nullptr;
    }

    /** Return an `int64_t` pointer if this is a signed integer, else return `nullptr`

        If `this->kind() == kind::int64`, returns a pointer
        to the underlying integer. Otherwise, returns `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_int64() )
            return true;
        @endcode

        Or the return value can be used to access the integer
        later:
        @code
        if( auto p = jv.is_int64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::int64_t*
    is_int64() noexcept
    {
        if(kind() == json::kind::int64)
            return &i64_.i;
        return nullptr;
    }

    /** Return a `uint64_t` pointer if this is a signed integer, else return `nullptr`

        If `this->kind() == kind::uint64`, returns a pointer
        to the underlying unsigned integer. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_uint64() )
            return true;
        @endcode

        Or the return value can be used to access the unsigned integer
        later:
        @code
        if( auto p = jv.is_uint64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::uint64_t const*
    is_uint64() const noexcept
    {
        if(kind() == json::kind::uint64)
            return &u64_.u;
        return nullptr;
    }

    /** Return a `uint64_t` pointer if this is an unsigned integer, else return `nullptr`

        If `this->kind() == kind::uint64`, returns a pointer
        to the underlying unsigned integer. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_uint64() )
            return true;
        @endcode

        Or the return value can be used to access the unsigned integer
        later:
        @code
        if( auto p = jv.is_uint64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::uint64_t*
    is_uint64() noexcept
    {
        if(kind() == json::kind::uint64)
            return &u64_.u;
        return nullptr;
    }

    /** Return a `double` pointer if this is a double, else return `nullptr`

        If `this->kind() == kind::double_`, returns a pointer
        to the underlying double. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_double() )
            return true;
        @endcode

        Or the return value can be used to access the double
        later:
        @code
        if( auto p = jv.is_double() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    double const*
    is_double() const noexcept
    {
        if(kind() == json::kind::double_)
            return &dub_.d;
        return nullptr;
    }

    /** Return a `double` pointer if this is a double, else return `nullptr`

        If `this->kind() == kind::double_`, returns a pointer
        to the underlying double. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_double() )
            return true;
        @endcode

        Or the return value can be used to access the double
        later:
        @code
        if( auto p = jv.is_double() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    double*
    is_double() noexcept
    {
        if(kind() == json::kind::double_)
            return &dub_.d;
        return nullptr;
    }

    /** Return a `bool` pointer if this is a boolean, else return `nullptr`

        If `this->kind() == kind::bool_`, returns a pointer
        to the underlying boolean. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_bool() )
            return true;
        @endcode

        Or the return value can be used to access the boolean
        later:
        @code
        if( auto p = jv.is_bool() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool const*
    is_bool() const noexcept
    {
        if(kind() == json::kind::bool_)
            return &bln_.b;
        return nullptr;
    }

    /** Return a `bool` pointer if this is a boolean, else return `nullptr`

        If `this->kind() == kind::bool_`, returns a pointer
        to the underlying boolean. Otherwise, returns
        `nullptr`.

        @par Example

        The return value can be used in a logical context:
        @code
        if( jv.is_bool() )
            return true;
        @endcode

        Or the return value can be used to access the boolean
        later:
        @code
        if( auto p = jv.is_bool() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool*
    is_bool() noexcept
    {
        if(kind() == json::kind::bool_)
            return &bln_.b;
        return nullptr;
    }

    /** Returns true if this is a null.

        This function returns `true` when
        @ref kind() equals `kind::null`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_null() const noexcept
    {
        return kind() == json::kind::null;
    }

    /** Returns true if this is an array or object.

        This function returns `true` if
        @ref kind() is either `kind::object` or
        `kind::array`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_structured() const noexcept
    {
        // VFALCO Could use bit 0x20 for this
        return
           kind() == json::kind::object ||
           kind() == json::kind::array;
    }

    /** Returns true if this is not an array or object.

        This function returns `true` if
        @ref kind() is neither `kind::object` nor
        `kind::array`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_primitive() const noexcept
    {
        // VFALCO Could use bit 0x20 for this
        return
           nul_.k != json::kind::object &&
           nul_.k != json::kind::array;
    }

    /** Returns true if this is a number.

        This function returns `true` when
        @ref kind() is one of the following values:
        `kind::int64`, `kind::uint64`, or
        `kind::double_`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_number() const noexcept
    {
        // VFALCO Could use bit 0x40 for this
        return
            kind() == json::kind::int64 ||
            kind() == json::kind::uint64 ||
            kind() == json::kind::double_;
    }

    //------------------------------------------------------
    //
    // Accessors
    //
    //------------------------------------------------------

    /** Return the memory resource associated with the value.

        This returns a pointer to the memory resource
        that was used to construct the value.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    storage_ptr const&
    storage() const noexcept
    {
        return sp_;
    }

    /** Return the associated @ref memory_resource

        This function returns an instance of
        @ref polymorphic_allocator constructed from the
        associated @ref memory_resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    allocator_type
    get_allocator() const noexcept;

    //------------------------------------------------------

    /** Return a reference to the underlying `object`, or throw an exception.

        If @ref is_object() is `true`, returns
        a reference to the underlying @ref object,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_object()`
    */
    object&
    as_object()
    {
        if(! is_object())
            detail::throw_invalid_argument(
                "not an object",
                BOOST_CURRENT_LOCATION);
        return obj_;
    }

    /** Return a reference to the underlying `object`, or throw an exception.

        If @ref is_object() is `true`, returns
        a reference to the underlying @ref object,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_object()`
    */
    object const&
    as_object() const
    {
        if(! is_object())
            detail::throw_invalid_argument(
                "not an object",
                BOOST_CURRENT_LOCATION);
        return obj_;
    }

    /** Return a reference to the underlying @ref array, or throw an exception.

        If @ref is_array() is `true`, returns
        a reference to the underlying @ref array,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_array()`
    */
    array&
    as_array()
    {
        if(! is_array())
            detail::throw_invalid_argument(
                "array required",
                BOOST_CURRENT_LOCATION);
        return arr_;
    }

    /** Return a reference to the underlying `array`, or throw an exception.

        If @ref is_array() is `true`, returns
        a reference to the underlying @ref array,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_array()`
    */
    array const&
    as_array() const
    {
        if(! is_array())
            detail::throw_invalid_argument(
                "array required",
                BOOST_CURRENT_LOCATION);
        return arr_;
    }

    /** Return a reference to the underlying `string`, or throw an exception.

        If @ref is_string() is `true`, returns
        a reference to the underlying @ref string,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_string()`
    */
    string&
    as_string()
    {
        if(! is_string())
            detail::throw_invalid_argument(
                "not a string",
                BOOST_CURRENT_LOCATION);
        return str_;
    }

    /** Return a reference to the underlying `string`, or throw an exception.

        If @ref is_string() is `true`, returns
        a reference to the underlying @ref string,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_string()`
    */
    string const&
    as_string() const
    {
        if(! is_string())
            detail::throw_invalid_argument(
                "not a string",
                BOOST_CURRENT_LOCATION);
        return str_;
    }

    /** Return a reference to the underlying `std::int64_t`, or throw an exception.

        If @ref is_int64() is `true`, returns
        a reference to the underlying `std::int64_t`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_int64()`
    */
    std::int64_t&
    as_int64()
    {
        if(! is_int64())
            detail::throw_invalid_argument(
                "not an int64",
                BOOST_CURRENT_LOCATION);
        return i64_.i;
    }

    /** Return the underlying `std::int64_t`, or throw an exception.

        If @ref is_int64() is `true`, returns
        the underlying `std::int64_t`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_int64()`
    */
    std::int64_t
    as_int64() const
    {
        if(! is_int64())
            detail::throw_invalid_argument(
                "not an int64",
                BOOST_CURRENT_LOCATION);
        return i64_.i;
    }

    /** Return a reference to the underlying `std::uint64_t`, or throw an exception.

        If @ref is_uint64() is `true`, returns
        a reference to the underlying `std::uint64_t`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_uint64()`
    */
    std::uint64_t&
    as_uint64()
    {
        if(! is_uint64())
            detail::throw_invalid_argument(
                "not a uint64",
                BOOST_CURRENT_LOCATION);
        return u64_.u;
    }

    /** Return the underlying `std::uint64_t`, or throw an exception.

        If @ref is_int64() is `true`, returns
        the underlying `std::uint64_t`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::length_error `! this->is_uint64()`
    */
    std::uint64_t
    as_uint64() const
    {
        if(! is_uint64())
            detail::throw_invalid_argument(
                "not a uint64",
                BOOST_CURRENT_LOCATION);
        return u64_.u;
    }

    /** Return a reference to the underlying `double`, or throw an exception.

        If @ref is_double() is `true`, returns
        a reference to the underlying `double`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_double()`
    */
    double&
    as_double()
    {
        if(! is_double())
            detail::throw_invalid_argument(
                "not a double",
                BOOST_CURRENT_LOCATION);
        return dub_.d;
    }

    /** Return the underlying `double`, or throw an exception.

        If @ref is_int64() is `true`, returns
        the underlying `double`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_double()`
    */
    double
    as_double() const
    {
        if(! is_double())
            detail::throw_invalid_argument(
                "not a double",
                BOOST_CURRENT_LOCATION);
        return dub_.d;
    }

    /** Return a reference to the underlying `bool`, or throw an exception.

        If @ref is_bool() is `true`, returns
        a reference to the underlying `bool`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_bool()`
    */
    bool&
    as_bool()
    {
        if(! is_bool())
            detail::throw_invalid_argument(
                "bool required",
                BOOST_CURRENT_LOCATION);
        return bln_.b;
    }

    /** Return the underlying `bool`, or throw an exception.

        If @ref is_bool() is `true`, returns
        the underlying `bool`,
        otherwise throws an exception.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @throw std::invalid_argument `! this->is_bool()`
    */
    bool
    as_bool() const
    {
        if(! is_bool())
            detail::throw_invalid_argument(
                "bool required",
                BOOST_CURRENT_LOCATION);
        return bln_.b;
    }

    //------------------------------------------------------

    /** Return a reference to the underlying `object`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_object()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    object&
    get_object() noexcept
    {
        BOOST_ASSERT(is_object());
        return obj_;
    }

    /** Return a reference to the underlying `object`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_object()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    object const&
    get_object() const noexcept
    {
        BOOST_ASSERT(is_object());
        return obj_;
    }

    /** Return a reference to the underlying `array`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_array()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    array&
    get_array() noexcept
    {
        BOOST_ASSERT(is_array());
        return arr_;
    }

    /** Return a reference to the underlying `array`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_array()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    array const&
    get_array() const noexcept
    {
        BOOST_ASSERT(is_array());
        return arr_;
    }

    /** Return a reference to the underlying `string`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_string()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    string&
    get_string() noexcept
    {
        BOOST_ASSERT(is_string());
        return str_;
    }

    /** Return a reference to the underlying `string`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_string()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    string const&
    get_string() const noexcept
    {
        BOOST_ASSERT(is_string());
        return str_;
    }

    /** Return a reference to the underlying `std::int64_t`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_int64()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::int64_t&
    get_int64() noexcept
    {
        BOOST_ASSERT(is_int64());
        return i64_.i;
    }

    /** Return the underlying `std::int64_t`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_int64()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::int64_t
    get_int64() const noexcept
    {
        BOOST_ASSERT(is_int64());
        return i64_.i;
    }

    /** Return a reference to the underlying `std::uint64_t`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_uint64()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::uint64_t&
    get_uint64() noexcept
    {
        BOOST_ASSERT(is_uint64());
        return u64_.u;
    }

    /** Return the underlying `std::uint64_t`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_uint64()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::uint64_t
    get_uint64() const noexcept
    {
        BOOST_ASSERT(is_uint64());
        return u64_.u;
    }

    /** Return a reference to the underlying `double`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_double()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    double&
    get_double() noexcept
    {
        BOOST_ASSERT(is_double());
        return dub_.d;
    }

    /** Return the underlying `double`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_double()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    double
    get_double() const noexcept
    {
        BOOST_ASSERT(is_double());
        return dub_.d;
    }

    /** Return a reference to the underlying `bool`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_bool()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool&
    get_bool() noexcept
    {
        BOOST_ASSERT(is_bool());
        return bln_.b;
    }

    /** Return the underlying `bool`, without checking.

        This is the fastest way to access the underlying
        representation when the kind is known in advance.

        @par Preconditions
        
        @code
        this->is_bool()
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    get_bool() const noexcept
    {
        BOOST_ASSERT(is_bool());
        return bln_.b;
    }

    //------------------------------------------------------

    /** Access an element, with bounds checking.

        This function is used to access elements of
        the underlying object, or throw an exception
        if the value is not an object.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @param key The key of the element to find.

        @return `this->as_object().at( key )`.
    */
    value const&
    at(string_view key) const
    {
        return as_object().at(key);
    }

    /** Access an element, with bounds checking.

        This function is used to access elements of
        the underlying array, or throw an exception
        if the value is not an array.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.

        @param pos A zero-based array index.

        @return `this->as_array().at( pos )`.
    */
    value const&
    at(std::size_t pos) const
    {
        return as_array().at(pos);
    }

    /** Return `true` if two values are equal.

        Two values are equal when they are the
        same kind and their referenced values
        are equal, or when they are both integral
        types and their integral representations
        are equal.

        @par Complexity
        Constant or linear in the size of
        the array, object, or string.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator==(
        value const& lhs,
        value const& rhs) noexcept
    {
        return lhs.equal(rhs);
    }

    /** Return `true` if two values are not equal.

        Two values are equal when they are the
        same kind and their referenced values
        are equal, or when they are both integral
        types and their integral representations
        are equal.

        @par Complexity
        Constant or linear in the size of
        the array, object, or string.

        @par Exception Safety
        No-throw guarantee.
    */
    friend
    bool
    operator!=(
        value const& lhs,
        value const& rhs) noexcept
    {
        return ! (lhs == rhs);
    }

private:
    static
    inline
    void
    relocate(
        value* dest,
        value const& src) noexcept;

    BOOST_JSON_DECL
    storage_ptr
    destroy() noexcept;

    BOOST_JSON_DECL
    bool
    equal(value const& other) const noexcept;
};

// Make sure things are as big as we think they should be
#if BOOST_JSON_ARCH == 64
BOOST_STATIC_ASSERT(sizeof(value) == 24);
#elif BOOST_JSON_ARCH == 32
BOOST_STATIC_ASSERT(sizeof(value) == 16);
#else 
# error Unknown architecture
#endif

//----------------------------------------------------------

/** Exchange the given values.

    Exchanges the contents of value `lhs` with
    another value `rhs`. Ownership of the respective
    @ref memory_resource objects is not transferred.

    @li If `*lhs.storage() == *rhs.storage()`,
    ownership of the underlying memory is swapped in
    constant time, with no possibility of exceptions.
    All iterators and references remain valid.

    @li If `*lhs.storage() != *rhs.storage`,
    the contents are logically swapped by a copy,
    which can throw. In this case all iterators and
    references are invalidated.

    @par Preconditions

    `&lhs != &rhs`
        
    @par Complexity
    Constant or linear in the sum of the sizes of
    the values.

    @par Exception Safety
    Strong guarantee.
    Calls to `memory_resource::allocate` may throw.

    @par Effects
    @code
    lhs.swap( rhs );
    @endcode

    @param lhs The value to exchange.

    @param rhs The value to exchange.

    @see @ref value::swap
*/
inline
void
swap(value& lhs, value& rhs)
{
    lhs.swap(rhs);
}

//----------------------------------------------------------

/** A key/value pair.

    This is the type of element used by the @ref object
    container.
*/
class key_value_pair
{
    friend struct detail::value_access;

    inline
    key_value_pair(
        pilfered<json::value> k,
        pilfered<json::value> v) noexcept;

public:
    /// Copy assignment (deleted).
    key_value_pair&
    operator=(key_value_pair const&) = delete;

    /** Destructor.

        The value is destroyed and all internally
        allocated memory is freed.
    */
    BOOST_JSON_DECL
    ~key_value_pair();

    /** Copy constructor.

        This constructs a key/value pair with a
        copy of another key/value pair, using
        the same memory resource as `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The key/value pair to copy.
    */
    BOOST_JSON_DECL
    key_value_pair(key_value_pair const& other);

    /** Copy constructor.

        This constructs a key/value pair with a
        copy of another key/value pair, using
        the specified memory resource.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The key/value pair to copy.

        @param sp A pointer to the @ref memory_resource
        to use. The element will acquire shared
        ownership of the memory resource.
    */
    BOOST_JSON_DECL
    key_value_pair(
        key_value_pair const& other,
        storage_ptr sp);

    /** Constructor.

        This constructs a key/value pair.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param key The key string to use.

        @param args Optional arguments forwarded to
        the @ref value constructor.
    */
    template<class... Args>
    key_value_pair(
        string_view key,
        Args&&... args);

    /** Constructor.

        This constructs a key/value pair. A
        copy of the specified value is made,
        using the specified memory resource.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param p A `std::pair` with the key
            string and @ref value to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The element will acquire shared
        ownership of the memory resource.
    */
    explicit
    key_value_pair(
        std::pair<
            string_view,
            json::value> const& p,
        storage_ptr sp = {})
        : key_value_pair(
            p.first,
            p.second,
            detail::move(sp))
    {
    }

    /** Constructor.

        This constructs a key/value pair.
        Ownership of the specified value is
        transferred by move construction.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param p A `std::pair` with the key
            string and @ref value to construct with.

        @param sp A pointer to the @ref memory_resource
        to use. The element will acquire shared
        ownership of the memory resource.
    */
    explicit
    key_value_pair(
        std::pair<
            string_view,
            json::value>&& p,
        storage_ptr sp = {})
        : key_value_pair(
            p.first,
            detail::move(p).second,
            detail::move(sp))
    {
    }

    /** Return the key of this element.

        After construction, the key may
        not be modified.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    string_view const
    key() const noexcept
    {
        return { key_, len_ };
    }

    /** Return the key of this element as a null-terminated string.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    char const*
    key_c_str() const noexcept
    {
        return key_;
    }

    /** Return the value of this element.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    json::value const&
    value() const noexcept
    {
        return value_;
    }

    /** Return the value of this element.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    json::value&
    value() noexcept
    {
        return value_;
    }

private:
    static
    inline
    std::uint32_t
    key_size(std::size_t n);

    json::value value_;
    char const* key_;
    std::uint32_t len_;
    std::uint32_t next_;
};

//----------------------------------------------------------

#ifdef BOOST_JSON_DOCS

/** Tuple-like element access.
    
    This overload permits the key and value
    of a `key_value_pair` to be accessed
    by index. For example:

    @code 

    key_value_pair kvp("num", 42);
    
    string_view key = get<0>(kvp);
    value& jv = get<1>(kvp);

    @endcode

    @par Structured Bindings

    When using C++17 or greater, objects of type
    @ref key_value_pair may be used to initialize
    structured bindings:

    @code

    key_value_pair kvp("num", 42);

    auto& [key, value] = kvp;

    @endcode

    Depending on the value of `I`, the return type will be:

    @li `string_view const` if `I == 0`, or

    @li `value&`, `value const&`, or `value&&` if `I == 1`.
    
    Any other value for `I` is ill-formed.

    @tparam I The element index to access.

    @par Constraints

    `std::is_same_v< std::remove_cvref_t<T>, key_value_pair >`

    @return `kvp.key()` if `I == 0`, or `kvp.value()` 
    if `I == 1`.

    @param kvp The @ref key_value_pair object
    to access.
*/
template<
    std::size_t I,
    class T>
__see_below__
get(T&& kvp) noexcept;

#else

template<std::size_t I>
auto
get(key_value_pair const&) noexcept ->
    typename std::conditional<I == 0,
        string_view const,
        value const&>::type
{
    static_assert(I == 0, 
        "key_value_pair index out of range");
}

template<std::size_t I>
auto
get(key_value_pair&) noexcept ->
    typename std::conditional<I == 0,
        string_view const,
        value&>::type
{
    static_assert(I == 0, 
        "key_value_pair index out of range");
}

template<std::size_t I>
auto
get(key_value_pair&&) noexcept ->
    typename std::conditional<I == 0,
        string_view const,
        value&&>::type
{
    static_assert(I == 0, 
        "key_value_pair index out of range");
}

/** Extracts a key_value_pair's key using tuple-like interface
*/
template<>
inline
string_view const
get<0>(key_value_pair const& kvp) noexcept 
{
    return kvp.key();
}

/** Extracts a key_value_pair's key using tuple-like interface
*/
template<>
inline
string_view const
get<0>(key_value_pair& kvp) noexcept
{
    return kvp.key();
}

/** Extracts a key_value_pair's key using tuple-like interface
*/
template<>
inline
string_view const
get<0>(key_value_pair&& kvp) noexcept
{
    return kvp.key();
}

/** Extracts a key_value_pair's value using tuple-like interface
*/
template<>
inline
value const&
get<1>(key_value_pair const& kvp) noexcept
{
    return kvp.value();
}

/** Extracts a key_value_pair's value using tuple-like interface
*/
template<>
inline
value&
get<1>(key_value_pair& kvp) noexcept
{
    return kvp.value();
}

/** Extracts a key_value_pair's value using tuple-like interface
*/
template<>
inline
value&&
get<1>(key_value_pair&& kvp) noexcept
{
    return std::move(kvp.value());
}

#endif

BOOST_JSON_NS_END

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

#ifndef BOOST_JSON_DOCS

namespace std {

/** Tuple-like size access for key_value_pair
*/
template<>
struct tuple_size<::boost::json::key_value_pair>
    : std::integral_constant<std::size_t, 2> 
{
};

/** Tuple-like access for the key type of key_value_pair
*/
template<>
struct tuple_element<0, ::boost::json::key_value_pair> 
{
    using type = ::boost::json::string_view const;
};

/** Tuple-like access for the value type of key_value_pair
*/
template<>
struct tuple_element<1, ::boost::json::key_value_pair> 
{
    using type = ::boost::json::value&;
};

/** Tuple-like access for the value type of key_value_pair
*/
template<>
struct tuple_element<1, ::boost::json::key_value_pair const> 
{
    using type = ::boost::json::value const&;
};

} // std

#endif

#ifdef __clang__
# pragma clang diagnostic pop
#endif

// These are here because value, array,
// and object form cyclic references.

#include <boost/json/detail/impl/array_impl.hpp>
#include <boost/json/detail/impl/object_impl.hpp>
#include <boost/json/impl/array.hpp>
#include <boost/json/impl/object.hpp>

// These must come after array and object
#include <boost/json/impl/value.hpp>
#include <boost/json/impl/value_ref.hpp>

#endif
