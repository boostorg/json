// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_PARSER_HPP
#define BOOST_CHARCONV_DETAIL_PARSER_HPP

#include <boost/charconv/detail/config.hpp>
#include <boost/charconv/detail/from_chars_result.hpp>
#include <boost/charconv/detail/from_chars_integer_impl.hpp>
#include <boost/charconv/detail/integer_search_trees.hpp>
#include <boost/charconv/limits.hpp>
#include <boost/charconv/chars_format.hpp>
#include <type_traits>
#include <limits>
#include <cerrno>
#include <cstdint>
#include <cstring>

#if defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace boost { namespace charconv { namespace detail {

template <typename Unsigned_Integer, typename Integer>
inline from_chars_result parser(const char* first, const char* last, bool& sign, Unsigned_Integer& significand, Integer& exponent, chars_format fmt = chars_format::general) noexcept
{
    if (first > last)
    {
        return {first, EINVAL};
    }

    auto next = first;
    bool all_zeros = true;

    // First extract the sign
    if (*next == '-')
    {
        sign = true;
        ++next;
    }
    else if (*next == '+')
    {
        return {next, EINVAL};
    }
    else
    {
        sign = false;
    }

    // Ignore leading zeros (e.g. 00005 or -002.3e+5)
    while (*next == '0' && next != last)
    {
        ++next;
    }

    // If the number is 0 we can abort now
    char exp_char;
    char capital_exp_char;
    if (fmt != chars_format::hex)
    {
        exp_char = 'e';
        capital_exp_char = 'E';
    }
    else
    {
        exp_char = 'p';
        capital_exp_char = 'P';
    }

    if (next == last || *next == exp_char || *next == -capital_exp_char)
    {
        significand = 0;
        exponent = 0;
        return {next, 0};
    }

    // Next we get the significand
    constexpr std::size_t significand_buffer_size = limits<Unsigned_Integer>::max_chars10 - 1; // Base 10 or 16
    char significand_buffer[significand_buffer_size] {};
    std::size_t i = 0;
    std::size_t dot_position = 0;
    Integer extra_zeros = 0;
    Integer leading_zero_powers = 0;

    while (*next != '.' && *next != exp_char && *next != capital_exp_char && next != last && i < significand_buffer_size)
    {
        all_zeros = false;
        significand_buffer[i] = *next;
        ++next;
        ++i;
    }

    bool fractional = false;
    if (next == last)
    {
        // if fmt is chars_format::scientific the e is required
        if (fmt == chars_format::scientific)
        {
            return {first, EINVAL};
        }
        
        exponent = 0;
        std::size_t offset = i;

        from_chars_result r {};
        if (fmt == chars_format::hex)
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
        }
        else
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand);
        }
        switch (r.ec)
        {
            case EINVAL:
                return {first, EINVAL};
            case ERANGE:
                return {next, ERANGE};
            default:
                return {next, 0};
        }
    }
    else if (*next == '.')
    {
        ++next;
        fractional = true;
        dot_position = i;

        // Process the fractional part if we have it
        //
        // if fmt is chars_format::scientific the e is required
        // if fmt is chars_format::fixed and not scientific the e is disallowed
        // if fmt is chars_format::general (which is scientific and fixed) the e is optional

        // If we have the value 0.00001 we can continue to chop zeros and adjust the exponent
        // so that we get the useful parts of the fraction
        if (all_zeros)
        {
            while (*next == '0' && next != last)
            {
                ++next;
                --leading_zero_powers;
            }

            if (next == last)
            {
                return {last, 0};
            }
        }

        while (*next != exp_char && *next != capital_exp_char && next != last && i < significand_buffer_size)
        {
            significand_buffer[i] = *next;
            ++next;
            ++i;        
        }
    }
    
    if (i == significand_buffer_size)
    {
        // We can not process any more significant figures into the significand so skip to the end
        // or the exponent part and capture the additional orders of magnitude for the exponent
        bool found_dot = false;
        while (*next != exp_char && *next != capital_exp_char && next != last)
        {
            ++next;
            if (!fractional && !found_dot)
            {
                ++extra_zeros;
            }
            if (*next == '.')
            {
                found_dot = true;
            }
        }
    }

    if (next == last)
    {
        if (fmt == chars_format::scientific)
        {
            return {first, EINVAL};
        }
        if (dot_position != 0 || fractional)
        {
            exponent = static_cast<Integer>(dot_position) - i + extra_zeros + leading_zero_powers;
        }
        else
        {
            exponent = extra_zeros + leading_zero_powers;
        }
        std::size_t offset = i;
        
        from_chars_result r {};
        if (fmt == chars_format::hex)
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
        }
        else
        {
            r = from_chars(significand_buffer, significand_buffer + offset, significand);
        }
        switch (r.ec)
        {
            case EINVAL:
                return {first, EINVAL};
            case ERANGE:
                return {next, ERANGE};
            default:
                return {next, 0};
        }
    }
    else if (*next == exp_char || *next == capital_exp_char)
    {
        // Would be a number without a significand e.g. e+03
        if (next == first)
        {
            return {next, EINVAL};
        }

        ++next;
        if (fmt == chars_format::fixed)
        {
            return {first, EINVAL};
        }

        exponent = i - 1;
        std::size_t offset = i;
        bool round = false;
        // If more digits are present than representable in the significand of the target type
        // we set the maximum
        if (offset > significand_buffer_size)
        {
            offset = significand_buffer_size - 1;
            i = significand_buffer_size;
            if (significand_buffer[offset] == '5' ||
                significand_buffer[offset] == '6' ||
                significand_buffer[offset] == '7' ||
                significand_buffer[offset] == '8' ||
                significand_buffer[offset] == '9')
            {
                round = true;
            }
        }

        from_chars_result r {};

        // If the significand is 0 from chars will return EINVAL because there is nothing in the buffer,
        // but it is a valid value. We need to continue parsing to get the correct value of ptr even
        // though we know we could bail now.
        //
        // See GitHub issue #29: https://github.com/cppalliance/charconv/issues/29
        if (offset != 0)
        {
            if (fmt == chars_format::hex)
            {
                r = from_chars(significand_buffer, significand_buffer + offset, significand, 16);
            } else
            {
                r = from_chars(significand_buffer, significand_buffer + offset, significand);
            }
            switch (r.ec)
            {
                case EINVAL:
                    return {first, EINVAL};
                case ERANGE:
                    return {next, ERANGE};
            }

            if (round)
            {
                significand += 1;
            }
        }
    }

    // Finally we get the exponent
    constexpr std::size_t exponent_buffer_size = 6; // Float128 min exp is −16382
    char exponent_buffer[exponent_buffer_size] {};
    Integer significand_digits = i;
    i = 0;

    // Get the sign first
    if (*next == '-')
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }
    else if (*next == '+')
    {
        ++next;
    }

    // Next strip any leading zeros
    while (*next == '0')
    {
        ++next;
    }

    // Process the significant values
    while (next != last && i < exponent_buffer_size)
    {
        exponent_buffer[i] = *next;
        ++next;
        ++i;
    }

    // If the exponent can't fit in the buffer the number is not representable
    if (next != last && i == exponent_buffer_size)
    {
        return {next, ERANGE};
    }

    // If the exponent was e+00 or e-00
    if (i == 0 || (i == 1 && exponent_buffer[0] == '-'))
    {
        if (fractional)
        {
            exponent = static_cast<Integer>(dot_position) - significand_digits;
        }
        else
        {
            exponent = extra_zeros;
        }

        return {next, 0};
    }

    const auto r = from_chars(exponent_buffer, exponent_buffer + i, exponent);

    exponent += leading_zero_powers;

    switch (r.ec)
    {
        case EINVAL:
            return {first, EINVAL};
        case ERANGE:
            return {next, ERANGE};
        default:
            if (fractional)
            {
                // Need to take the offset from 1.xxx because compute_floatXXX assumes the significand is an integer
                // so the exponent is off by the number of digits in the significand - 1
                if (fmt == chars_format::hex)
                {
                    // In hex the number of digits parsed is possibly less than the number of digits in base10
                    exponent -= num_digits(significand) - dot_position;
                }
                else
                {
                    exponent -= significand_digits - dot_position;
                }
            }
            else
            {
                exponent += extra_zeros;
            }
            return {next, 0};
    }
}

}}} // Namespaces

#if defined(__GNUC__) && __GNUC__ < 5 && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

#endif // BOOST_CHARCONV_DETAIL_PARSER_HPP
