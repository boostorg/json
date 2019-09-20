//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_NUMBER_HPP
#define BOOST_JSON_NUMBER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <iosfwd>
#include <type_traits>

namespace boost {
namespace json {

/** The representation of parsed numbers.
*/
class number
{
    struct base10_ieee
    {
        unsigned
        long long   mant;
        short       exp;
        bool        sign;
    };

    enum kind
    {
        type_int64,
        type_uint64,
        type_double,
        type_ieee
    };

    union
    {
        unsigned
        long long   uint64_;
        long long   int64_;
        double      double_;
        base10_ieee ieee_;
    };

    kind k_;

public:
    static std::size_t constexpr
        max_string_chars =
            1 +     // '-'
            19 +    // unsigned 64-bit mantissa
            1 +     // 'e'
            1 +     // '-'
            5;      // unsigned 16-bit exponent

    using mantissa_type =
        unsigned long long;

    using exponent_type =
        short;

    number() = default;
    number(number const&) = default;
    number& operator=(
        number const&) = default;

    /** Construct a number from mantissa, exponent, and sign
    */
    BOOST_JSON_DECL
    number(
        mantissa_type mant,
        exponent_type exp,
        bool sign) noexcept;

    /// Construct a number from a signed integer
    BOOST_JSON_DECL
    number(short v) noexcept;

    /// Construct a number from a signed integer
    BOOST_JSON_DECL
    number(int v) noexcept;

    /// Construct a number from a signed integer
    BOOST_JSON_DECL
    number(long v) noexcept;

    /// Construct a number from a signed integer
    BOOST_JSON_DECL
    number(long long v) noexcept;

    /// Construct a number from an unsigned integer
    BOOST_JSON_DECL
    number(unsigned short v) noexcept;

    /// Construct a number from an unsigned integer
    BOOST_JSON_DECL
    number(unsigned int v) noexcept;

    /// Construct a number from an unsigned integer
    BOOST_JSON_DECL
    number(unsigned long v) noexcept;

    /// Construct a number from an unsigned integer
    BOOST_JSON_DECL
    number(unsigned long long v) noexcept;

    /// Construct a number from a floating point value
    BOOST_JSON_DECL
    number(float v) noexcept;

    /// Construct a number from a floating point value
    BOOST_JSON_DECL
    number(double v) noexcept;

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
    struct pow10;

    BOOST_JSON_DECL
    void
    assign_signed(
        long long i) noexcept;

    BOOST_JSON_DECL
    void
    assign_unsigned(
        unsigned long long i) noexcept;

    BOOST_JSON_DECL
    void
    assign_double(double f) noexcept;

    friend
    std::ostream&
    operator<<(std::ostream& os, number const& n);

    friend
    bool
    operator==(
        number const& lhs,
        number const& rhs) noexcept;

    friend
    bool
    operator!=(
        number const& lhs,
        number const& rhs) noexcept;

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
