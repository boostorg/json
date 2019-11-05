//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_HPP
#define BOOST_JSON_VALUE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/array.hpp>
#include <boost/json/error.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/object.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string.hpp>
#include <boost/json/detail/is_specialized.hpp>
#include <boost/json/detail/value.hpp>
#include <boost/json/detail/assert.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iosfwd>
#include <new>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

class value;

/** Customization point for assigning to and from class types.
*/
template<class T>
struct value_exchange final
#ifndef GENERATING_DOCUMENTATION
    : detail::primary_template
#endif
{
    static
    void
    to_json(T const& t, value& v)
    {
        detail::call_to_json(t, v);
    }

    static
    void
    from_json(T& t, value const& v)
    {
        detail::call_from_json(t, v);
    }
};

/** Trait to determine if a type can be assigned to a json value.
*/
template<class T>
using has_from_json =
#ifdef GENERATING_DOCUMENTATION
    __see_below__;
#else
    std::integral_constant<bool,
        detail::is_specialized<value_exchange<
            detail::remove_cr<T>>>::value ||
        detail::has_adl_from_json<
            detail::remove_cr<T>>::value ||
        detail::has_mf_from_json<
            detail::remove_cr<T>>::value>;
#endif

/** Returns `true` if a JSON value can be constructed from `T`
*/
template<class T>
using has_to_json =
#ifdef GENERATING_DOCUMENTATION
    __see_below__;
#else
    std::integral_constant<bool,
        detail::is_specialized<value_exchange<
            detail::remove_cr<T>>>::value ||
        detail::has_adl_to_json<
            detail::remove_cr<T>>::value ||
        detail::has_mf_to_json<
            detail::remove_cr<T>>::value>;
#endif

//----------------------------------------------------------

/** The type used to represent any JSON value

    @par Thread Safety:

    Distinct instances may be accessed concurrently.
    Non-const member functions of a shared instance
    may not be called concurrently with any other
    member functions of that instance.
*/
class value
{
    struct undo;
    struct init_iter;
    friend class value_test;

#ifndef GENERATING_DOCUMENTATION
    struct scalar
    {
        storage_ptr sp;
        union
        {
            std::uint64_t u;
            std::int64_t i;
            double d;
            bool b;
        };

        scalar() = default;
        scalar(storage_ptr sp_)
            : sp(std::move(sp_))
        {
        }

        ~scalar()
        {
        }
    };

    // XSL scripts have trouble with private anon unions
    union
    {
        object      obj_;
        array       arr_;
        string      str_;
        scalar      sca_;
    };
#endif

    json::kind kind_;

public:
    /** Destructor.

        The value and all of its contents are destroyed.
        Any dynamically allocated internal storage
        is freed.

        @par Complexity

        Linear in the size of `*this`.
    */
    BOOST_JSON_DECL
    ~value();

    /** Default constructor.

        Default constructed values are null.
        The container and all of its contents will use the
        specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp The storage to use. If this parameter
        is omitted, the default storage is used.
    */
    explicit
    value(storage_ptr sp = {}) noexcept
        : sca_(std::move(sp))
        , kind_(json::kind::null)
    {
    }

    /** Pilfer constructor.

        Constructs the container with the contents
        of `other` using pilfer semantics.
        Ownership of the storage is transferred.

        @note

        After construction, the moved-from object may
        only be destroyed.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The container to pilfer.

        @see
        
        Pilfering constructors are described in
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">Valueless Variants Considered Harmful</a>, by Peter Dimov.
    */
    BOOST_JSON_DECL
    value(pilfered<value> other) noexcept;

    /** Copy constructor.

        The container and all of its contents will use
        the same storage as `other`.

        @par Complexity

        Linear in the size of `other`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The value to copy.
    */
    value(value const& other)
        : value(other, other.get_storage())
    {
    }

    /** Copy constructor

        The container and all of its contents will use the
        specified storage.

        @par Complexity

        Linear in the size of `other`.

        @param other The value to copy.

        @param sp The storage to use.
    */
    BOOST_JSON_DECL
    value(
        value const& other,
        storage_ptr sp);

