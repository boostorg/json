// Copyright 2020-2022 Junekey Jeon
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.
//
// Some parts are copied from Dragonbox project.
//
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_JSON_DETAIL_DRAGONBOX_DRAGONBOX_COMMON_HPP
#define BOOST_JSON_DETAIL_DRAGONBOX_DRAGONBOX_COMMON_HPP

#include <boost/json/detail/charconv/detail/config.hpp>
#include <boost/json/detail/dragonbox/bit_layouts.hpp>
#include <boost/json/detail/dragonbox/emulated128.hpp>
#include <boost/core/bit.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <climits>

namespace boost {
namespace json {
namespace detail {

template <typename T>
struct physical_bits
{
    static constexpr std::size_t value = sizeof(T) * CHAR_BIT;
};

template <typename T>
struct value_bits
{
    static constexpr std::size_t value = std::numeric_limits<typename std::enable_if<std::is_unsigned<T>::value, T>::type>::digits;
};

#ifdef BOOST_NO_CXX17_INLINE_VARIABLES

template <typename T> constexpr std::size_t physical_bits<T>::value;
template <typename T> constexpr std::size_t value_bits<T>::value;

#endif

// A floating-point traits class defines ways to interpret a bit pattern of given size as an
// encoding of floating-point number. This is a default implementation of such a traits class,
// supporting ways to interpret 32-bits into a binary32-encoded floating-point number and to
// interpret 64-bits into a binary64-encoded floating-point number. Users might specialize this
// class to change the default behavior for certain types.
template <typename T>
struct default_float_traits
{
    // I don't know if there is a truly reliable way of detecting
    // IEEE-754 binary32/binary64 formats; I just did my best here.
    static_assert(std::numeric_limits<T>::is_iec559 && std::numeric_limits<T>::radix == 2 &&
                  (detail::physical_bits<T>::value == 32 || detail::physical_bits<T>::value == 64),
                    "default_ieee754_traits only works for 32-bits or 64-bits types "
                    "supporting binary32 or binary64 formats!");

    // The type that is being viewed.
    using type = T;

    // Refers to the format specification class.
    using format =
        typename std::conditional<detail::physical_bits<T>::value == 32, detail::ieee754_binary32, detail::ieee754_binary64>::type;

    // Defines an unsignedeger type that is large enough to carry a variable of type T.
    // Most of the operations will be done on this integer type.
    using carrier_uint = typename std::conditional<detail::physical_bits<T>::value == 32, std::uint32_t, std::uint64_t>::type;

    static_assert(sizeof(carrier_uint) == sizeof(T), "carrier_uint must be T");

    // Number of bits in the above unsignedeger type.
    static constexpr int carrier_bits = static_cast<int>(detail::physical_bits<carrier_uint>::value);

    // Convert from carrier_uint into the original type.
    // Depending on the floating-point encoding format, this operation might not be possible for
    // some specific bit patterns. However, the contract is that u always denotes a
    // valid bit pattern, so this function must be assumed to be noexcept.
    static T carrier_to_float(carrier_uint u) noexcept
    {
        T x;
        std::memcpy(&x, &u, sizeof(carrier_uint));
        return x;
    }

    // Same as above.
    static carrier_uint float_to_carrier(T x) noexcept
    {
        carrier_uint u;
        std::memcpy(&u, &x, sizeof(carrier_uint));
        return u;
    }

    // Extract exponent bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. This function does not do bias adjustment.
    static constexpr unsigned extract_exponent_bits(carrier_uint u) noexcept
    {
        return static_cast<unsigned>(u >> format::exponent_bits) & static_cast<unsigned>((1U << format::exponent_bits) - 1);
    }

    // Extract significand bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. The result does not contain the implicit bit.
    static constexpr carrier_uint extract_significand_bits(carrier_uint u) noexcept
    {
        return static_cast<carrier_uint>(u & static_cast<carrier_uint>((static_cast<carrier_uint>(1) << format::significand_bits) - 1));
    }

