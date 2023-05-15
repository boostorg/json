// Copyright 2020-2023 Junekey Jeon
// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/to_chars.hpp>
#include <cstring>
#include <cstdio>
#include <cstdint>

namespace boost { namespace charconv { namespace detail { namespace to_chars_detail {

#ifdef BOOST_MSVC
# pragma warning(push)
# pragma warning(disable: 4127) // Conditional expression is constant (e.g. BOOST_IF_CONSTEXPR statements)
#endif

    // These "//"'s are to prevent clang-format to ruin this nice alignment.
    // Thanks to reddit user u/mcmcc:
    // https://www.reddit.com/r/cpp/comments/so3wx9/dragonbox_110_is_released_a_fast_floattostring/hw8z26r/?context=3
    static constexpr char radix_100_table[] = {
        '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', //
        '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', //
        '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', //
        '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', //
        '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', //
        '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', //
        '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', //
        '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', //
        '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', //
        '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', //
        '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', //
        '5', '5', '5', '6', '5', '7', '5', '8', '5', '9', //
        '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', //
        '6', '5', '6', '6', '6', '7', '6', '8', '6', '9', //
        '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', //
        '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', //
        '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', //
        '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', //
        '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', //
        '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'  //
    };
    static constexpr char radix_100_head_table[] = {
        '0', '.', '1', '.', '2', '.', '3', '.', '4', '.', //
        '5', '.', '6', '.', '7', '.', '8', '.', '9', '.', //
        '1', '.', '1', '.', '1', '.', '1', '.', '1', '.', //
        '1', '.', '1', '.', '1', '.', '1', '.', '1', '.', //
        '2', '.', '2', '.', '2', '.', '2', '.', '2', '.', //
        '2', '.', '2', '.', '2', '.', '2', '.', '2', '.', //
        '3', '.', '3', '.', '3', '.', '3', '.', '3', '.', //
        '3', '.', '3', '.', '3', '.', '3', '.', '3', '.', //
        '4', '.', '4', '.', '4', '.', '4', '.', '4', '.', //
        '4', '.', '4', '.', '4', '.', '4', '.', '4', '.', //
        '5', '.', '5', '.', '5', '.', '5', '.', '5', '.', //
        '5', '.', '5', '.', '5', '.', '5', '.', '5', '.', //
        '6', '.', '6', '.', '6', '.', '6', '.', '6', '.', //
        '6', '.', '6', '.', '6', '.', '6', '.', '6', '.', //
        '7', '.', '7', '.', '7', '.', '7', '.', '7', '.', //
        '7', '.', '7', '.', '7', '.', '7', '.', '7', '.', //
        '8', '.', '8', '.', '8', '.', '8', '.', '8', '.', //
        '8', '.', '8', '.', '8', '.', '8', '.', '8', '.', //
        '9', '.', '9', '.', '9', '.', '9', '.', '9', '.', //
        '9', '.', '9', '.', '9', '.', '9', '.', '9', '.'  //
    };

    static void print_1_digit(std::uint32_t n, char* buffer) noexcept
    {
        BOOST_IF_CONSTEXPR (('0' & 0xf) == 0)
        {
            *buffer = char('0' | n);
        }
        else
        {
            *buffer = char('0' + n);
        }
    }

    static void print_2_digits(std::uint32_t n, char* buffer) noexcept 
    {
        std::memcpy(buffer, radix_100_table + n * 2, 2);
    }

    // These digit generation routines are inspired by James Anhalt's itoa algorithm:
    // https://github.com/jeaiii/itoa
    // The main idea is for given n, find y such that floor(10^k * y / 2^32) = n holds,
    // where k is an appropriate integer depending on the length of n.
    // For example, if n = 1234567, we set k = 6. In this case, we have
    // floor(y / 2^32) = 1,
    // floor(10^2 * ((10^0 * y) mod 2^32) / 2^32) = 23,
    // floor(10^2 * ((10^2 * y) mod 2^32) / 2^32) = 45, and
    // floor(10^2 * ((10^4 * y) mod 2^32) / 2^32) = 67.
    // See https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/ for more explanation.

