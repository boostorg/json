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
#include <boost/json/number.hpp>
#include <boost/json/object.hpp>
#include <boost/json/storage.hpp>
#include <boost/json/string.hpp>
#include <boost/json/detail/is_specialized.hpp>
#include <boost/json/detail/value.hpp>
#include <boost/type_traits/make_void.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iosfwd>
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
*/
class value
{
    struct undo;
    struct init_iter;
    friend class value_test;

#ifndef GENERATING_DOCUMENTATION
    // XSL scripts have trouble with private anon unions
    struct native
    {
        union
        {
            number  num_;
            bool    bool_;
        };
        storage_ptr sp_;
    };

    union
    {
        object    obj_;
        array     arr_;
        string    str_;
        native    nat_;
    };
#endif

    json::kind kind_;

public:
    /// Destroy a value and all of its contents
    BOOST_JSON_DECL
    ~value();

    /** Construct a null value

        The container and all of its contents will use the
        default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    value() noexcept;

    /** Construct a null value

        The container and all of its contents will use the
        specified storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    explicit
    value(storage_ptr sp) noexcept;

    /** Construct a value of the specified kind

        The container and all of its contents will use the
        specified storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param k The kind of JSON value.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    value(
        json::kind k,
        storage_ptr sp = default_storage()) noexcept;

    /** Copy constructor

        The container and all of its contents will use the
        default storage.

        @par Complexity

        Linear in the size of `other`.

        @param other The value to copy.
    */
    BOOST_JSON_DECL
    value(value const& other);

    /** Copy constructor

        The container and all of its contents will use the
        specified storage object.

        @par Complexity

        Linear in the size of `other`.

        @param other The value to copy.

        @param sp A pointer to the @ref storage to use. The
        container will acquire shared ownership of the pointer.
    */
    BOOST_JSON_DECL
    value(
        value const& other,
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
    value(pilfered<value> other) noexcept;

    /** Move constructor

        Constructs the container with the contents of `other`
        using move semantics. Ownership of the underlying
        memory is transferred.
        The container acquires shared ownership of the
        @ref storage used by `other`.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The container to move
    */
    BOOST_JSON_DECL
    value(value&& other) noexcept;

    /** Move constructor

        Using `*sp` as the @ref storage for the new container,
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

        @param other The container to move

        @param sp A pointer to the @ref storage to use. The
        container array will acquire shared ownership of the pointer.
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

        @param other The container to assign from
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

        @param other The container to copy
    */
    BOOST_JSON_DECL
    value& operator=(value const& other);

    //------------------------------------------------------
    //
    // Conversion
    //
    //------------------------------------------------------

    /** Construct an object
    */
    BOOST_JSON_DECL
    value(object obj) noexcept;

    /** Construct an object
    */
    BOOST_JSON_DECL
    value(object obj, storage_ptr sp);

    /** Construct an array
    */
    BOOST_JSON_DECL
    value(array arr) noexcept;

    /** Construct an array
    */
    BOOST_JSON_DECL
    value(array arr, storage_ptr sp);

    /** Construct a string
    */
    BOOST_JSON_DECL
    value(string str) noexcept;

    /** Construct a string
    */
    BOOST_JSON_DECL
    value(string str, storage_ptr sp);

    /** Construct a number
    */
    BOOST_JSON_DECL
    value(number num) noexcept;

    /** Construct a number
    */
    BOOST_JSON_DECL
    value(number num, storage_ptr sp);

    /** Construct an object or array
    */
    BOOST_JSON_DECL
    value(
        std::initializer_list<value> init);

    /** Construct an object or array
    */
    BOOST_JSON_DECL
    value(
        std::initializer_list<value> init,
        storage_ptr sp);

    /** Assign an object
    */
    BOOST_JSON_DECL
    value&
    operator=(object obj);

    /** Assign an array
    */
    BOOST_JSON_DECL
    value&
    operator=(array arr);

    /** Assign a string
    */
    BOOST_JSON_DECL
    value&
    operator=(string str);

    /** Assign a number
    */
    BOOST_JSON_DECL
    value&
    operator=(number num);

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Reset the json to the specified type.

        This changes the value to hold a value of the
        specified type. Any previous contents are cleared.

        @param k The kind to set. If the new kind is an
        object, array, or string the resulting value will be
        empty. Otherwise, the value will be in an undefined,
        valid state.
    */
    value&
    reset(json::kind k = json::kind::null) noexcept
    {
        value(k, get_storage()).swap(*this);
        return *this;
    }

    /** Set the value to an empty object, and return it.

        This calls `reset(json::kind::object)` and returns
        `as_object()`. The previous contents of the value
        are destroyed.
    */
    object&
    emplace_object() noexcept
    {
        reset(json::kind::object);
        return obj_;
    }

    /** Set the value to an empty array, and return it.

        This calls `reset(json::kind::array)` and returns
        `as_array()`. The previous contents of the value
        are destroyed.
    */
    array&
    emplace_array() noexcept
    {
        reset(json::kind::array);
        return arr_;
    }

    /** Set the value to an empty string, and return it.

        This calls `reset(json::kind::string)` and returns
        `as_string()`. The previous contents of the value
        are destroyed.
    */
    string&
    emplace_string() noexcept
    {
        reset(json::kind::string);
        return str_;
    }

    /** Set the value to an uninitialized number, and return it.

        This calls `reset(json::kind::number)` and returns
        `as_number()`. The previous contents of the value
        are destroyed.
    */
    number&
    emplace_number() noexcept
    {
        reset(json::kind::number);
        return nat_.num_;
    }

    /** Set the value to an uninitialized boolean, and return it.

        This calls `reset(json::kind::boolean)` and returns
        `as_bool()`. The previous contents of the value
        are destroyed.
    */
    bool&
    emplace_bool() noexcept
    {
        reset(json::kind::boolean);
        return nat_.bool_;
    }

    /// Set the value to a null
    void
    emplace_null() noexcept
    {
        reset(json::kind::null);
    }

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
        storage_ptr sp = default_storage())
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
    //
    // Observers
    //
    //------------------------------------------------------