    // Remove the exponent bits and extract significand bits together with the sign bit.
    static constexpr carrier_uint remove_exponent_bits(carrier_uint u, unsigned exponent_bits) noexcept
    {
        return u ^ (static_cast<carrier_uint>(exponent_bits) << format::significand_bits);
    }

    // Shift the obtained signed significand bits to the left by 1 to remove the sign bit.
    static constexpr carrier_uint remove_sign_bit_and_shift(carrier_uint u) noexcept
    {
        return static_cast<carrier_uint>(static_cast<carrier_uint>(u) << 1);
    }

    // The actual value of exponent is obtained by adding this value to the extracted exponent bits
    static constexpr int exponent_bias = 1 - (1 << (carrier_bits - format::significand_bits - 2));

    // Obtain the actual value of the binary exponent from the extracted exponent bits.
    static constexpr int binary_exponent(unsigned exponent_bits) noexcept
    {
        return exponent_bits == 0 ? format::min_exponent : static_cast<int>(exponent_bits) + format::exponent_bias;
    }

    // Obtain the actual value of the binary exponent from the extracted significand bits and
    // exponent bits.
    static constexpr carrier_uint binary_significand(carrier_uint significand_bits, unsigned exponent_bits) noexcept
    {
        return exponent_bits == 0 ? significand_bits : (significand_bits | (static_cast<carrier_uint>(1) << format::significand_bits));
    }


    // Various boolean observer functions

    static constexpr bool is_nonzero(carrier_uint u) noexcept { return (u << 1) != 0; }

    static constexpr bool is_positive(carrier_uint u) noexcept
    {
        return u < static_cast<carrier_uint>(1) << (format::significand_bits + format::exponent_bits);
    }

    static constexpr bool is_negative(carrier_uint u) noexcept { return !is_positive(u); }

    static constexpr bool is_finite(unsigned exponent_bits) noexcept
    {
        //constexpr unsigned exponent_bits_all_set = (1u << format::exponent_bits) - 1;
        return exponent_bits != (1u << format::exponent_bits) - 1;
    }

    static constexpr bool has_all_zero_significand_bits(carrier_uint u) noexcept
    {
        return (u << 1) == 0;
    }

    static constexpr bool has_even_significand_bits(carrier_uint u) noexcept
    {
        return u % 2 == 0;
    }
};

// Convenient wrappers for floating-point traits classes.
// In order to reduce the argument passing overhead, these classes should be as simple as
// possible (e.g., no inheritance, no private non-static data member, etc.; this is an
// unfortunate fact about common ABI convention).

template <typename T, typename Traits = default_float_traits<T>>
struct float_bits;

template <typename T, typename Traits = default_float_traits<T>>
struct signed_significand_bits;

template <typename T, typename Traits>
struct float_bits
{
    using type = T;
    using traits_type = Traits;
    using carrier_uint = typename traits_type::carrier_uint;

    carrier_uint u;

    float_bits() = default;
    constexpr explicit float_bits(carrier_uint bit_pattern) noexcept : u{bit_pattern} {}
    constexpr explicit float_bits(T float_value) noexcept : u{traits_type::float_to_carrier(float_value)} {}

    constexpr T to_float() const noexcept { return traits_type::carrier_to_float(u); }

    // Extract exponent bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. This function does not do bias adjustment.
    constexpr unsigned extract_exponent_bits() const noexcept
    {
        return traits_type::extract_exponent_bits(u);
    }

    // Extract significand bits from a bit pattern.
    // The result must be aligned to the LSB so that there is no additional zero paddings
    // on the right. The result does not contain the implicit bit.
    constexpr carrier_uint extract_significand_bits() const noexcept
    {
        return traits_type::extract_significand_bits(u);
    }