    /** Move constructor

        Constructs the container with the contents of `other`
        using move semantics. Ownership of the underlying
        memory is transferred.
        The container acquires shared ownership of the
        storage used by `other`.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The value to move.
    */
    BOOST_JSON_DECL
    value(value&& other) noexcept;

    /** Move constructor

        Using `*sp` as the storage for the new container,
        moves all the elements from `other`.

        @li If `*other.get_storage() == *sp`, ownership of the
        underlying memory is transferred in constant time, with
        no possibility of exceptions.

        @li If `*other.get_storage() != *sp`, a copy is performed.
        In this case, the moved-from container is not changed.

        The container and all of its contents will use the
        specified storage object.
        
        @par Complexity

        Constant or linear in the size of `other`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to move.

        @param sp The storage to use.
    */
    BOOST_JSON_DECL
    value(
        value&& other,
        storage_ptr sp);

    /** Move assignment operator

        Replaces the contents with those of `other` using move
        semantics (the data in `other` is moved into this container).

        @li If `*other.get_storage() == get_storage()`,
        ownership of the  underlying memory is transferred in
        constant time, with no possibility of exceptions.

        @li If `*other.get_storage() != *sp`, a copy is performed.
        In this case the moved-from container is not modified,
        and exceptions may be thrown.

        @par Complexity

        Constant or linear in the size of `*this` plus `other`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The value to assign from.
    */
    BOOST_JSON_DECL
    value& operator=(value&& other);

    /** Copy assignment operator

        Replaces the contents with a copy of `other`.

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

    /** Construct an object.
    */
    value(object obj) noexcept
        : obj_(std::move(obj))
        , kind_(json::kind::object)
    {
    }

    /** Construct an object.
    */
    value(object obj, storage_ptr sp)
        : obj_(std::move(obj), std::move(sp))
        , kind_(json::kind::object)
    {
    }

    /** Construct an object.
    */
    value(object_kind_t,
        storage_ptr sp = {}) noexcept
        : obj_(std::move(sp))
        , kind_(json::kind::object)
    {
    }

    /** Construct an array.
    */
    value(array arr) noexcept
        : arr_(std::move(arr))
        , kind_(json::kind::array)
    {
    }

    /** Construct an array.
    */
    value(array arr, storage_ptr sp)
        : arr_(std::move(arr), std::move(sp))
        , kind_(json::kind::array)
    {
    }

    /** Construct an array.
    */
    value(array_kind_t,
        storage_ptr sp = {}) noexcept
        : arr_(std::move(sp))
        , kind_(json::kind::array)
    {
    }

    /** Construct a string.
    */
    value(
        string str) noexcept
        : str_(std::move(str))
        , kind_(json::kind::string)
    {
    }

    /** Construct a string.
    */
    value(string str, storage_ptr sp)
        : str_(std::move(str), std::move(sp))
        , kind_(json::kind::string)
    {
    }

    /** Construct a string.
    */
    value(
        string_view s,
        storage_ptr sp = {})
        : str_(s, std::move(sp))
        , kind_(json::kind::string)
    {
    }

    /** Construct a string.
    */
    value(
        char const* s,
        storage_ptr sp = {})
        : str_(s, std::move(sp))
        , kind_(json::kind::string)
    {
    }

