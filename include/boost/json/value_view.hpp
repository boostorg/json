//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_JSON_VALUE_VIEW_HPP
#define BOOST_JSON_VALUE_VIEW_HPP


#include <boost/json/detail/config.hpp>
#include <boost/json/array_view.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/object_view.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/detail/except.hpp>
#include <boost/json/detail/value_view.hpp>
#include <boost/describe/enum_to_string.hpp>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

/** The type used to represent any JSON value

    This is a view type which works like
    a variant of the basic JSON data types: array_view,
    object_view, string_view, number, boolean, and null.

    @par Thread Safety

    Distinct instances may be accessed concurrently.
*/
class value_view
{
#ifndef BOOST_JSON_DOCS
    using scalar_view = detail::scalar_view;
    union
    {
        array_view arr_;
        object_view obj_;
        detail::string_view_with_kind str_;
        scalar_view sca_;
    };
#endif

    inline bool is_scalar() const noexcept
    {
        return sca_.k < json::kind::string;
    }

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
    ~value_view() noexcept;

    /** Default constructor.

        The constructed value is null,
        using the default memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    value_view() noexcept
            : sca_()
    {
    }

    template<typename Value>
    value_view(const Value & val,
               typename std::enable_if<std::is_same<Value, value>::value>::type * = nullptr)
    {
        switch(val.kind())
        {
            case json::kind::null:    new (&sca_) scalar_view(); break;
            case json::kind::bool_:   new (&sca_) scalar_view(val.get_bool()); break;
            case json::kind::int64:   new (&sca_) scalar_view(val.get_int64()); break;
            case json::kind::uint64:  new (&sca_) scalar_view(val.get_uint64()); break;
            case json::kind::double_: new (&sca_) scalar_view(val.get_double()); break;
            case json::kind::string:
                new (&str_) detail::string_view_with_kind(val.get_string());
                break;
            case json::kind::array:
                new (&arr_) array_view(val.get_array());
                break;
            case json::kind::object:
                new (&arr_) object_view(val.get_object());
                break;
        }
    }

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
    value_view(value_view const &other) noexcept = default;

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
    value_view(value_view &&other) noexcept = default;

    //------------------------------------------------------
    //
    // Conversion
    //
    //------------------------------------------------------

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
    value_view(
            std::nullptr_t) noexcept
            : sca_()
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
    value_view(
          bool b) noexcept;
#else

    template<class Bool, class = typename std::enable_if<
            std::is_same<Bool, bool>::value>::type
    >
    value_view(
            Bool b) noexcept
            : sca_(b)
    {
    }

#endif

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
    value_view(
            signed char i) noexcept
            : sca_(static_cast<std::int64_t>(
                           i))
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
    value_view(
            short i) noexcept
            : sca_(static_cast<std::int64_t>(
                           i))
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
    value_view(
            int i) noexcept
            : sca_(static_cast<std::int64_t>(i))
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
    value_view(
            long i) noexcept
            : sca_(static_cast<std::int64_t>(i))
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
    value_view(
            long long i) noexcept
            : sca_(static_cast<std::int64_t>(i))
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
    value_view(
            unsigned char u) noexcept
            : sca_(static_cast<std::uint64_t>(
                           u))
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
    value_view(
            unsigned short u) noexcept
            : sca_(static_cast<std::uint64_t>(u))
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
    value_view(
            unsigned int u) noexcept
            : sca_(static_cast<std::uint64_t>(u))
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
    value_view(
            unsigned long u) noexcept
            : sca_(static_cast<std::uint64_t>(u))
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
    value_view(
            unsigned long long u) noexcept
            : sca_(static_cast<std::uint64_t>(u))
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
    value_view(
            double d) noexcept
            : sca_(d)
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
    value_view(
            string_view s)
            : str_(s)
    {
    }

    template<typename StringLike>
    value_view(const StringLike & s,
               typename std::enable_if<std::is_convertible<StringLike, string_view>::value>::type * = nullptr)
            : value_view(string_view(s))
    {}

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
    value_view(
            char const *s)
            : str_(s)
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
    value_view(
            string_kind_t) noexcept
            : str_()
    {
    }

#if defined(BOOST_DESCRIBE_CXX14)
    template<typename Enum>
    value_view(Enum en,
               typename std::enable_if<is_described_enum<Enum>::value>::type = nullptr
            ) : value_view(describe::enum_to_string(en, ""))
    {
        if (str_.view.empty())
            throw_exception(std::invalid_argument("Invalid enum value"));
    }
#endif

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
    value_view(const array_view &other) noexcept
            : arr_(std::move(other))
    {
    }

    template<typename Range >
    value_view(const Range & r,
               typename std::enable_if<std::is_convertible<Range, array_view>::value>::type * = nullptr)
             : value_view(array_view(r))
    {}

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
    value_view(
            array_kind_t) noexcept
            : arr_()
    {
    }

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
    value_view(object_view other) noexcept
            : obj_(std::move(other))
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
    value_view(
            object_kind_t) noexcept
            : obj_()
    {
    }

    //------------------------------------------------------
    //
    // Assignment
    //
    //------------------------------------------------------

    /** Copy assignment.

        The contents of the value are replaced with an
        element-wise copy of the contents of `other`.

        @par Complexity
        Linear in the size of `*this` plus `other`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The value to copy.
    */
    value_view &
    operator=(value_view const &other) = default;

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
    value_view &
    operator=(value_view &&other) = default;