    // Remove the exponent bits and extract significand bits together with the sign bit.
    constexpr signed_significand_bits<type, traits_type> remove_exponent_bits(unsigned exponent_bits) const noexcept
    {
        return signed_significand_bits<type, traits_type>(traits_type::remove_exponent_bits(u, exponent_bits));
    }

    // Obtain the actual value of the binary exponent from the extracted exponent bits.
    static constexpr int binary_exponent(unsigned exponent_bits) noexcept
    {
        return traits_type::binary_exponent(exponent_bits);
    }

    constexpr int binary_exponent() const noexcept
    {
        return binary_exponent(extract_exponent_bits());
    }

    // Obtain the actual value of the binary exponent from the extracted significand bits and
    // exponent bits.
    static constexpr carrier_uint binary_significand(carrier_uint significand_bits, unsigned exponent_bits) noexcept
    {
        return traits_type::binary_significand(significand_bits, exponent_bits);
    }

    constexpr carrier_uint binary_significand() const noexcept
    {
        return binary_significand(extract_significand_bits(), extract_exponent_bits());
    }

    constexpr bool is_nonzero() const noexcept { return traits_type::is_nonzero(u); }

    constexpr bool is_positive() const noexcept { return traits_type::is_positive(u); }

    constexpr bool is_negative() const noexcept { return traits_type::is_negative(u); }

    constexpr bool is_finite(unsigned exponent_bits) const noexcept { return traits_type::is_finite(exponent_bits); }

    constexpr bool is_finite() const noexcept { return traits_type::is_finite(extract_exponent_bits()); }

    constexpr bool has_even_significand_bits() const noexcept { return traits_type::has_even_significand_bits(u); }
};

template <typename T, typename Traits>
struct signed_significand_bits
{
    using type = T;
    using traits_type = Traits;
    using carrier_uint = typename traits_type::carrier_uint;

    carrier_uint u;

    signed_significand_bits() = default;
    constexpr explicit signed_significand_bits(carrier_uint bit_pattern) noexcept
        : u{bit_pattern} {}

    // Shift the obtained signed significand bits to the left by 1 to remove the sign bit.
    constexpr carrier_uint remove_sign_bit_and_shift() const noexcept
    {
        return traits_type::remove_sign_bit_and_shift(u);
    }

    constexpr bool is_positive() const noexcept { return traits_type::is_positive(u); }

    constexpr bool is_negative() const noexcept { return traits_type::is_negative(u); }

    constexpr bool has_all_zero_significand_bits() const noexcept
    {
        return traits_type::has_all_zero_significand_bits(u);
    }