    /** Construct a signed integer.
    */
    value(short i, storage_ptr sp = {})
        : kind_(json::kind::int64)
    {
        sca_.i = i;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a signed integer.
    */
    value(int i, storage_ptr sp = {})
        : kind_(json::kind::int64)
    {
        sca_.i = i;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a signed integer.
    */
    value(long i, storage_ptr sp = {})
        : kind_(json::kind::int64)
    {
        sca_.i = i;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a signed integer.
    */
    value(long long i, storage_ptr sp = {})
        : kind_(json::kind::int64)
    {
        sca_.i = i;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct an unsigned integer.
    */
    value(unsigned short u, storage_ptr sp = {})
        : kind_(json::kind::uint64)
    {
        sca_.u = u;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct an unsigned integer.
    */
    value(unsigned int u, storage_ptr sp = {})
        : kind_(json::kind::uint64)
    {
        sca_.u = u;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct an unsigned integer.
    */
    value(unsigned long u, storage_ptr sp = {})
        : kind_(json::kind::uint64)
    {
        sca_.u = u;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct an unsigned integer.
    */
    value(unsigned long long u, storage_ptr sp = {})
        : kind_(json::kind::uint64)
    {
        sca_.u = u;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a floating point number.
    */
    value(float d,
        storage_ptr sp = {})
        : kind_(json::kind::double_)
    {
        sca_.d = d;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a floating point number.
    */
    value(double d,
        storage_ptr sp = {})
        : kind_(json::kind::double_)
    {
        sca_.d = d;
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a floating point number.
    */
    value(long double d,
        storage_ptr sp = {})
        : kind_(json::kind::double_)
    {
        sca_.d = static_cast<double>(d);
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
    }

    /** Construct a boolean.
    */
#ifdef GENERATING_DOCUMENTATION
    value(bool b,
        storage_ptr = {}) noexcept;
#else
    template<class Bool
        ,class = typename std::enable_if<
            std::is_same<Bool, bool>::value>::type
    >
    value(Bool b,
        storage_ptr sp = {}) noexcept;
#endif

    /** Construct a null
    */
    value(
        std::nullptr_t,
        storage_ptr sp = {})
        : sca_(std::move(sp))
        , kind_(json::kind::null)
    {
    }

    /** Construct an object or array
    */
    BOOST_JSON_DECL
    value(
        std::initializer_list<value> init,
        storage_ptr sp = {});

    /** Assign to this value
    */
    template<class T
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<
            value, T, storage_ptr>::value>::type
#endif
    >
    value&
    operator=(T&& t)
    {
        value tmp(
            std::forward<T>(t),
            get_storage());
        tmp.swap(*this);
        return *this;
    }

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Set the value to an empty object, and return it.

        This calls `reset(json::kind::object)` and returns
        @ref as_object().
        The previous contents are destroyed.

        @par Complexity

        Linear in the existing size of `*this`.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    object&
    emplace_object() noexcept;

    /** Set the value to an empty array, and return it.

        This calls `reset(json::kind::array)` and returns
        @ref as_array().
        The previous contents are destroyed.

        @par Complexity

        Linear in the existing size of `*this`.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    array&
    emplace_array() noexcept;

    /** Set the value to an empty string, and return it.

        This calls `reset(json::kind::string)` and returns
        @ref as_string().
        The previous contents are destroyed.

        @par Complexity

        Linear in the existing size of `*this`.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    string&
    emplace_string() noexcept;

    BOOST_JSON_DECL
    std::int64_t&
    emplace_int64() noexcept;

    BOOST_JSON_DECL
    std::uint64_t&
    emplace_uint64() noexcept;

    BOOST_JSON_DECL
    double&
    emplace_double() noexcept;

    /** Set the value to boolean false, and return it.

        This calls `reset(json::kind::boolean)` and returns
        @ref as_bool().
        The previous contents are destroyed.

        @par Complexity

        Linear in the existing size of `*this`.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    bool&
    emplace_bool() noexcept;

    /** Set the value to null.

        This calls `reset(json::kind::null)`.
        The previous contents are destroyed.

        @par Complexity

        Linear in the existing size of `*this`.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    emplace_null() noexcept;

    /** Swap the contents

        Exchanges the contents of this container with another
        container.
        All iterators and references remain valid.

        @par Precondition

        `*get_storage() == *other.get_storage()`

        @par Complexity

        Constant or linear in the size of `*this` plus `other`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The container to swap with
    */
    BOOST_JSON_DECL
    void
    swap(value& other);

    //------------------------------------------------------
    //
    // Exchange
    //
    //------------------------------------------------------

    /// Construct from another type using the specified storage
    template<
        class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            has_to_json<T>::value>::type
    #endif
    >
    value(
        T const& t,
        storage_ptr sp = {})
        : value(std::move(sp))
    {
        value_exchange<
            detail::remove_cr<T>
                >::to_json(t, *this);
    }

    /// Assign a value from another type
    template<
        class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            has_to_json<T>::value>::type
    #endif
    >
    value&
    operator=(T const& t)
    {
        value_exchange<
            detail::remove_cr<T>
                >::to_json(t, *this);
        return *this;
    }

    /** Try to assign a value to another type

        This function attempts to assign the contents of
        `*this` to the variable `t`.

        @par Complexity

        Linear in the size of `*this`.

        @par Exception Safety

        Strong guarantee.

        @throws system error Thrown upon failure
    */
    template<class T>
    void
    store(T& t) const
    {
        // If this assert goes off, it means that there are no known
        // ways to convert a JSON value into a user defined type `T`.
        // There are three ways to fix this:
        //
        // 1. Add the member function `T::from_json(value const&)`,
        //
        // 2. Add the free function `from_json(T&, value const&)`
        //    in the same namespace as T, or
        //
        // 3. Specialize `json::value_exchange` for `T`, and provide
        //    the static member `from_json(T&, value const&)`.

        static_assert(
            has_from_json<T>::value,
            "Destination type is unknown");
        value_exchange<
            detail::remove_cr<T>
                >::from_json(t, *this);
    }

    //------------------------------------------------------

    /** Returns `true` if this is an array containing only a key and value

        This function returns `true` if all the following
        conditions are met:

        @li @ref kind() returns `kind::array`
        @li `this->as_array().size() == 2`
        @li `this->as_array()[0].is_string() == true`
        
        Otherwise, the function returns `false`.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    bool
    is_key_value_pair() const noexcept;

    /** Returns `true` if the initializer list consists only of key-value pairs

        This function returns `true` if @ref is_key_value_pair()
        is true for every element in the initializer list.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        No-throw guarantee.

        @param init The initializer list to inspect.
    */
    static
    BOOST_JSON_DECL
    bool
    maybe_object(
        std::initializer_list<value> init) noexcept;

    //------------------------------------------------------
    //
    // Observers
    //
    //------------------------------------------------------

    /** Returns the kind of this JSON value

        This function returns the discriminating enumeration
        constant of type @ref json::kind corresponding to the type
        of value held in the union.

        @par Complexity

        Constant.
    */
    json::kind
    kind() const noexcept
    {
        return kind_;
    }

    /** Returns true if this is an object

        This function returns `true` if
        @ref kind() equals `kind::object`.

        @par Complexity

        Constant.
    */
    bool
    is_object() const noexcept
    {
        return kind_ == json::kind::object;
    }

    /** Returns true if this is an array

        This function returns `true` if
        @ref kind() equals `kind::array`.

        @par Complexity

        Constant.
    */
    bool
    is_array() const noexcept
    {
        return kind_ == json::kind::array;
    }

    /** Returns true if this is a string

        This function returns `true` if
        @ref kind() equals `kind::string`.

        @par Complexity

        Constant.
    */
    bool
    is_string() const noexcept
    {
        return kind_ == json::kind::string;
    }

    /** Returns true if this is a number

        This function returns `true` if
        the value contained is a signed or
        unsigned integer, or a double precision
        floating point.

        @par Complexity

        Constant.
    */
    bool
    is_number() const noexcept
    {
        return
            kind_ == json::kind::int64 ||
            kind_ == json::kind::uint64 ||
            kind_ == json::kind::double_;
    }

    bool
    is_int64() const noexcept
    {
        return kind_ == json::kind::int64;
    }

    bool
    is_uint64() const noexcept
    {
        return kind_ == json::kind::uint64;
    }

    bool
    is_double() const noexcept
    {
        return kind_ == json::kind::double_;
    }

    /** Returns true if this is a boolean

        This function returns `true` if
        @ref kind() equals `kind::boolean`.

        @par Complexity

        Constant.
    */
    bool
    is_bool() const noexcept
    {
        return kind_ == json::kind::boolean;
    }

    /** Returns true if this is a null

        This function returns `true` if
        @ref kind() equals `kind::null`.

        @par Complexity

        Constant.
    */
    bool
    is_null() const noexcept
    {
        return kind_ == json::kind::null;
    }

    /** Returns true if this is not an array or object

        This function returns `true` if
        @ref kind() is neither `kind::object` nor
        `kind::array`.

        @par Complexity

        Constant.
    */
    bool
    is_primitive() const noexcept
    {
       return
           kind_ != json::kind::object &&
           kind_ != json::kind::array;
    }

    /** Returns true if this is an array or object

        This function returns `true` if
        @ref kind() is either `kind::object` or
        `kind::array`.

        @par Complexity

        Constant.
    */
    bool
    is_structured() const noexcept
    {
       return
           kind_ == json::kind::object ||
           kind_ == json::kind::array;
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
    BOOST_JSON_DECL
    storage_ptr const&
    get_storage() const noexcept;

    /** Return a pointer to an object, or nullptr.

        If @ref kind() returns `kind::object`,
        returns a pointer to the object. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    object*
    if_object() noexcept
    {
        if(kind_ == json::kind::object)
            return &obj_;
        return nullptr;
    }

    /** Return a pointer to an object, or nullptr.

        If @ref kind() returns `kind::object`,
        returns a pointer to the object. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    object const*
    if_object() const noexcept
    {
        if(kind_ == json::kind::object)
            return &obj_;
        return nullptr;
    }

    /** Return a pointer to an array, or nullptr.

        If @ref kind() returns `kind::array`,
        returns a pointer to the array. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    array*
    if_array() noexcept
    {
        if(kind_ == json::kind::array)
            return &arr_;
        return nullptr;
    }

    /** Return a pointer to an array, or nullptr.

        If @ref kind() returns `kind::array`,
        returns a pointer to the array. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    array const*
    if_array() const noexcept
    {
        if(kind_ == json::kind::array)
            return &arr_;
        return nullptr;
    }

    /** Return a pointer to a string, or nullptr.

        If @ref kind() returns `kind::string`,
        returns a pointer to the string. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    string*
    if_string() noexcept
    {
        if(kind_ == json::kind::string)
            return &str_;
        return nullptr;
    }

    /** Return a pointer to a string, or nullptr.

        If @ref kind() returns `kind::string`,
        returns a pointer to the string. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    string const*
    if_string() const noexcept
    {
        if(kind_ == json::kind::string)
            return &str_;
        return nullptr;
    }

    std::int64_t*
    if_int64() noexcept
    {
        if(kind_ == json::kind::int64)
            return &sca_.i;
        return nullptr;
    }

    std::int64_t const*
    if_int64() const noexcept
    {
        if(kind_ == json::kind::int64)
            return &sca_.i;
        return nullptr;
    }

    std::uint64_t*
    if_uint64() noexcept
    {
        if(kind_ == json::kind::uint64)
            return &sca_.u;
        return nullptr;
    }

    std::uint64_t const*
    if_uint64() const noexcept
    {
        if(kind_ == json::kind::uint64)
            return &sca_.u;
        return nullptr;
    }

    double*
    if_double() noexcept
    {
        if(kind_ == json::kind::double_)
            return &sca_.d;
        return nullptr;
    }

    double const*
    if_double() const noexcept
    {
        if(kind_ == json::kind::double_)
            return &sca_.d;
        return nullptr;
    }

    /** Return a pointer to a bool, or nullptr.

        If @ref kind() returns `kind::boolean`,
        returns a pointer to the `bool`. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    bool*
    if_bool() noexcept
    {
        if(kind_ == json::kind::boolean)
            return &sca_.b;
        return nullptr;
    }

    /** Return a pointer to a bool, or nullptr.

        If @ref kind() returns `kind::boolean`,
        returns a pointer to the `bool`. Otherwise,
        returns `nullptr`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    bool const*
    if_bool() const noexcept
    {
        if(kind_ == json::kind::boolean)
            return &sca_.b;
        return nullptr;
    }

    //------------------------------------------------------

    /** Return a reference to the object, or throw an exception.

        If @ref kind() returns `kind::object`,
        returns a reference to the object. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not an object.
    */
    object&
    as_object()
    {
        if(kind_ != json::kind::object)
            BOOST_JSON_THROW(
                system_error(
                    error::not_object));
        return obj_;
    }

    /** Return a reference to the object, or throw an exception.

        If @ref kind() returns `kind::object`,
        returns a reference to the object. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not an object.
    */
    object const&
    as_object() const
    {
        if(kind_ != json::kind::object)
            BOOST_JSON_THROW(
                system_error(
                    error::not_object));
        return obj_;
    }

    /** Return a reference to the array, or throw an exception.

        If @ref kind() returns `kind::array`,
        returns a reference to the array. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not an array.
    */
    array&
    as_array()
    {
        if(kind_ != json::kind::array)
            BOOST_JSON_THROW(
                system_error(
                    error::not_array));
        return arr_;
    }

    /** Return a reference to the array, or throw an exception.

        If @ref kind() returns `kind::array`,
        returns a reference to the array. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not an array.
    */
    array const&
    as_array() const
    {
        if(kind_ != json::kind::array)
            BOOST_JSON_THROW(
                system_error(
                    error::not_array));
        return arr_;
    }

    /** Return a reference to the string, or throw an exception.

        If @ref kind() returns `kind::string`,
        returns a reference to the string. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not a string.
    */
    string&
    as_string()
    {
        if(kind_ != json::kind::string)
            BOOST_JSON_THROW(
                system_error(
                    error::not_string));
        return str_;
    }

    /** Return a reference to the string, or throw an exception.

        If @ref kind() returns `kind::string`,
        returns a reference to the string. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not a string.
    */
    string const&
    as_string() const
    {
        if(kind_ != json::kind::string)
            BOOST_JSON_THROW(
                system_error(
                    error::not_string));
        return str_;
    }

    std::int64_t&
    as_int64()
    {
        if(kind_ != json::kind::int64)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.i;
    }


    std::int64_t const&
    as_int64() const
    {
        if(kind_ != json::kind::int64)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.i;
    }

    std::uint64_t&
    as_uint64()
    {
        if(kind_ != json::kind::uint64)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.u;
    }


    std::uint64_t const&
    as_uint64() const
    {
        if(kind_ != json::kind::uint64)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.u;
    }

    double&
    as_double()
    {
        if(kind_ != json::kind::double_)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.d;
    }


    double const&
    as_double() const
    {
        if(kind_ != json::kind::double_)
            BOOST_JSON_THROW(
                system_error(
                    error::not_number));
        return sca_.d;
    }

    /** Return a reference to the bool, or throw an exception.

        If @ref kind() returns `kind::boolean`,
        returns a reference to the `bool`. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not a boolean.
    */
    bool&
    as_bool()
    {
        if(kind_ != json::kind::boolean)
            BOOST_JSON_THROW(
                system_error(
                    error::not_bool));
        return sca_.b;
    }

    /** Return a reference to the bool, or throw an exception.

        If @ref kind() returns `kind::boolean`,
        returns a reference to the `bool`. Otherwise,
        throws an exception.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.

        @throw system_error if `*this` is not a boolean.
    */
    bool const&
    as_bool() const
    {
        if(kind_ != json::kind::boolean)
            BOOST_JSON_THROW(
                system_error(
                    error::not_bool));
        return sca_.b;
    }

    //------------------------------------------------------

    /** Return a value as an object, without checking.

        This is a fast way to gain access to an object
        value when the kind is known.

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
        BOOST_JSON_ASSERT(is_object());
        return obj_;
    }

    /** Return a value as an object, without checking.

        This is a fast way to gain access to an object
        value when the kind is known.

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
        BOOST_JSON_ASSERT(is_object());
        return obj_;
    }

    /** Return a value as an array, without checking.

        This is a fast way to gain access to an array
        value when the kind is known.

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
        BOOST_JSON_ASSERT(is_array());
        return arr_;
    }

    /** Return a value as an array, without checking.

        This is a fast way to gain access to an array
        value when the kind is known.

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
        BOOST_JSON_ASSERT(is_array());
        return arr_;
    }

    //------------------------------------------------------

private:
    BOOST_JSON_DECL
    storage_ptr
    destroy() noexcept;
};

} // json
} // boost

#include <boost/json/impl/array.hpp>
#include <boost/json/impl/object.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/array.ipp>
#include <boost/json/impl/object.ipp>
#endif

// These must come after array and object
#include <boost/json/impl/value.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/value.ipp>
#endif

#endif