    /** Assignment.

        Replace `*this` with null.

        @par Exception Safety
        No-throw guarantee.

        @par Complexity
        Linear in the size of `*this`.
    */
    value_view &
    operator=(std::nullptr_t) noexcept
    {
        if (is_scalar())
        {
            sca_.k = json::kind::null;
        } else
        {
            ::new(&sca_) scalar_view();
        }
        return *this;
    }

    /** Assignment.

        Replace `*this` with `b`.

        @par Exception Safety
        No-throw guarantee.

        @par Complexity
        Linear in the size of `*this`.

        @param b The new value.
    */
#ifdef BOOST_JSON_DOCS
    value_view& operator=(bool b) noexcept;
#else

    template<class Bool, class = typename std::enable_if<
            std::is_same<Bool, bool>::value>::type
    >
    value_view &operator=(Bool b) noexcept
    {
        if (is_scalar())
        {
            sca_.b = b;
            sca_.k = json::kind::bool_;
        } else
        {
            ::new(&sca_) scalar_view();
        }
        return *this;
    }

#endif

    /** Assignment.

        Replace `*this` with `i`.

        @par Exception Safety
        No-throw guarantee.

        @par Complexity
        Linear in the size of `*this`.

        @param i The new value.
    */
    /** @{ */
    value_view &operator=(signed char i) noexcept
    {
        return operator=(
                static_cast<long long>(i));
    }

    value_view &operator=(short i) noexcept
    {
        return operator=(
                static_cast<long long>(i));
    }

    value_view &operator=(int i) noexcept
    {
        return operator=(
                static_cast<long long>(i));
    }

    value_view &operator=(long i) noexcept
    {
        return operator=(
                static_cast<long long>(i));
    }

    value_view &operator=(long long i) noexcept
    {
        if (is_scalar())
        {
            sca_.i = i;
            sca_.k = json::kind::int64;
        } else
        {
            ::new(&sca_) scalar_view(static_cast<
                                    std::int64_t>(i));
        }
        return *this;
    }
    /** @} */

    /** Assignment.

        Replace `*this` with `i`.

        @par Exception Safety
        No-throw guarantee.

        @par Complexity
        Linear in the size of `*this`.

        @param u The new value.
    */
    /** @{ */
    value_view &operator=(unsigned char u) noexcept
    {
        return operator=(static_cast<
                                 unsigned long long>(u));
    }

    value_view &operator=(unsigned short u) noexcept
    {
        return operator=(static_cast<
                                 unsigned long long>(u));
    }