    BOOST_FORCEINLINE static void print_9_digits(std::uint32_t s32, int& exponent,
                                                char*& buffer) noexcept 
    {
        // -- IEEE-754 binary32
        // Since we do not cut trailing zeros in advance, s32 must be of 6~9 digits
        // unless the original input was subnormal.
        // In particular, when it is of 9 digits it shouldn't have any trailing zeros.
        // -- IEEE-754 binary64
        // In this case, s32 must be of 7~9 digits unless the input is subnormal,
        // and it shouldn't have any trailing zeros if it is of 9 digits.
        if (s32 >= 100000000)
        {
            // 9 digits.
            // 1441151882 = ceil(2^57 / 1'0000'0000) + 1
            auto prod = s32 * std::uint64_t(1441151882);
            prod >>= 25;
            std::memcpy(buffer, radix_100_head_table + std::uint32_t(prod >> 32) * 2, 2);

            prod = std::uint32_t(prod) * std::uint64_t(100);
            print_2_digits(std::uint32_t(prod >> 32), buffer + 2);
            prod = std::uint32_t(prod) * std::uint64_t(100);
            print_2_digits(std::uint32_t(prod >> 32), buffer + 4);
            prod = std::uint32_t(prod) * std::uint64_t(100);
            print_2_digits(std::uint32_t(prod >> 32), buffer + 6);
            prod = std::uint32_t(prod) * std::uint64_t(100);
            print_2_digits(std::uint32_t(prod >> 32), buffer + 8);

            exponent += 8;
            buffer += 10;
        }
        else if (s32 >= 1000000) 
        {
            // 7 or 8 digits.
            // 281474978 = ceil(2^48 / 100'0000) + 1
            auto prod = s32 * std::uint64_t(281474978);
            prod >>= 16;
            const auto head_digits = std::uint32_t(prod >> 32);
            // If s32 is of 8 digits, increase the exponent by 7.
            // Otherwise, increase it by 6.
            exponent += (6 + unsigned(head_digits >= 10));

            // Write the first digit and the decimal point.
            std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
            // This third character may be overwritten later but we don't care.
            buffer[2] = radix_100_table[head_digits * 2 + 1];

            // Remaining 6 digits are all zero?
            if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 1000000)) 
            {
                // The number of characters actually need to be written is:
                //   1, if only the first digit is nonzero, which means that either s32 is of 7
                //   digits or it is of 8 digits but the second digit is zero, or
                //   3, otherwise.
                // Note that buffer[2] is never '0' if s32 is of 7 digits, because the input is
                // never zero.
                buffer += (1 + (unsigned(head_digits >= 10) & unsigned(buffer[2] > '0')) * 2);
            }
            else 
            {
                // At least one of the remaining 6 digits are nonzero.
                // After this adjustment, now the first destination becomes buffer + 2.
                buffer += unsigned(head_digits >= 10);

                // Obtain the next two digits.
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 2);