    /// Returns the kind of this JSON value
    json::kind
    kind() const noexcept
    {
        return kind_;
    }

    /// Returns `true` if this is an object
    bool
    is_object() const noexcept
    {
        return kind_ == json::kind::object;
    }

    /// Returns `true` if this is an array
    bool
    is_array() const noexcept
    {
        return kind_ == json::kind::array;
    }

    /// Returns `true` if this is a string
    bool
    is_string() const noexcept
    {
        return kind_ == json::kind::string;
    }

    /// Returns `true` if this is a number
    bool
    is_number() const noexcept
    {
        return kind_ == json::kind::number;
    }

    bool
    is_bool() const noexcept
    {
        return kind_ == json::kind::boolean;
    }

    bool
    is_null() const noexcept
    {
        return kind_ == json::kind::null;
    }

    //---

    /// Returns `true` if this is not an array or object
    bool
    is_primitive() const noexcept
    {
        switch(kind_)
        {
        case json::kind::object:
        case json::kind::array:
            return false;
        default:
            return true;
        }
    }

    /// Returns `true` if this is an array or object
    bool
    is_structured() const noexcept
    {
        return ! is_primitive();
    }

    /// Returns `true` if this is a number representable as a `std::int64_t`
    bool
    is_int64() const noexcept
    {
        return
            kind_ == json::kind::number &&
            nat_.num_.is_int64();
    }

    /// Returns `true` if this is a number representable as a `std::uint64_t`
    bool
    is_uint64() const noexcept
    {
        return
            kind_ == json::kind::number &&
            nat_.num_.is_uint64();
    }

    /** Returns `true` if this is a number representable as a `double`

        The return value will always be the same as the
        value returned from @ref is_number.
    */
    bool
    is_double() const noexcept
    {
        return kind_ == json::kind::number;
    }

