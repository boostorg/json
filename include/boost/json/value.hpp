//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_HPP
#define BOOST_JSON_VALUE_HPP

#include <boost/json/config.hpp>
#include <boost/json/array.hpp>
#include <boost/json/error.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/object.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string.hpp>
#include <boost/json/value_ref.hpp>
#include <boost/json/detail/scalar_impl.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iosfwd>
#include <new>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** The type used to represent any JSON value

    @par Thread Safety:

    Distinct instances may be accessed concurrently.
    Non-const member functions of a shared instance
    may not be called concurrently with any other
    member functions of that instance.
*/
class value
{
#ifndef GENERATING_DOCUMENTATION
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

    static
    constexpr
    json::kind
    short_string_ =
        static_cast<json::kind>(
            ((unsigned char)
            json::kind::string) | 0x80);

    struct undo;
    struct init_iter;

public:
    /** Destructor.

        The value and all of its contents are destroyed.
        Any dynamically allocated internal storage
        is freed.

        @par Complexity

        Constant, or linear in size for array or object.
    */
    BOOST_JSON_DECL
    ~value();

#ifndef GENERATING_DOCUMENTATION
    // private
    inline value(detail::unchecked_object&& uo);
    inline value(detail::unchecked_array&& ua);
#endif

    /** Default constructor.

        The constructed value is null,
        using the default storage.

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
        using the specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        contents of `other`, using the storage of `other`.

        @par Complexity

        Linear in the size of `other`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The value to copy.
    */
    value(value const& other)
        : value(other, other.storage())
    {
    }

    /** Copy constructor

        The value is constructed with a copy of the
        contents of `other`, using the specified storage.

        @par Complexity

        Linear in the size of `other`.

        @param other The value to copy.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    value(
        value const& other,
        storage_ptr sp);

    /** Move constructor

        The value is constructed by acquiring ownership of
        the contents of `other` and shared ownership of
        the storage of `other`.
        
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
        storage:

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
        Calls to @ref storage::allocate may throw.

        @param other The value to move.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        Calls to @ref storage::allocate may throw.

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
        Calls to @ref storage::allocate may throw.

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
        same storage. To transfer ownership, use `std::move`:

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
        using the specified storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        using the specified storage.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    value(
        object&& other,
        storage_ptr sp)
        : obj_(
            other,
            detail::move(sp))
    {
    }

    /** Construct an @ref object.

        This is the fastest way to construct
        an empty object, using the specified
        storage. The variable @ref object_kind
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

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
        same storage. To transfer ownership, use `std::move`:

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
        using the specified storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The array to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        using the specified storage.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The array to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        storage. The variable @ref array_kind
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

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
        same storage. To transfer ownership, use `std::move`:

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
        using the specified storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        using the specified storage.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        string view `s`, using the specified storage.

        @par Complexity

        Linear in `s.size()`.

        @par Exception Safety
        
        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to construct with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        storage.

        @par Complexity

        Linear in `std::strlen(s)`.

        @par Exception Safety
        
        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The null-terminated string to construct
        with.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        storage. The variable @ref string_kind
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        the specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param b The initial value.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
#ifdef GENERATING_DOCUMENTATION
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

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
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
        an @ref array is created. The contents of
        the initializer list are copied to the newly
        constructed value using the specified storage.

        @param init The initializer list to copy. 

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    value(
        std::initializer_list<value_ref> init,
        storage_ptr sp = {});

    /** Assignment.

        @par Constraints

        @par Complexity

        Constant or linear in the size of
        `*this` plus `t`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
    */
    template<class T
#ifndef GENERATING_DOCUMENTATION
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

        The value is replaced with an empty @ref object
        using the current storage, destroying the
        previous contents.

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
        using the current storage, destroying the
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
        using the current storage, destroying the
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
        value. Ownership of the respective @ref storage
        objects is not transferred.

        @li If `*other.storage() == *sp`, ownership of the
        underlying memory is swapped in constant time, with
        no possibility of exceptions. All iterators and
        references remain valid.

        @li If `*other.storage() != *sp`, the contents are
        logically swapped by making copies, which can throw.
        In this case all iterators and references are invalidated.

        @par Preconditions

        `&other != this`
        
        @par Complexity

        Constant or linear in the sum of the sizes of
        the values.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

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
    */
    json::kind
    kind() const noexcept
    {
        return static_cast<json::kind>(
            static_cast<unsigned char>(
                nul_.k) & 0x7f);
    }

    /** Returns true if this is an `object`.

        This function returns `true` when
        @ref kind() equals `kind::object`.

        @par Complexity

        Constant.
    */
    bool
    is_object() const noexcept
    {
        return kind() == json::kind::object;
    }

    /** Returns true if this is an `array`.

        This function returns `true` when
        @ref kind() equals `kind::array`.

        @par Complexity

        Constant.
    */
    bool
    is_array() const noexcept
    {
        return kind() == json::kind::array;
    }

    /** Returns true if this is a `string`.

        This function returns `true` when
        @ref kind() equals `kind::string`.

        @par Complexity

        Constant.
    */
    bool
    is_string() const noexcept
    {
        return kind() == json::kind::string;
    }