    value_view &operator=(unsigned int u) noexcept
    {
        return operator=(static_cast<
                                 unsigned long long>(u));
    }

    value_view &operator=(unsigned long u) noexcept
    {
        return operator=(static_cast<
                                 unsigned long long>(u));
    }

    value_view &operator=(unsigned long long u) noexcept
    {
        if (is_scalar())
        {
            sca_.u = u;
            sca_.k = json::kind::uint64;
        } else
        {
            ::new(&sca_) scalar_view(static_cast<std::uint64_t>(u));
        }
        return *this;
    }
    /** @} */

    /** Assignment.

        Replace `*this` with `d`.

        @par Exception Safety
        No-throw guarantee.

        @par Complexity
        Linear in the size of `*this`.

        @param d The new value.
    */
    value_view &operator=(double d) noexcept
    {
        if (is_scalar())
        {
            sca_.d = d;
            sca_.k = json::kind::double_;
        } else
        {
            ::new(&sca_) scalar_view(d);
        }
        return *this;
    }

    /** Assignment.

        Replace `*this` with a copy of the string `s`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @par Complexity
        Linear in the sum of sizes of `*this` and `s`

        @param s The new string.
    */
    /** @{ */
    value_view &operator=(string_view s)
    {
        *this = value_view(s);
        return *this;
    }

    value_view &operator=(char const *s)
    {
        *this = value_view(s);
        return *this;
    }
    /** @} */

    /** Assignment.

        Replace `*this` with a copy of the array `arr`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @par Complexity
        Linear in the sum of sizes of `*this` and `arr`

        @param arr The new array.
    */
    value_view &operator=(array_view arr)
    {
        *this = value_view(arr);
        return *this;
    }

