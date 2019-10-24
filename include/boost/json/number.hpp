//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_NUMBER_HPP
#define BOOST_JSON_NUMBER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/ieee_decimal.hpp>
#include <boost/json/storage.hpp>
#include <boost/json/detail/string.hpp>
#include <boost/pilfer.hpp>
#include <cstdint>
#include <iosfwd>
#include <type_traits>

namespace boost {
namespace json {

namespace detail {

template<class T>
using remove_cv_t = typename
    std::remove_cv<T>::type;

} // detail

/** Metafunction returning `true` if a `T` can be assigned to @ref number
*/
#ifdef GENERATING_DOCUMENTATION
template<class T>
using is_number = __see_below__;
#else
template<class T>
using is_number =
    std::integral_constant<bool,
        std::is_arithmetic<T>::value &&
        ! std::is_same<detail::remove_cv_t<T>,
            bool>::value &&
        ! std::is_same<detail::remove_cv_t<T>,
            char>::value &&
        ! std::is_same<detail::remove_cv_t<T>,
            wchar_t>::value &&
        ! std::is_same<detail::remove_cv_t<T>,
            unsigned char>::value>;
#endif

/** The representation of parsed numbers.
*/
class number
{
    enum class kind : char
    {
        type_int64,
        type_uint64,
        type_double
    };

    storage_ptr sp_;

#ifndef GENERATING_DOCUMENTATION
    // The XSLT has problems with private anon unions
    union
    {
        double double_;
        long long int64_;
        unsigned long long uint64_;
    };
#endif

    kind kind_;

public:
    static std::size_t constexpr
        max_string_chars =
        #ifdef GENERATING_DOCUMENTATION
            27
        #else
             1 +    // '-'
            19 +    // unsigned 64-bit mantissa
             1 +    // 'e'
             1 +    // '-'
             5      // unsigned 16-bit exponent
        #endif
            ;

    //------------------------------------------------------

    BOOST_JSON_DECL
    ~number();

    BOOST_JSON_DECL
    number() noexcept;

    BOOST_JSON_DECL
    explicit
    number(storage_ptr sp) noexcept;

    BOOST_JSON_DECL
    number(pilfered<number> other) noexcept;

    BOOST_JSON_DECL
    number(number const& other);

    BOOST_JSON_DECL
    number(
        number const& other,
        storage_ptr sp);

    BOOST_JSON_DECL
    number(number&& other);

    BOOST_JSON_DECL
    number(
        number&& other,
        storage_ptr sp);

    BOOST_JSON_DECL
    number&
    operator=(number const& other);

    //------------------------------------------------------

    /** Construct a number from mantissa, exponent, and sign
    */
    BOOST_JSON_DECL
    explicit
    number(ieee_decimal const& dec) noexcept;

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            is_number<T>::value>::type
    #endif
    >
    number(T t) noexcept
    {
        assign_impl(t);
    }

    storage_ptr const&
    get_storage() const noexcept
    {
        return sp_;
    }

    /// Return true if the number is negative
    BOOST_JSON_DECL
    bool
    is_negative() const noexcept;

    /// Return true if the number is integral
    BOOST_JSON_DECL
    bool
    is_integral() const noexcept;

    /// Return true if the number can be represented with a signed 64-bit integer
    BOOST_JSON_DECL
    bool
    is_int64() const noexcept;

    /// Return true if the number can be represented with an unsigned 64-bit integer
    BOOST_JSON_DECL
    bool
    is_uint64() const noexcept;

    /** Return the number as a signed 64-bit integer

        The return value is undefined unless @ref is_int64 returns `true`.
    */
    BOOST_JSON_DECL
    std::int_least64_t
    get_int64() const noexcept;

    /** Return the number as an unsigned 64-bit integer

        The return value is undefined unless @ref is_uint64 returns `true`.
    */
    BOOST_JSON_DECL
    std::uint_least64_t
    get_uint64() const noexcept;

    /** Return the number as floating point
    */
    BOOST_JSON_DECL
    double
    get_double() const noexcept;

    /** Convert the number to a string.

        The destination must contain at least
        @ref max_string_chars bytes of valid storage.

        @return A string view representing the number as
        as string. Storage for the view comes from `dest`.
    */
    BOOST_JSON_DECL
    string_view
    print(
        char* buf,
        std::size_t buf_size) const noexcept;

private:
    template<class T>
    void
    assign_impl(T t,
        typename std::enable_if<
            std::is_unsigned<
            T>::value>::type* = 0) noexcept
    {
        kind_ = kind::type_uint64;
        uint64_ = t;
    }

    template<class T>
    void
    assign_impl(T t,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_integral<T>::value
            >::type* = 0) noexcept
    {
        kind_ = kind::type_int64;
        int64_ = t;
    }

    template<class T>
    void
    assign_impl(T t,
        typename std::enable_if<
            ! std::is_integral<
            T>::value>::type* = 0) noexcept
    {
        kind_ = kind::type_double;
        // VFALCO silence warnings
        // when `T` is `long double`.
        double_ = static_cast<double>(t);
    }

    BOOST_JSON_DECL
    friend
    std::ostream&
    operator<<(std::ostream& os, number const& n);

    BOOST_JSON_DECL
    friend
    bool
    operator==(
        number const& lhs,
        number const& rhs) noexcept;

    BOOST_JSON_DECL
    friend
    bool
    operator!=(
        number const& lhs,
        number const& rhs) noexcept;

    inline
    storage_ptr
    release_storage() noexcept
    {
        return std::move(sp_);
    }

    friend class value;
};

BOOST_JSON_DECL
bool
operator==(
    number const& lhs,
    number const& rhs) noexcept;

BOOST_JSON_DECL
bool
operator!=(
    number const& lhs,
    number const& rhs) noexcept;

BOOST_JSON_DECL
std::ostream&
operator<<(std::ostream& os, number const& n);

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/number.ipp>
#endif

#endif