    constexpr bool has_even_significand_bits() const noexcept
    {
        return traits_type::has_even_significand_bits(u);
    }
};

////////////////////////////////////////////////////////////////////////////////////////
// Some simple utilities for constexpr computation.
////////////////////////////////////////////////////////////////////////////////////////

template <class Int, class Int2>
BOOST_JSON_CXX14_CONSTEXPR Int compute_power(Int a, Int2 exp) noexcept
{
    BOOST_ASSERT(exp >= 0);

    Int res = 1;
    while (exp > 0)
    {
        if (exp % 2 != 0)
        {
            res *= a;
        }

        a *= a;
        exp >>= 1;
    }
    return res;
}

static constexpr std::uint64_t power_of_10[] = {
    UINT64_C(1), UINT64_C(10), UINT64_C(100), UINT64_C(1000), UINT64_C(10000),
    UINT64_C(100000), UINT64_C(1000000), UINT64_C(10000000), UINT64_C(100000000),
    UINT64_C(1000000000), UINT64_C(10000000000), UINT64_C(100000000000), UINT64_C(1000000000000),
    UINT64_C(10000000000000), UINT64_C(100000000000000), UINT64_C(1000000000000000),
    UINT64_C(10000000000000000), UINT64_C(100000000000000000), UINT64_C(1000000000000000000),
    UINT64_C(10000000000000000000)
};

static_assert(sizeof(power_of_10) == 20 * sizeof(std::uint64_t), "There should be the first 20 powers of 10");


template <unsigned a, typename UInt>
BOOST_JSON_CXX14_CONSTEXPR int count_factors(UInt n) noexcept
{
    int c = 0;

    while (n % a == 0)
    {
        n /= a;
        ++c;
    }
    return c;
}

////////////////////////////////////////////////////////////////////////////////////////
// Utilities for fast/constexpr log computation.
////////////////////////////////////////////////////////////////////////////////////////

namespace log  {
static_assert((-1 >> 1) == -1, "right-shift for signed integers must be arithmetic");

// Compute floor(e * c - s).
enum class multiply : std::uint32_t {};
enum class subtract : std::uint32_t {};
enum class shift : std::size_t {};
enum class min_exponent : std::int32_t {};
enum class max_exponent : std::int32_t {};

template <multiply m, subtract f, shift k, min_exponent e_min, max_exponent e_max>
constexpr int compute(int e) noexcept
{
    return static_cast<int>((std::int32_t(e) * std::int32_t(m) - std::int32_t(f)) >> std::size_t(k));
}

// For constexpr computation.
// Returns -1 when n = 0.
template <class UInt>
BOOST_JSON_CXX14_CONSTEXPR int floor_log2(UInt n) noexcept
{
    int count = -1;
    while (n != 0)
    {
        ++count;
        n >>= 1;
    }

    return count;
}

static constexpr int floor_log10_pow2_min_exponent = -2620;

static constexpr int floor_log10_pow2_max_exponent = 2620;

constexpr int floor_log10_pow2(int e) noexcept
{
    using namespace log;
    return compute<multiply(315653), subtract(0), shift(20),
                    min_exponent(floor_log10_pow2_min_exponent),
                    max_exponent(floor_log10_pow2_max_exponent)>(e);
}

static constexpr int floor_log2_pow10_min_exponent = -1233;

static constexpr int floor_log2_pow10_max_exponent = 1233;

constexpr int floor_log2_pow10(int e) noexcept
{
    using namespace log;
    return compute<multiply(1741647), subtract(0), shift(19),
                    min_exponent(floor_log2_pow10_min_exponent),
                    max_exponent(floor_log2_pow10_max_exponent)>(e);
}

static constexpr int floor_log10_pow2_minus_log10_4_over_3_min_exponent = -2985;

static constexpr int floor_log10_pow2_minus_log10_4_over_3_max_exponent = 2936;

constexpr int floor_log10_pow2_minus_log10_4_over_3(int e) noexcept
{
    using namespace log;
    return compute<multiply(631305), subtract(261663), shift(21),
                    min_exponent(floor_log10_pow2_minus_log10_4_over_3_min_exponent),
                    max_exponent(floor_log10_pow2_minus_log10_4_over_3_max_exponent)>(e);
}

static constexpr int floor_log5_pow2_min_exponent = -1831;

static constexpr int floor_log5_pow2_max_exponent = 1831;

constexpr int floor_log5_pow2(int e) noexcept
{
    using namespace log;
    return compute<multiply(225799), subtract(0), shift(19),
                    min_exponent(floor_log5_pow2_min_exponent),
                    max_exponent(floor_log5_pow2_max_exponent)>(e);
}

static constexpr int floor_log5_pow2_minus_log5_3_min_exponent = -3543;

static constexpr int floor_log5_pow2_minus_log5_3_max_exponent = 2427;

constexpr int floor_log5_pow2_minus_log5_3(int e) noexcept
{
    using namespace log;
    return compute<multiply(451597), subtract(715764), shift(20),
                    min_exponent(floor_log5_pow2_minus_log5_3_min_exponent),
                    max_exponent(floor_log5_pow2_minus_log5_3_max_exponent)>(e);
}
} // Namespace log

} // namespace detail
} // namespace json
} // namespace boost

#endif // BOOST_JSON_DETAIL_DRAGONBOX_DRAGONBOX_COMMON_HPP