    /** Assignment.

        Replace `*this` with a copy of the obect `obj`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @par Complexity
        Linear in the sum of sizes of `*this` and `obj`

        @param obj The new object.
    */
    value_view &operator=(object_view obj)
    {
        *this = value_view(obj);
        return *this;
    }
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
                        sca_.k) & 0x3f);
    }

    /** Return `true` if this is an array

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::array;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_array() const noexcept
    {
        return kind() == json::kind::array;
    }

    /** Return `true` if this is an object

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::object;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_object() const noexcept
    {
        return kind() == json::kind::object;
    }

    /** Return `true` if this is a string

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::string;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_string() const noexcept
    {
        return kind() == json::kind::string;
    }

    /** Return `true` if this is a signed integer

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::int64;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_int64() const noexcept
    {
        return kind() == json::kind::int64;
    }

    /** Return `true` if this is a unsigned integer

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::uint64;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_uint64() const noexcept
    {
        return kind() == json::kind::uint64;
    }

    /** Return `true` if this is a double

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::double_;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_double() const noexcept
    {
        return kind() == json::kind::double_;
    }

    /** Return `true` if this is a bool

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::bool_;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    bool
    is_bool() const noexcept
    {
        return kind() == json::kind::bool_;
    }

    /** Returns true if this is a null.

        This function is used to determine if the underlying
        representation is a certain kind.

        @par Effects
        @code
        return this->kind() == kind::null;
        @endcode

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
                sca_.k != json::kind::object &&
                sca_.k != json::kind::array;
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

    /** Return an @ref array pointer if this is an array, else return `nullptr`

        If `this->kind() == kind::array`, returns a pointer
        to the underlying array. Otherwise, returns `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_array() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const array_view *
    if_array() const noexcept
    {
        if (kind() == json::kind::array)
            return &arr_;
        return nullptr;
    }

    /** Return an @ref object pointer if this is an object, else return `nullptr`

        If `this->kind() == kind::object`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_object() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const object_view *
    if_object() const noexcept
    {
        if (kind() == json::kind::object)
            return &obj_;
        return nullptr;
    }

    /** Return a @ref string pointer if this is a string, else return `nullptr`

        If `this->kind() == kind::string`, returns a pointer
        to the underlying object. Otherwise, returns `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_string() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const string_view *
    if_string() const noexcept
    {
        if (kind() == json::kind::string)
            return &str_.view;
        return nullptr;
    }

    /** Return an `int64_t` pointer if this is a signed integer, else return `nullptr`

        If `this->kind() == kind::int64`, returns a pointer
        to the underlying integer. Otherwise, returns `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_int64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const std::int64_t *
    if_int64() const noexcept
    {
        if (kind() == json::kind::int64)
            return &sca_.i;
        return nullptr;
    }

    /** Return a `uint64_t` pointer if this is an unsigned integer, else return `nullptr`

        If `this->kind() == kind::uint64`, returns a pointer
        to the underlying unsigned integer. Otherwise, returns
        `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_uint64() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const std::uint64_t *
    if_uint64() const noexcept
    {
        if (kind() == json::kind::uint64)
            return &sca_.u;
        return nullptr;
    }

    /** Return a `double` pointer if this is a double, else return `nullptr`

        If `this->kind() == kind::double_`, returns a pointer
        to the underlying double. Otherwise, returns
        `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_double() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const double *
    if_double() const noexcept
    {
        if (kind() == json::kind::double_)
            return &sca_.d;
        return nullptr;
    }

    /** Return a `bool` pointer if this is a boolean, else return `nullptr`

        If `this->kind() == kind::bool_`, returns a pointer
        to the underlying boolean. Otherwise, returns
        `nullptr`.

        @par Example
        The return value is used in both a boolean context and
        to assign a variable:
        @code
        if( auto p = jv.if_bool() )
            return *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    const bool *
    if_bool() const noexcept
    {
        if (kind() == json::kind::bool_)
            return &sca_.b;
        return nullptr;
    }

    //------------------------------------------------------

    /** Return the stored number cast to an arithmetic type.

        This function attempts to return the stored value
        converted to the arithmetic type `T` which may not
        be `bool`:

        @li If `T` is an integral type and the stored
        value is a number which can be losslessly converted,
        the conversion is performed without error and the
        converted number is returned.

        @li If `T` is an integral type and the stored value
        is a number which cannot be losslessly converted,
        then the operation fails with an error.

        @li If `T` is a floating point type and the stored
        value is a number, the conversion is performed
        without error. The converted number is returned,
        with a possible loss of precision.

        @li Otherwise, if the stored value is not a number;
        that is, if `this->is_number()` returns `false`, then
        the operation fails with an error.

        @par Constraints
        @code
        std::is_arithmetic< T >::value && ! std::is_same< T, bool >::value
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @return The converted number.

        @param ec Set to the error, if any occurred.
    */
/** @{ */
    template<class T>
#ifdef BOOST_JSON_DOCS
    T
#else
    typename std::enable_if<
            std::is_arithmetic<T>::value &&
            !std::is_same<T, bool>::value,
            T>::type
#endif
    to_number(error_code &ec) const noexcept
    {
        error e;
        auto result = to_number < T > (e);
        BOOST_JSON_FAIL(ec, e);
        return result;
    }

    template<class T>
#ifdef BOOST_JSON_DOCS
    T
#else
    typename std::enable_if<
            std::is_arithmetic<T>::value &&
            !std::is_same<T, bool>::value,
            T>::type
#endif
    to_number(std::error_code &ec) const noexcept
    {
        error_code jec;
        auto result = to_number < T > (jec);
        ec = jec;
        return result;
    }
/** @} */

    /** Return the stored number cast to an arithmetic type.

        This function attempts to return the stored value
        converted to the arithmetic type `T` which may not
        be `bool`:

        @li If `T` is an integral type and the stored
        value is a number which can be losslessly converted,
        the conversion is performed without error and the
        converted number is returned.

        @li If `T` is an integral type and the stored value
        is a number which cannot be losslessly converted,
        then the operation fails with an error.

        @li If `T` is a floating point type and the stored
        value is a number, the conversion is performed
        without error. The converted number is returned,
        with a possible loss of precision.

        @li Otherwise, if the stored value is not a number;
        that is, if `this->is_number()` returns `false`, then
        the operation fails with an error.

        @par Constraints
        @code
        std::is_arithmetic< T >::value && ! std::is_same< T, bool >::value
        @endcode

        @par Complexity
        Constant.

        @return The converted number.

        @throw system_error on error.
    */
    template<class T>
