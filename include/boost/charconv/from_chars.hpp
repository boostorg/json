// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
#define BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/parser.hpp>
#include <boost/charconv/detail/compute_float32.hpp>
#include <boost/charconv/detail/compute_float64.hpp>
#include <boost/charconv/detail/bit_layouts.hpp>
#include <boost/charconv/config.hpp>
#include <boost/charconv/chars_format.hpp>
#include <cmath>

namespace boost { namespace charconv {

// integer overloads

BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, bool& value, int base = 10) noexcept = delete;
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, signed char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned char& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned short& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned int& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, unsigned long long& value, int base = 10) noexcept
{
    return detail::from_chars(first, last, value, base);
}

#ifdef BOOST_CHARCONV_HAS_INT128
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::int128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::int128_type, boost::uint128_type>(first, last, value, base);
}
BOOST_CHARCONV_GCC5_CONSTEXPR from_chars_result from_chars(const char* first, const char* last, boost::uint128_type& value, int base = 10) noexcept
{
    return detail::from_chars_integer_impl<boost::uint128_type, boost::uint128_type>(first, last, value, base);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// Floating Point
//----------------------------------------------------------------------------------------------------------------------

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4244) // Implict converion when BOOST_IF_CONSTEXPR expands to if
#elif defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace detail {

template <typename T>
from_chars_result from_chars_strtod(const char* first, const char* last, T& value) noexcept
{
    // For strto(f/d)
    // Floating point value corresponding to the contents of str on success.
    // If the converted value falls out of range of corresponding return type, range error occurs and HUGE_VAL, HUGE_VALF or HUGE_VALL is returned.
    // If no conversion can be performed, 0 is returned and *str_end is set to str.

    value = 0;
    char* str_end;
    T return_value {};
    BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        return_value = std::strtof(first, &str_end);
        if (return_value == HUGE_VALF)
        {
            return {last, ERANGE};
        }
    }
    else BOOST_IF_CONSTEXPR (std::is_same<T, double>::value)
    {
        return_value = std::strtod(first, &str_end);
        if (return_value == HUGE_VAL)
        {
            return {last, ERANGE};
        }
    }
    else
    {
        return_value = std::strtold(first, &str_end);
        if (return_value == HUGE_VALL)
        {
            return {last, ERANGE};
        }
    }

    // Since this is a fallback routine we are safe to check for 0
    if (return_value == 0 && str_end == last)
    {
        return {first, EINVAL};
    }

    value = return_value;
    return {str_end, 0};
}

template <typename T>
from_chars_result from_chars_float_impl(const char* first, const char* last, T& value, chars_format fmt) noexcept
{
    bool sign {};
    std::uint64_t significand {};
    std::int64_t  exponent {};

    auto r = boost::charconv::detail::parser(first, last, sign, significand, exponent, fmt);
    if (r.ec != 0)
    {
        return r;
    }
    else if (significand == 0)
    {
        value = sign ? static_cast<T>(-0.0L) : static_cast<T>(0.0L);
        return r;
    }

    bool success {};
    T return_val {};
    BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
    {
        return_val = compute_float32(exponent, significand, sign, success);
    }
    else
    {
        return_val = compute_float64(exponent, significand, sign, success);
    }

    if (!success)
    {
        if (significand == 1 && exponent == 0)
        {
            value = 1;
            r.ptr = last;
            r.ec = 0;
        }
        else
        {
            BOOST_IF_CONSTEXPR (std::is_same<T, float>::value)
            {
                if (return_val == HUGE_VALF || return_val == -HUGE_VALF)
                {
                    value = return_val;
                    r.ec = ERANGE;
                }
                else if (exponent < -46)
                {
                    value = sign ? -0.0F : 0.0;
                    r.ec = ERANGE;
                }
                else
                {
                    r = from_chars_strtod(first, last, value);
                }
            }
            else BOOST_IF_CONSTEXPR (std::is_same<T, double>::value)
            {
                if (return_val == HUGE_VAL || return_val == -HUGE_VAL)
                {
                    value = return_val;
                    r.ec = ERANGE;
                }
                else if (exponent < -325)
                {
                    value = sign ? -0.0 : 0.0;
                    r.ec = ERANGE;
                }
                else
                {
                    r = from_chars_strtod(first, last, value);
                }
            }
            else BOOST_IF_CONSTEXPR (std::is_same<T, long double>::value)
            {
                if (return_val == HUGE_VALL || return_val == -HUGE_VALL)
                {
                    value = return_val;
                    r.ec = ERANGE;
                }
                #if BOOST_CHARCONV_LDBL_BITS == 64
                else if (exponent < -325)
                #else // 80 or 128 bit long doubles have same range
                else if (exponent < -4965)
                #endif
                {
                    value = sign ? -0.0L : 0.0L;
                    r.ec = ERANGE;
                }

                else
                {
                    r = from_chars_strtod(first, last, value);
                }
            }
        }
    }
    else
    {
        value = return_val;
    }

    return r;
}

} // Namespace detail

#ifdef BOOST_MSVC
# pragma warning(pop)
#elif defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

// Only 64 bit long double overloads are fully implemented
#if BOOST_CHARCONV_LDBL_BITS == 64 || defined(BOOST_MSVC)
#define BOOST_CHARCONV_FULL_LONG_DOUBLE_IMPL
#endif

BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, float& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, double& value, chars_format fmt = chars_format::general) noexcept;
BOOST_CHARCONV_DECL from_chars_result from_chars(const char* first, const char* last, long double& value, chars_format fmt = chars_format::general) noexcept;

} // namespace charconv
} // namespace boost

#endif // #ifndef BOOST_CHARCONV_FROM_CHARS_HPP_INCLUDED