    /** Returns `true` if this is an array with just a key and value
    */
    BOOST_JSON_DECL
    bool
    is_key_value_pair() const noexcept;

    /** Returns `true` if the init list consists only of key-value pairs
    */
    static
    BOOST_JSON_DECL
    bool
    maybe_object(
        std::initializer_list<value> init) noexcept;

    //------------------------------------------------------
    //
    // Accessors
    //
    //------------------------------------------------------

    BOOST_JSON_DECL
    storage_ptr const&
    get_storage() const noexcept;

    object*
    if_object() noexcept
    {
        if(kind_ == json::kind::object)
            return &obj_;
        return nullptr;
    }

    object const*
    if_object() const noexcept
    {
        if(kind_ == json::kind::object)
            return &obj_;
        return nullptr;
    }

    array*
    if_array() noexcept
    {
        if(kind_ == json::kind::array)
            return &arr_;
        return nullptr;
    }

    array const*
    if_array() const noexcept
    {
        if(kind_ == json::kind::array)
            return &arr_;
        return nullptr;
    }

    string*
    if_string() noexcept
    {
        if(kind_ == json::kind::string)
            return &str_;
        return nullptr;
    }

    string const*
    if_string() const noexcept
    {
        if(kind_ == json::kind::string)
            return &str_;
        return nullptr;
    }

    number*
    if_number() noexcept
    {
        if(kind_ == json::kind::number)
            return &nat_.num_;
        return nullptr;
    }

    number const*
    if_number() const noexcept
    {
        if(kind_ == json::kind::number)
            return &nat_.num_;
        return nullptr;
    }

    bool*
    if_bool() noexcept
    {
        if(kind_ == json::kind::boolean)
            return &nat_.bool_;
        return nullptr;
    }

    bool const*
    if_bool() const noexcept
    {
        if(kind_ == json::kind::boolean)
            return &nat_.bool_;
        return nullptr;
    }

    //------------------------------------------------------

    object&
    as_object() noexcept
    {
        if(kind_ != json::kind::object)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_object));
        return obj_;
    }

    object const&
    as_object() const
    {
        if(kind_ != json::kind::object)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_object));
        return obj_;
    }

    array&
    as_array()
    {
        if(kind_ != json::kind::array)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_array));
        return arr_;
    }

    array const&
    as_array() const
    {
        if(kind_ != json::kind::array)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_array));
        return arr_;
    }

    string&
    as_string()
    {
        if(kind_ != json::kind::string)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_string));
        return str_;
    }

    string const&
    as_string() const
    {
        if(kind_ != json::kind::string)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_string));
        return str_;
    }

    number&
    as_number()
    {
        if(kind_ != json::kind::number)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_number));
        return nat_.num_;
    }

    number const&
    as_number() const
    {
        if(kind_ != json::kind::number)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_number));
        return nat_.num_;
    }

    bool&
    as_bool() noexcept
    {
        if(kind_ != json::kind::boolean)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_bool));
        return nat_.bool_;
    }

    bool const&
    as_bool() const noexcept
    {
        if(kind_ != json::kind::boolean)
            BOOST_THROW_EXCEPTION(
                system_error(
                    error::not_bool));
        return nat_.bool_;
    }

    //------------------------------------------------------

    std::int64_t
    get_int64() const noexcept
    {
        BOOST_ASSERT(is_int64());
        return nat_.num_.get_int64();
    }

    std::uint64_t
    get_uint64() const noexcept
    {
        BOOST_ASSERT(is_uint64());
        return nat_.num_.get_uint64();
    }

    double
    get_double() const noexcept
    {
        BOOST_ASSERT(is_double());
        return nat_.num_.get_double();
    }

    //------------------------------------------------------

private:
    BOOST_JSON_DECL
    void
    construct(
        json::kind, storage_ptr) noexcept;
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