#ifdef BOOST_JSON_DOCS
    T
#else
    typename std::enable_if<
            std::is_arithmetic<T>::value &&
            !std::is_same<T, bool>::value,
            T>::type
#endif
    to_number() const
    {
        error_code ec;
        auto result = to_number < T > (ec);
        if (ec)
            detail::throw_system_error(ec);
        return result;
    }

    //------------------------------------------------------
    //
    // Accessors
    //
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
    /* @{ */
    object_view
    as_object() const &
    {
        if (!is_object())
            detail::throw_invalid_argument("not an object");
        return obj_;
    }
    /* @} */

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
    /* @{ */
    array_view
    as_array() const
    {
        if (!is_array())
            detail::throw_invalid_argument("array required");
        return arr_;
    }
    /* @} */

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
    /* @{ */
    string_view
    as_string() const
    {
        if (!is_string())
            detail::throw_invalid_argument("not a string");
        return str_.view;
    }
    /* @} */

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
        if (!is_int64())
            detail::throw_invalid_argument("not an int64");
        return sca_.i;
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
        if (!is_uint64())
            detail::throw_invalid_argument("not a uint64");
        return sca_.u;
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
        if (!is_double())
            detail::throw_invalid_argument("not a double");
        return sca_.d;
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
        if (!is_bool())
            detail::throw_invalid_argument("bool required");
        return sca_.b;
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
    /* @{ */
    object_view
    get_object() const noexcept
    {
        BOOST_ASSERT(is_object());
        return obj_;
    }
    /* @} */

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
    /* @{ */
    array_view const
    get_array() const noexcept
    {
        BOOST_ASSERT(is_array());
        return arr_;
    }
    /* @} */

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
    /* @{ */
    string_view
    get_string() const noexcept
    {
        BOOST_ASSERT(is_string());
        return str_.view;
    }
    /* @} */

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
        return sca_.i;
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
        return sca_.u;
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
        return sca_.d;
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
        return sca_.b;
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
/** @{ */
    value_view
    at(string_view key) const
    {
        return as_object().at(key);
    }
/** @} */

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
/** @{ */
    value_view
    at(std::size_t pos) const
    {
        return as_array().at(pos);
    }
/** @} */

    /** Access an element via JSON Pointer.

        This function is used to access a (potentially nested)
        element of the value using a JSON Pointer string.

        @par Complexity
        Linear in the sizes of `ptr` and underlying array, object, or string.

        @par Exception Safety
        Strong guarantee.

        @param ptr JSON Pointer string.

        @return reference to the element identified by `ptr`.

        @throw system_error if an error occurs.

        @see
        <a href="https://datatracker.ietf.org/doc/html/rfc6901">
            RFC 6901 - JavaScript Object Notation (JSON) Pointer</a>
    */
/** @{ */
    BOOST_JSON_DECL
    value_view
    at_pointer(string_view ptr) const;

/** @} */

    /** Access an element via JSON Pointer.

        This function is used to access a (potentially nested)
        element of the value using a JSON Pointer string.

        @par Complexity
        Linear in the sizes of `ptr` and underlying array, object, or string.

        @par Exception Safety
        No-throw guarantee.

        @param ptr JSON Pointer string.

        @param ec Set to the error, if any occurred.

        @return pointer to the element identified by `ptr`.

        @see
        <a href="https://datatracker.ietf.org/doc/html/rfc6901">
            RFC 6901 - JavaScript Object Notation (JSON) Pointer</a>
    */