                // Remaining 4 digits are all zero?
                if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 10000)) 
                {
                    buffer += (3 + unsigned(buffer[3] > '0'));
                }
                else 
                {
                    // At least one of the remaining 4 digits are nonzero.

                    // Obtain the next two digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 4);

                    // Remaining 2 digits are all zero?
                    if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 100))
                    {
                        buffer += (5 + unsigned(buffer[5] > '0'));
                    }
                    else 
                    {
                        // Obtain the last two digits.
                        prod = std::uint32_t(prod) * std::uint64_t(100);
                        print_2_digits(std::uint32_t(prod >> 32), buffer + 6);

                        buffer += (7 + unsigned(buffer[7] > '0'));
                    }
                }
            }
        }
        else if (s32 >= 10000)
        {
            // 5 or 6 digits.
            // 429497 = ceil(2^32 / 1'0000)
            auto prod = s32 * std::uint64_t(429497);
            const auto head_digits = std::uint32_t(prod >> 32);

            // If s32 is of 6 digits, increase the exponent by 5.
            // Otherwise, increase it by 4.
            exponent += (4 + unsigned(head_digits >= 10));

            // Write the first digit and the decimal point.
            std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
            // This third character may be overwritten later but we don't care.
            buffer[2] = radix_100_table[head_digits * 2 + 1];

            // Remaining 4 digits are all zero?
            if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 10000)) 
            {
                // The number of characters actually written is 1 or 3, similarly to the case of
                // 7 or 8 digits.
                buffer += (1 + (unsigned(head_digits >= 10) & unsigned(buffer[2] > '0')) * 2);
            }
            else 
            {
                // At least one of the remaining 4 digits are nonzero.
                // After this adjustment, now the first destination becomes buffer + 2.
                buffer += unsigned(head_digits >= 10);

                // Obtain the next two digits.
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 2);

                // Remaining 2 digits are all zero?
                if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 100))
                {
                    buffer += (3 + unsigned(buffer[3] > '0'));
                }
                else
                {
                    // Obtain the last two digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 4);

                    buffer += (5 + unsigned(buffer[5] > '0'));
                }
            }
        }
        else if (s32 >= 100)
        {
            // 3 or 4 digits.
            // 42949673 = ceil(2^32 / 100)
            auto prod = s32 * std::uint64_t(42949673);
            const auto head_digits = std::uint32_t(prod >> 32);

            // If s32 is of 4 digits, increase the exponent by 3.
            // Otherwise, increase it by 2.
            exponent += (2 + int(head_digits >= 10));

            // Write the first digit and the decimal point.
            std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
            // This third character may be overwritten later but we don't care.
            buffer[2] = radix_100_table[head_digits * 2 + 1];

            // Remaining 2 digits are all zero?
            if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 100))
            {
                // The number of characters actually written is 1 or 3, similarly to the case of
                // 7 or 8 digits.
                buffer += (1 + (unsigned(head_digits >= 10) & unsigned(buffer[2] > '0')) * 2);
            }
            else
            {
                // At least one of the remaining 2 digits are nonzero.
                // After this adjustment, now the first destination becomes buffer + 2.
                buffer += unsigned(head_digits >= 10);

                // Obtain the last two digits.
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 2);

                buffer += (3 + unsigned(buffer[3] > '0'));
            }
        }
        else
        {
            // 1 or 2 digits.
            // If s32 is of 2 digits, increase the exponent by 1.
            exponent += int(s32 >= 10);

            // Write the first digit and the decimal point.
            std::memcpy(buffer, radix_100_head_table + s32 * 2, 2);
            // This third character may be overwritten later but we don't care.
            buffer[2] = radix_100_table[s32 * 2 + 1];

            // The number of characters actually written is 1 or 3, similarly to the case of
            // 7 or 8 digits.
            buffer += (1 + (unsigned(s32 >= 10) & unsigned(buffer[2] > '0')) * 2);
        }
    }

    template <>
    char* to_chars<float, dragonbox_float_traits<float>>(std::uint32_t s32, int exponent, char* buffer, chars_format fmt) noexcept
    {
        // Print significand.
        print_9_digits(s32, exponent, buffer);

        // Print exponent and return
        if (exponent < 0)
        {
            std::memcpy(buffer, "e-", 2);
            buffer += 2;
            exponent = -exponent;
        }
        else if (exponent == 0)
        {
            if (fmt == chars_format::scientific)
            {
                std::memcpy(buffer, "e+00", 4);
                buffer += 4;
            }

            return buffer;
        }
        else 
        {
            std::memcpy(buffer, "e+", 2);
            buffer += 2;
        }

        print_2_digits(std::uint32_t(exponent), buffer);
        buffer += 2;

        return buffer;
    }

    template <>
    char* to_chars<double, dragonbox_float_traits<double>>(const std::uint64_t significand, int exponent, char* buffer, chars_format fmt) noexcept {
        // Print significand by decomposing it into a 9-digit block and a 8-digit block.
        std::uint32_t first_block;
        std::uint32_t second_block {};
        bool no_second_block;

        if (significand >= 100000000)
        {
            first_block = std::uint32_t(significand / 100000000);
            second_block = std::uint32_t(significand) - first_block * 100000000;
            exponent += 8;
            no_second_block = (second_block == 0);
        }
        else
        {
            first_block = std::uint32_t(significand);
            no_second_block = true;
        }

        if (no_second_block)
        {
            print_9_digits(first_block, exponent, buffer);
        }
        else
        {
            // We proceed similarly to print_9_digits(), but since we do not need to remove
            // trailing zeros, the procedure is a bit simpler.
            if (first_block >= 100000000)
            {
                // The input is of 17 digits, thus there should be no trailing zero at all.
                // The first block is of 9 digits.
                // 1441151882 = ceil(2^57 / 1'0000'0000) + 1
                auto prod = first_block * std::uint64_t(1441151882);
                prod >>= 25;
                std::memcpy(buffer, radix_100_head_table + std::uint32_t(prod >> 32) * 2, 2);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 2);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 4);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 6);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 8);

                // The second block is of 8 digits.
                // 281474978 = ceil(2^48 / 100'0000) + 1
                prod = second_block * std::uint64_t(281474978);
                prod >>= 16;
                prod += 1;
                print_2_digits(std::uint32_t(prod >> 32), buffer + 10);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 12);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 14);
                prod = std::uint32_t(prod) * std::uint64_t(100);
                print_2_digits(std::uint32_t(prod >> 32), buffer + 16);

                exponent += 8;
                buffer += 18;
            }
            else
            {
                if (first_block >= 1000000)
                {
                    // 7 or 8 digits.
                    // 281474978 = ceil(2^48 / 100'0000) + 1
                    auto prod = first_block * std::uint64_t(281474978);
                    prod >>= 16;
                    const auto head_digits = std::uint32_t(prod >> 32);

                    std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
                    buffer[2] = radix_100_table[head_digits * 2 + 1];

                    exponent += (6 + unsigned(head_digits >= 10));
                    buffer += unsigned(head_digits >= 10);

                    // Print remaining 6 digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 2);
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 4);
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 6);

                    buffer += 8;
                }
                else if (first_block >= 10000)
                {
                    // 5 or 6 digits.
                    // 429497 = ceil(2^32 / 1'0000)
                    auto prod = first_block * std::uint64_t(429497);
                    const auto head_digits = std::uint32_t(prod >> 32);

                    std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
                    buffer[2] = radix_100_table[head_digits * 2 + 1];

                    exponent += (4 + unsigned(head_digits >= 10));
                    buffer += unsigned(head_digits >= 10);

                    // Print remaining 4 digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 2);
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 4);

                    buffer += 6;
                }
                else if (first_block >= 100)
                {
                    // 3 or 4 digits.
                    // 42949673 = ceil(2^32 / 100)
                    auto prod = first_block * std::uint64_t(42949673);
                    const auto head_digits = std::uint32_t(prod >> 32);

                    std::memcpy(buffer, radix_100_head_table + head_digits * 2, 2);
                    buffer[2] = radix_100_table[head_digits * 2 + 1];

                    exponent += (2 + unsigned(head_digits >= 10));
                    buffer += unsigned(head_digits >= 10);

                    // Print remaining 2 digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 2);

                    buffer += 4;
                }
                else
                {
                    // 1 or 2 digits.
                    std::memcpy(buffer, radix_100_head_table + first_block * 2, 2);
                    buffer[2] = radix_100_table[first_block * 2 + 1];

                    exponent += unsigned(first_block >= 10);
                    buffer += (2 + unsigned(first_block >= 10));
                }

                // Next, print the second block.
                // The second block is of 8 digits, but we may have trailing zeros.
                // 281474978 = ceil(2^48 / 100'0000) + 1
                auto prod = second_block * std::uint64_t(281474978);
                prod >>= 16;
                prod += 1;
                print_2_digits(std::uint32_t(prod >> 32), buffer);

                // Remaining 6 digits are all zero?
                if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 1000000))
                {
                    buffer += (1 + unsigned(buffer[1] > '0'));
                }
                else
                {
                    // Obtain the next two digits.
                    prod = std::uint32_t(prod) * std::uint64_t(100);
                    print_2_digits(std::uint32_t(prod >> 32), buffer + 2);

                    // Remaining 4 digits are all zero?
                    if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 10000)) 
                    {
                        buffer += (3 + unsigned(buffer[3] > '0'));
                    }
                    else
                    {
                        // Obtain the next two digits.
                        prod = std::uint32_t(prod) * std::uint64_t(100);
                        print_2_digits(std::uint32_t(prod >> 32), buffer + 4);

                        // Remaining 2 digits are all zero?
                        if (std::uint32_t(prod) <= std::uint32_t((std::uint64_t(1) << 32) / 100)) 
                        {
                            buffer += (5 + unsigned(buffer[5] > '0'));
                        }
                        else 
                        {
                            // Obtain the last two digits.
                            prod = std::uint32_t(prod) * std::uint64_t(100);
                            print_2_digits(std::uint32_t(prod >> 32), buffer + 6);
                            buffer += (7 + unsigned(buffer[7] > '0'));
                        }
                    }
                }
            }
        }
        if (exponent < 0)
        {
            std::memcpy(buffer, "e-", 2);
            buffer += 2;
            exponent = -exponent;
        }
        else if (exponent == 0)
        {
            if (fmt == chars_format::scientific)
            {
                std::memcpy(buffer, "e+00", 4);
                buffer += 4;
            }

            return buffer;
        }
        else
        {
            std::memcpy(buffer, "e+", 2);
            buffer += 2;
        }

        if (exponent >= 100) 
        {
            // d1 = exponent / 10; d2 = exponent % 10;
            // 6554 = ceil(2^16 / 10)
            auto prod = std::uint32_t(exponent) * std::uint32_t(6554);
            auto d1 = prod >> 16;
            prod = std::uint16_t(prod) * std::uint32_t(5); // * 10
            auto d2 = prod >> 15;                          // >> 16
            print_2_digits(d1, buffer);
            print_1_digit(d2, buffer + 2);
            buffer += 3;
        }
        else
        {
            print_2_digits(static_cast<std::uint32_t>(exponent), buffer);
            buffer += 2;
        }

        return buffer;
    }

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

}}}} // Namespaces

boost::charconv::to_chars_result boost::charconv::to_chars(char* first, char* last, float value,
                                                           boost::charconv::chars_format fmt, int precision) noexcept
{
    return boost::charconv::detail::to_chars_float_impl(first, last, value, fmt, precision);
}

boost::charconv::to_chars_result boost::charconv::to_chars(char* first, char* last, double value,
                                                           boost::charconv::chars_format fmt, int precision) noexcept
{
    return boost::charconv::detail::to_chars_float_impl(first, last, value, fmt, precision);
}

#ifdef BOOST_CHARCONV_FULL_LONG_DOUBLE_TO_CHARS_IMPL
boost::charconv::to_chars_result boost::charconv::to_chars(char* first, char* last, long double value,
                                                           boost::charconv::chars_format fmt, int precision) noexcept
{
    return boost::charconv::detail::to_chars_float_impl(first, last, static_cast<double>(value), fmt, precision);
}
#else
boost::charconv::to_chars_result boost::charconv::to_chars( char* first, char* last, long double value ) noexcept
{
    std::snprintf( first, last - first, "%.*Lg", std::numeric_limits<long double>::max_digits10, value );
    return { first + std::strlen(first), 0 };
}
#endif