    /** Returns true if this is a `std::int64_t`.

        This function returns `true` when
        @ref kind() equals `kind::int64`.

        @par Complexity

        Constant.
    */
    bool
    is_int64() const noexcept
    {
        return kind() == json::kind::int64;
    }

    /** Returns true if this is a `std::uint64_t`.

        This function returns `true` when
        @ref kind() equals `kind::uint64`.

        @par Complexity

        Constant.
    */
    bool
    is_uint64() const noexcept
    {
        return kind() == json::kind::uint64;
    }

    /** Returns true if this is a `double`.

        This function returns `true` when
        @ref kind() equals `kind::double_`.

        @par Complexity

        Constant.
    */
    bool
    is_double() const noexcept
    {
        return kind() == json::kind::double_;
    }

    /** Returns true if this is a `bool`.

        This function returns `true` when
        @ref kind() equals `kind::bool_`.

        @par Complexity

        Constant.
    */
    bool
    is_bool() const noexcept
    {
        return kind() == json::kind::bool_;
    }

    /** Returns true if this is a null.

        This function returns `true` when
        @ref kind() equals `kind::null`.

        @par Complexity

        Constant.
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

    /** Return the storage associated with the value.

        This returns a pointer to the storage object
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

    /** Return a pointer to the underlying `object`, or `nullptr`.

        If @ref is_object() is `true`, returns
        a pointer to the underlying @ref object,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    object*
    if_object() noexcept
    {
        if(is_object())
            return &obj_;
        return nullptr;
    }

    /** Return a pointer to the underlying `object`, or `nullptr`.

        If @ref is_object() is `true`, returns
        a pointer to the underlying @ref object,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    object const*
    if_object() const noexcept
    {
        if(is_object())
            return &obj_;
        return nullptr;
    }

    /** Return a pointer to the underlying `array`, or `nullptr`.

        If @ref is_array() is `true`, returns
        a pointer to the underlying @ref array,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    array*
    if_array() noexcept
    {
        if(is_array())
            return &arr_;
        return nullptr;
    }

    /** Return a pointer to the underlying `array`, or `nullptr`.

        If @ref is_array() is `true`, returns
        a pointer to the underlying @ref array,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    array const*
    if_array() const noexcept
    {
        if(is_array())
            return &arr_;
        return nullptr;
    }

    /** Return a pointer to the underlying `string`, or `nullptr`.

        If @ref is_string() is `true`, returns
        a pointer to the underlying @ref string,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    string*
    if_string() noexcept
    {
        if(is_string())
            return &str_;
        return nullptr;
    }

    /** Return a pointer to the underlying `string`, or `nullptr`.

        If @ref is_string() is `true`, returns
        a pointer to the underlying @ref string,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    string const*
    if_string() const noexcept
    {
        if(is_string())
            return &str_;
        return nullptr;
    }

    /** Return a pointer to the underlying `std::int64`, or `nullptr`.

        If @ref is_int64() is `true`, returns
        a pointer to the underlying `std::int64`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    std::int64_t*
    if_int64() noexcept
    {
        if(is_int64())
            return &i64_.i;
        return nullptr;
    }

    /** Return a pointer to the underlying `std::int64`, or `nullptr`.

        If @ref is_int64() is `true`, returns
        a pointer to the underlying `std::int64`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    std::int64_t const*
    if_int64() const noexcept
    {
        if(is_int64())
            return &i64_.i;
        return nullptr;
    }

    /** Return a pointer to the underlying `std::uint64`, or `nullptr`.

        If @ref is_uint64() is `true`, returns
        a pointer to the underlying `std::uint64`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    std::uint64_t*
    if_uint64() noexcept
    {
        if(is_uint64())
            return &u64_.u;
        return nullptr;
    }

    /** Return a pointer to the underlying `std::uint64`, or `nullptr`.

        If @ref is_uint64() is `true`, returns
        a pointer to the underlying `std::uint64`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    std::uint64_t const*
    if_uint64() const noexcept
    {
        if(is_uint64())
            return &u64_.u;
        return nullptr;
    }

    /** Return a pointer to the underlying `double`, or `nullptr`.

        If @ref is_double() is `true`, returns
        a pointer to the underlying `double`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    double*
    if_double() noexcept
    {
        if(is_double())
            return &dub_.d;
        return nullptr;
    }

    /** Return a pointer to the underlying `double`, or `nullptr`.

        If @ref is_double() is `true`, returns
        a pointer to the underlying `double`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    double const*
    if_double() const noexcept
    {
        if(is_double())
            return &dub_.d;
        return nullptr;
    }

    /** Return a pointer to the underlying `bool`, or `nullptr`.

        If @ref is_bool() is `true`, returns
        a pointer to the underlying `bool`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    bool*
    if_bool() noexcept
    {
        if(is_bool())
            return &bln_.b;
        return nullptr;
    }

    /** Return a pointer to the underlying `bool`, or `nullptr`.

        If @ref is_bool() is `true`, returns
        a pointer to the underlying `bool`,
        otherwise returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    bool const*
    if_bool() const noexcept
    {
        if(is_bool())
            return &bln_.b;
        return nullptr;
    }

    //------------------------------------------------------

    /** Return a reference to the underlying `object`, or throw an exception.

        If @ref is_object() is `true`, returns
        a reference to the underlying @ref object,
        otherwise throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error `! this->is_object()`
    */
    object&
    as_object()
    {
        if(! is_object())
            BOOST_JSON_THROW(
                system_error(
                    error::not_object));
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

        @throw system_error `! this->is_object()`
    */
    object const&
    as_object() const
    {
        if(! is_object())
            BOOST_JSON_THROW(
                system_error(
                    error::not_object));
        return obj_;
    }

    /** Return a reference to the underlying `array`, or throw an exception.

        If @ref is_array() is `true`, returns
        a reference to the underlying @ref array,
        otherwise throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error `! this->is_array()`
    */
    array&
    as_array()
    {
        if(! is_array())
            BOOST_JSON_THROW(
                system_error(
                    error::not_array));
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

        @throw system_error `! this->is_array()`
    */
    array const&
    as_array() const
    {
        if(! is_array())
            BOOST_JSON_THROW(
                system_error(
                    error::not_array));
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

        @throw system_error `! this->is_string()`
    */
    string&
    as_string()
    {
        if(! is_string())
            BOOST_JSON_THROW(
                system_error(
                    error::not_string));
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

        @throw system_error `! this->is_string()`
    */
    string const&
    as_string() const
    {
        if(! is_string())
            BOOST_JSON_THROW(
                system_error(
                    error::not_string));
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

        @throw system_error `! this->is_int64()`
    */
    std::int64_t&
    as_int64()
    {
        if(! is_int64())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_int64()`
    */
    std::int64_t
    as_int64() const
    {
        if(! is_int64())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_uint64()`
    */
    std::uint64_t&
    as_uint64()
    {
        if(! is_uint64())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_uint64()`
    */
    std::uint64_t
    as_uint64() const
    {
        if(! is_uint64())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_double()`
    */
    double&
    as_double()
    {
        if(! is_double())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_double()`
    */
    double
    as_double() const
    {
        if(! is_double())
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
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

        @throw system_error `! this->is_bool()`
    */
    bool&
    as_bool()
    {
        if(! is_bool())
            BOOST_JSON_THROW(
                system_error(
                    error::not_bool));
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

        @throw system_error `! this->is_bool()`
    */
    bool
    as_bool() const
    {
        if(! is_bool())
            BOOST_JSON_THROW(
                system_error(
                    error::not_bool));
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

    @par Preconditions

    `&lhs != &rhs`
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
struct key_value_pair
{
    /** Copy assignment (deleted).
    */
    key_value_pair&
    operator=(key_value_pair const&) = delete;

    /** Destructor.

        The value is destroyed and all internal
        storage is freed.
    */
    BOOST_JSON_DECL
    ~key_value_pair();

    /** Copy constructor.

        This constructs a key/value pair with a
        copy of another key/value pair, using
        the same storage as `other`.

        @param other The key/value pair to copy.
    */
    BOOST_JSON_DECL
    key_value_pair(key_value_pair const& other);

    /** Copy constructor.

        This constructs a key/value pair with a
        copy of another key/value pair, using
        the specified storage.

        @param other The key/value pair to copy.

        @param sp A pointer to the @ref storage
        to use. The element will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    key_value_pair(
        key_value_pair const& other,
        storage_ptr sp);

    /** Constructor.

        This constructs a key/value pair.

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
        using the specified storage.

        @param p A `std::pair` with the key
            string and @ref value to construct with.

        @param sp A pointer to the @ref storage
        to use. The element will acquire shared
        ownership of the storage object.
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

        @param p A `std::pair` with the key
            string and @ref value to construct with.

        @param sp A pointer to the @ref storage
        to use. The element will acquire shared
        ownership of the storage object.
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
    */
    string_view const
    key() const noexcept
    {
        return { key_, len_ };
    }

    /** Return the value of this element.
    */
    json::value const&
    value() const noexcept
    {
        return value_;
    }

    /** Return the value of this element.
    */
    json::value&
    value() noexcept
    {
        return value_;
    }

private:
#ifndef GENERATING_DOCUMENTATION
    // docca emits this when it shouldn't
    friend struct detail::next_access;
#endif

    key_value_pair* next_;
    json::value value_;
    std::size_t len_;
    char* key_;
};

} // json
} // boost

// These are here because value, array,
// and object form cyclic references.

#include <boost/json/detail/impl/array_impl.hpp>
#include <boost/json/detail/impl/object_impl.hpp>
#include <boost/json/impl/array.hpp>
#include <boost/json/impl/object.hpp>

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/array_impl.ipp>
#include <boost/json/detail/impl/object_impl.ipp>
#include <boost/json/impl/array.ipp>
#include <boost/json/impl/object.ipp>
#endif

// These must come after array and object
#include <boost/json/impl/value.hpp>
#include <boost/json/impl/value_ref.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/value.ipp>
#include <boost/json/impl/value_ref.ipp>
#endif

#endif