/** @{ */
    BOOST_JSON_DECL
    boost::optional<value_view>
    find_pointer(string_view ptr, error_code &ec) const noexcept;

    BOOST_JSON_DECL
    boost::optional<value_view>
    find_pointer(string_view ptr, std::error_code &ec) const noexcept;

/** @} */

    //------------------------------------------------------

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
            value_view const &lhs,
            value_view const &rhs) noexcept
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
            value_view const &lhs,
            value_view const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Serialize @ref value to an output stream.

        This function serializes a `value` as JSON into the output stream.

        @return Reference to `os`.

        @par Complexity
        Constant or linear in the size of `jv`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param os The output stream to serialize to.

        @param jv The value to serialize.
    */
    BOOST_JSON_DECL
    friend
    std::ostream &
    operator<<(
            std::ostream &os,
            value_view const &jv);

    /** Parse @ref value from an input stream.

        This function parses JSON from an input stream into a `value`. If
        parsing fails, `std::ios_base::failbit` will be set for `is` and
        `jv` will be left unchanged. Regardless of whether `skipws` flag is set
        on `is`, consumes whitespace before and after JSON, because whitespace
        is considered a part of JSON. Behaves as [_FormattedInputFunction_]
        (https://en.cppreference.com/w/cpp/named_req/FormattedInputFunction).<br>

        Note: this operator cannot assume that the stream only contains a
        single JSON document, which may result in **very underwhelming
        performance**, if the stream isn't cooperative. If you know that your
        input consists of a single JSON document, consider using @ref parse
        function instead.

        @return Reference to `is`.

        @par Complexity
        Linear in the size of JSON data.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
        The stream may throw as configured by
        [`std::ios::exceptions`](https://en.cppreference.com/w/cpp/io/basic_ios/exceptions).

        @param is The input stream to parse from.

        @param jv The value to parse into.

        @see @ref parse.
    */
    BOOST_JSON_DECL
    friend
    std::istream &
    operator>>(
            std::istream &is,
            value_view &jv);

    /** Helper for `boost::hash` support

        Computes a hash value for `jv`. This function is used by
        `boost::hash<value>`. Similar overloads for @ref array, @ref object,
        and @ref string do not exist, because those types are supported by
        `boost::hash` out of the box.

        @return hash value for `jv`.

        @param jv `value` for which a hash is to be computed.

        @see [Boost.ContainerHash](https://boost.org/libs/container_hash).
     */
#ifndef BOOST_JSON_DOCS

    template<
            class T,
            typename std::enable_if<
                    std::is_same<detail::remove_cvref<T>, value_view>::value>::type *
            = nullptr>
    friend
    std::size_t
    hash_value(T const &jv) noexcept
#else
    friend
      inline
      std::size_t
      hash_value( value const& jv ) noexcept
#endif
    {
        return detail::hash_value_impl(jv);
    }

private:
    BOOST_JSON_DECL
    bool
    equal(value_view const &other) const noexcept;
};


//----------------------------------------------------------

/** A key/value pair.

    This is the type of element used by the @ref object
    container.
*/
class key_value_view_pair
{

 public:
  /// Copy assignment (deleted).
  key_value_view_pair&
  operator=(key_value_view_pair const&) = default;

  /** Destructor.

      The value is destroyed and all internally
      allocated memory is freed.
  */
  ~key_value_view_pair() noexcept = default;

  /** Copy constructor.

      This constructs a key/value pair with a
      copy of another key/value pair, using
      the specified memory resource.

      @par Exception Safety
      Strong guarantee.

      @param other The key/value pair to reference.

      @param sp A pointer to the @ref memory_resource
      to use. The element will acquire shared
      ownership of the memory resource.
  */
    key_value_view_pair(
      key_value_view_pair const& other) = default;

    key_value_view_pair(
        string_view key,
        json::value_view value) : key_(key), value_(value) {}

    key_value_view_pair(key_value_pair const& other);

  /** Move constructor.

      The pair is constructed by acquiring
      ownership of the contents of `other` and
      shared ownership of `other`'s memory resource.

      @note

      After construction, the moved-from pair holds an
      empty key, and a null value with its current
      storage pointer.

      @par Complexity
      Constant.

      @par Exception Safety
      No-throw guarantee.

      @param other The pair to move.
  */
  key_value_view_pair(
      key_value_view_pair&& other) noexcept = default;

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
  key_value_view_pair(
      std::pair<
          string_view,
          json::value_view> const& p)
      : key_value_view_pair(p.first, p.second)
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
  key_value_view_pair(
      std::pair<
          string_view,
          json::value_view>&& p)
      : key_value_view_pair(
      p.first,
      std::move(p).second)
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
    return key_;
  }

  /** Return the value of this element.

      @par Complexity
      Constant.

      @par Exception Safety
      No-throw guarantee.
  */
  /* @{ */
  json::value_view const&
  value() const noexcept
  {
    return value_;
  }
  /* @} */

 private:
    string_view key_;
    json::value_view value_;
};

//----------------------------------------------------------

#ifdef BOOST_JSON_DOCS

/** Tuple-like element access.

    This overload permits the key and value
    of a `key_value_view_pair` to be accessed
    by index. For example:

    @code

    key_value_view_pair kvp("num", 42);

    string_view key = get<0>(kvp);
    value_view& jv = get<1>(kvp);

    @endcode

    @par Structured Bindings

    When using C++17 or greater, objects of type
    @ref key_value_view_pair may be used to initialize
    structured bindings:

    @code

    key_value_view_pair kvp("num", 42);

    auto& [key, value] = kvp;

    @endcode

    Depending on the value of `I`, the return type will be:

    @li `string_view const` if `I == 0`, or

    @li `value_view&`, `value const&`, or `value_view&&` if `I == 1`.

    Any other value for `I` is ill-formed.

    @tparam I The element index to access.

    @par Constraints

    `std::is_same_v< std::remove_cvref_t<T>, key_value_view_pair >`

    @return `kvp.key()` if `I == 0`, or `kvp.value_view()`
    if `I == 1`.

    @param kvp The @ref key_value_view_pair object
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
get(key_value_view_pair const&) noexcept ->
typename std::conditional<I == 0,
    string_view const,
    value_view const&>::type
{
  static_assert(I == 0,
                "key_value_view_pair index out of range");
}

/** Extracts a key_value_view_pair's key using tuple-like interface
*/
template<>
inline
string_view const
get<0>(key_value_view_pair const& kvp) noexcept
{
  return kvp.key();
}

/** Extracts a key_value_view_pair's value using tuple-like interface
*/
template<>
inline
value_view const&
get<1>(key_value_view_pair const& kvp) noexcept
{
  return kvp.value();
}

#endif

} // namespace json
} // namespace boost

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

#ifndef BOOST_JSON_DOCS

namespace std {

/** Tuple-like access for the key type of key_value_view_pair
*/
template<>
struct tuple_element<0, ::boost::json::key_value_view_pair>
{
  using type = ::boost::json::string_view ;
};

/** Tuple-like access for the value type of key_value_view_pair
*/
template<>
struct tuple_element<1, ::boost::json::key_value_view_pair>
{
  using type = ::boost::json::value_view;
};

template<>
struct tuple_size< ::boost::json::key_value_view_pair >
        : std::integral_constant<std::size_t, 2>
{
};

} // std

#endif

// std::hash specialization
#ifndef BOOST_JSON_DOCS
namespace std {
template <>
struct hash< ::boost::json::value_view > {
  BOOST_JSON_DECL
  std::size_t
  operator()(::boost::json::value_view const& jv) const noexcept;
};
} // std
#endif


#ifdef __clang__
# pragma clang diagnostic pop
#endif

// These are here because value, array,
// and object form cyclic references.

#include <boost/json/impl/array_view.hpp>
#include <boost/json/impl/object_view.hpp>

#endif //BOOST_JSON_VALUE_VIEW_HPP
