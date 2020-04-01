//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_BASIC_PARSER_IMPL_HPP
#define BOOST_JSON_BASIC_PARSER_IMPL_HPP

#include <boost/json/config.hpp>
#include <boost/json/basic_parser.hpp>
#include <boost/json/error.hpp>
#include <boost/json/detail/buffer.hpp>
#include <boost/json/detail/sse2.hpp>
#include <cmath>
#include <cstring>

/*  This file must be manually included to get the
    function template definitions for basic_parser.
*/

/*  Reference:

    https://www.json.org/

    RFC 7159: The JavaScript Object Notation (JSON) Data Interchange Format
    https://tools.ietf.org/html/rfc7159

    https://ampl.com/netlib/fp/dtoa.c
*/

#ifndef BOOST_JSON_DOCS

namespace boost {
namespace json {

enum class basic_parser::state : char
{
    ele1, ele2, ele3,
    nul1, nul2, nul3,
    tru1, tru2, tru3,
    fal1, fal2, fal3, fal4,
    str1, str2, str3, str4,
    str5, str6, str7,
    sur1, sur2, sur3, sur4,
    sur5, sur6,
    obj1, obj2, obj3, obj4,
    obj5, obj6, obj7,
    arr1, arr2, arr3, arr4,
    num1, num2, num3, num4,
    num5, num6, num7, num8,
    exp1, exp2, exp3
};

//----------------------------------------------------------

namespace detail {

inline
double
pow10(int exp) noexcept
{
    static double const tab[618] = {
                        1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301,

        1e-300, 1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291,
        1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282, 1e-281,
        1e-280, 1e-279, 1e-278, 1e-277, 1e-276, 1e-275, 1e-274, 1e-273, 1e-272, 1e-271,
        1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264, 1e-263, 1e-262, 1e-261,
        1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254, 1e-253, 1e-252, 1e-251,
        1e-250, 1e-249, 1e-248, 1e-247, 1e-246, 1e-245, 1e-244, 1e-243, 1e-242, 1e-241,
        1e-240, 1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231,
        1e-230, 1e-229, 1e-228, 1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221,
        1e-220, 1e-219, 1e-218, 1e-217, 1e-216, 1e-215, 1e-214, 1e-213, 1e-212, 1e-211,
        1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204, 1e-203, 1e-202, 1e-201,

        1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192, 1e-191,
        1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183, 1e-182, 1e-181,
        1e-180, 1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174, 1e-173, 1e-172, 1e-171,
        1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165, 1e-164, 1e-163, 1e-162, 1e-161,
        1e-160, 1e-159, 1e-158, 1e-157, 1e-156, 1e-155, 1e-154, 1e-153, 1e-152, 1e-151,
        1e-150, 1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144, 1e-143, 1e-142, 1e-141,
        1e-140, 1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132, 1e-131,
        1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121,
        1e-120, 1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111,
        1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102, 1e-101,

        1e-100, 1e-099, 1e-098, 1e-097, 1e-096, 1e-095, 1e-094, 1e-093, 1e-092, 1e-091,
        1e-090, 1e-089, 1e-088, 1e-087, 1e-086, 1e-085, 1e-084, 1e-083, 1e-082, 1e-081,
        1e-080, 1e-079, 1e-078, 1e-077, 1e-076, 1e-075, 1e-074, 1e-073, 1e-072, 1e-071,
        1e-070, 1e-069, 1e-068, 1e-067, 1e-066, 1e-065, 1e-064, 1e-063, 1e-062, 1e-061,
        1e-060, 1e-059, 1e-058, 1e-057, 1e-056, 1e-055, 1e-054, 1e-053, 1e-052, 1e-051,
        1e-050, 1e-049, 1e-048, 1e-047, 1e-046, 1e-045, 1e-044, 1e-043, 1e-042, 1e-041,
        1e-040, 1e-039, 1e-038, 1e-037, 1e-036, 1e-035, 1e-034, 1e-033, 1e-032, 1e-031,
        1e-030, 1e-029, 1e-028, 1e-027, 1e-026, 1e-025, 1e-024, 1e-023, 1e-022, 1e-021,
        1e-020, 1e-019, 1e-018, 1e-017, 1e-016, 1e-015, 1e-014, 1e-013, 1e-012, 1e-011,
        1e-010, 1e-009, 1e-008, 1e-007, 1e-006, 1e-005, 1e-004, 1e-003, 1e-002, 1e-001,

        1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009,
        1e+010, 1e+011, 1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019,
        1e+020, 1e+021, 1e+022, 1e+023, 1e+024, 1e+025, 1e+026, 1e+027, 1e+028, 1e+029,
        1e+030, 1e+031, 1e+032, 1e+033, 1e+034, 1e+035, 1e+036, 1e+037, 1e+038, 1e+039,
        1e+040, 1e+041, 1e+042, 1e+043, 1e+044, 1e+045, 1e+046, 1e+047, 1e+048, 1e+049,
        1e+050, 1e+051, 1e+052, 1e+053, 1e+054, 1e+055, 1e+056, 1e+057, 1e+058, 1e+059,
        1e+060, 1e+061, 1e+062, 1e+063, 1e+064, 1e+065, 1e+066, 1e+067, 1e+068, 1e+069,
        1e+070, 1e+071, 1e+072, 1e+073, 1e+074, 1e+075, 1e+076, 1e+077, 1e+078, 1e+079,
        1e+080, 1e+081, 1e+082, 1e+083, 1e+084, 1e+085, 1e+086, 1e+087, 1e+088, 1e+089,
        1e+090, 1e+091, 1e+092, 1e+093, 1e+094, 1e+095, 1e+096, 1e+097, 1e+098, 1e+099,

        1e+100, 1e+101, 1e+102, 1e+103, 1e+104, 1e+105, 1e+106, 1e+107, 1e+108, 1e+109,
        1e+110, 1e+111, 1e+112, 1e+113, 1e+114, 1e+115, 1e+116, 1e+117, 1e+118, 1e+119,
        1e+120, 1e+121, 1e+122, 1e+123, 1e+124, 1e+125, 1e+126, 1e+127, 1e+128, 1e+129,
        1e+130, 1e+131, 1e+132, 1e+133, 1e+134, 1e+135, 1e+136, 1e+137, 1e+138, 1e+139,
        1e+140, 1e+141, 1e+142, 1e+143, 1e+144, 1e+145, 1e+146, 1e+147, 1e+148, 1e+149,
        1e+150, 1e+151, 1e+152, 1e+153, 1e+154, 1e+155, 1e+156, 1e+157, 1e+158, 1e+159,
        1e+160, 1e+161, 1e+162, 1e+163, 1e+164, 1e+165, 1e+166, 1e+167, 1e+168, 1e+169,
        1e+170, 1e+171, 1e+172, 1e+173, 1e+174, 1e+175, 1e+176, 1e+177, 1e+178, 1e+179,
        1e+180, 1e+181, 1e+182, 1e+183, 1e+184, 1e+185, 1e+186, 1e+187, 1e+188, 1e+189,
        1e+190, 1e+191, 1e+192, 1e+193, 1e+194, 1e+195, 1e+196, 1e+197, 1e+198, 1e+199,

        1e+200, 1e+201, 1e+202, 1e+203, 1e+204, 1e+205, 1e+206, 1e+207, 1e+208, 1e+209,
        1e+210, 1e+211, 1e+212, 1e+213, 1e+214, 1e+215, 1e+216, 1e+217, 1e+218, 1e+219,
        1e+220, 1e+221, 1e+222, 1e+223, 1e+224, 1e+225, 1e+226, 1e+227, 1e+228, 1e+229,
        1e+230, 1e+231, 1e+232, 1e+233, 1e+234, 1e+235, 1e+236, 1e+237, 1e+238, 1e+239,
        1e+240, 1e+241, 1e+242, 1e+243, 1e+244, 1e+245, 1e+246, 1e+247, 1e+248, 1e+249,
        1e+250, 1e+251, 1e+252, 1e+253, 1e+254, 1e+255, 1e+256, 1e+257, 1e+258, 1e+259,
        1e+260, 1e+261, 1e+262, 1e+263, 1e+264, 1e+265, 1e+266, 1e+267, 1e+268, 1e+269,
        1e+270, 1e+271, 1e+272, 1e+273, 1e+274, 1e+275, 1e+276, 1e+277, 1e+278, 1e+279,
        1e+280, 1e+281, 1e+282, 1e+283, 1e+284, 1e+285, 1e+286, 1e+287, 1e+288, 1e+289,
        1e+290, 1e+291, 1e+292, 1e+293, 1e+294, 1e+295, 1e+296, 1e+297, 1e+298, 1e+299,

        1e+300, 1e+301, 1e+302, 1e+303, 1e+304, 1e+305, 1e+306, 1e+307, 1e+308 };

    if (exp < -308 || exp > 308)
    {
        return std::pow(10.0, exp);
    }
    else
    {
        exp += 308;
        BOOST_ASSERT(exp >= 0 && exp < 618);
        return tab[exp];
    }
}

inline
double
dec_to_float(
    std::uint64_t m,
    std::int32_t e,
    bool neg) noexcept
{
    if(neg)
        return (-static_cast<
            double>(m)) *
            pow10(e);
    return (static_cast<
        double>(m)) *
        pow10(e);
}

} // detail

//----------------------------------------------------------

bool
basic_parser::
is_control(char c) noexcept
{
    return static_cast<unsigned char>(c) < 32;
}

char
basic_parser::
hex_digit(char c) noexcept
{
#if 1
    // by Peter Dimov
    if( c >= '0' && c <= '9' )
        return c - '0';
    c &= ~0x20;
    if( c >= 'A' && c <= 'F' )
        return 10 + c - 'A';
    return -1;
#else
    // VFALCO This is a tad slower and makes the binary larger
    static constexpr char tab[] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //   0
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //  16
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //  32
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1, //  48
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //  64
        10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //  80
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //  96
        10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 112
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 128
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 144
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 160
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 176
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 192
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 208
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 224
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 240
    };
    return tab[static_cast<unsigned char>(c)];
#endif
}

void
basic_parser::
reserve()
{
    // Reserve the largest stack we need,
    // to avoid reallocation during suspend.
    auto const n =
        1 +
        (1 + sizeof(std::size_t)) * depth_ +
        1
        ;
    st_.reserve(n);
}

void
basic_parser::
suspend(state st)
{
    reserve();
    st_.push(st);
}

void
basic_parser::
suspend(state st, std::size_t n)
{
    reserve();
    st_.push(n);
    st_.push(st);
}

void
basic_parser::
suspend(state st, number const& num)
{
    reserve();
    num_ = num;
    st_.push(st);
}

// return `false` if fully consumed
bool
basic_parser::
skip_white(const_stream& cs)
{
    char const * p = cs.data();
    std::size_t n = cs.remain();

    std::size_t n2 = detail::count_whitespace( p, n );
    cs.skip( n2 );

    return n2 < n;
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_element(
    Handler& h,
    const_stream& cs) ->
        result
{
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::ele1: goto do_ele1;
        case state::ele2: goto do_ele2;
        case state::ele3: goto do_ele3;
        }
    }
do_ele1:
    if(BOOST_JSON_UNLIKELY(
        ! skip_white(cs)))
    {
        if(more_)
            suspend(state::ele1);
        return result::partial;
    }
do_ele2:
    {
        result r = parse_value<StackEmpty>(h, cs);
        if(BOOST_JSON_UNLIKELY(r))
        {
            if(more_ && r == result::partial)
                suspend(state::ele2);
            return r;
        }
    }
do_ele3:
    if(BOOST_JSON_UNLIKELY(
        ! skip_white(cs)))
    {
        if(more_)
        {
            suspend(state::ele3);
            return result::partial;
        }
    }
    return result::ok;
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_value(
    Handler& h,
    const_stream& cs0) ->
        result
{
    if(StackEmpty || st_.empty())
    {
        switch(*cs0)
        {
        case 'n':
            if(BOOST_JSON_LIKELY(cs0.remain() >= 4))
            {
                if(BOOST_JSON_LIKELY(std::memcmp(
                    cs0.data(), "null", 4) == 0))
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_null(ec_)))
                        return result::fail;
                    cs0.skip(4);
                    return result::ok;
                }
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs0;
            return parse_null<true>(h, cs0);
        case 't':
            if(BOOST_JSON_LIKELY(cs0.remain() >= 4))
            {
                if(BOOST_JSON_LIKELY(std::memcmp(
                    cs0.data(), "true", 4) == 0))
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_bool(true, ec_)))
                        return result::fail;
                    cs0.skip(4);
                    return result::ok;
                }
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs0;
            return parse_true<true>(h, cs0);
        case 'f':
            if(BOOST_JSON_LIKELY(cs0.remain() >= 5))
            {
                if(BOOST_JSON_LIKELY(std::memcmp(
                    cs0.data() + 1, "alse", 4) == 0))
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_bool(false, ec_)))
                        return result::fail;
                    cs0.skip(5);
                    return result::ok;
                }
                ec_ = error::expected_false;
                return result::fail;
            }
            ++cs0;
            return parse_false<true>(h, cs0);
        case '\x22': // '"'
            return parse_string<true>(h, cs0);
        case '{':
            return parse_object<true>(h, cs0);
        case '[':
            return parse_array<true>(h, cs0);
        default:
            return parse_number<true>(h, cs0);
        }
    }
    else
    {
        return resume_value<StackEmpty>(h, cs0);
    }
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
resume_value(
    Handler& h,
    const_stream& cs0) ->
        result
{
    state st;
    st_.peek(st);
    switch(st)
    {
    default:
    case state::nul1: case state::nul2:
    case state::nul3:
        return parse_null<StackEmpty>(h, cs0);

    case state::tru1: case state::tru2:
    case state::tru3:
        return parse_true<StackEmpty>(h, cs0);

    case state::fal1: case state::fal2:
    case state::fal3: case state::fal4:
        return parse_false<StackEmpty>(h, cs0);

    case state::str1: case state::str2:
    case state::str3: case state::str4:
    case state::str5: case state::str6:
    case state::str7:
    case state::sur1: case state::sur2:
    case state::sur3: case state::sur4:
    case state::sur5: case state::sur6:
        return parse_string<StackEmpty>(h, cs0);

    case state::arr1: case state::arr2:
    case state::arr3: case state::arr4:
        return parse_array<StackEmpty>(h, cs0);
        
    case state::obj1: case state::obj2:
    case state::obj3: case state::obj4:
    case state::obj5: case state::obj6:
    case state::obj7:
        return parse_object<StackEmpty>(h, cs0);
        
    case state::num1: case state::num2:
    case state::num3: case state::num4:
    case state::num5: case state::num6:
    case state::num7: case state::num8:
    case state::exp1: case state::exp2:
    case state::exp3:
        return parse_number<StackEmpty>(h, cs0);
    }
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_null(
    Handler& h,
    const_stream& cs0) ->
        result
{
    detail::local_const_stream cs(cs0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::nul1: goto do_nul1;
        case state::nul2: goto do_nul2;
        case state::nul3: goto do_nul3;
        }
    }
do_nul1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'u'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul1);
        return result::partial;
    }
do_nul2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul2);
        return result::partial;
    }
do_nul3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul3);
        return result::partial;
    }
    if(BOOST_JSON_UNLIKELY(
        ! h.on_null(ec_)))
        return result::fail;
    return result::ok;
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_true(
    Handler& h,
    const_stream& cs0) ->
        result
{
    detail::local_const_stream cs(cs0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::tru1: goto do_tru1;
        case state::tru2: goto do_tru2;
        case state::tru3: goto do_tru3;
        }
    }
do_tru1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'r'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru1);
        return result::partial;
    }
do_tru2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'u'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru2);
        return result::partial;
    }
do_tru3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'e'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru3);
        return result::partial;
    }
    if(BOOST_JSON_UNLIKELY(
        ! h.on_bool(true, ec_)))
        return result::fail;
    return result::ok;
}

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_false(
    Handler& h,
    const_stream& cs0) ->
        result
{
    detail::local_const_stream cs(cs0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::fal1: goto do_fal1;
        case state::fal2: goto do_fal2;
        case state::fal3: goto do_fal3;
        case state::fal4: goto do_fal4;
        }
    }
do_fal1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'a'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal1);
        return result::partial;
    }
do_fal2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal2);
        return result::partial;
    }
do_fal3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 's'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal3);
        return result::partial;
    }
do_fal4:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'e'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal4);
        return result::partial;
    }
    if(BOOST_JSON_UNLIKELY(
        ! h.on_bool(false, ec_)))
        return result::fail;
    return result::ok;
}

//----------------------------------------------------------

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_string(
    Handler& h,
    const_stream& cs0) ->
        result
{
    detail::local_const_stream cs(cs0);
    detail::buffer<BOOST_JSON_PARSER_BUFFER_SIZE> temp;
    char const* start;
    if(StackEmpty || st_.empty())
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != '\x22')) // '"'
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
        start = cs.data();
    }
    else
    {
        start = cs.data();
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::str1: goto do_str1;
        case state::str2: goto do_str2;
        case state::str3: goto do_str3;
        case state::str4: goto do_str4;
        case state::str5: goto do_str5;
        case state::str6: goto do_str6;
        case state::str7: goto do_str7;
        case state::sur1: goto do_sur1;
        case state::sur2: goto do_sur2;
        case state::sur3: goto do_sur3;
        case state::sur4: goto do_sur4;
        case state::sur5: goto do_sur5;
        case state::sur6: goto do_sur6;
        }
    }

    //----------------------------------
    //
    // zero-copy unescaped runs
    //
do_str1:
    cs.skip(detail::count_unescaped(
        cs.data(), cs.remain()));
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            char const c = *cs;
            if(BOOST_JSON_LIKELY(
                c == '\x22')) // '"'
            {
                if(is_key_)
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_key({ start,
                        cs.used(start)}, ec_)))
                        return result::fail;
                    is_key_ = false;
                }
                else
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_string({ start,
                        cs.used(start)}, ec_)))
                        return result::fail;
                }
                ++cs;
                return result::ok;
            }
            else if(BOOST_JSON_LIKELY(
                c == '\\'))
            {
                if(BOOST_JSON_LIKELY(
                    cs.data() > start))
                {
                    if(is_key_)
                    {
                        if(BOOST_JSON_UNLIKELY(
                            ! h.on_key_part({start,
                                cs.used(start)}, ec_)))
                            return result::fail;
                    }
                    else
                    {
                        if(BOOST_JSON_UNLIKELY(
                            ! h.on_string_part({start,
                                cs.used(start)}, ec_)))
                            return result::fail;
                    }
                }
                goto do_str2;
            }
            else if(BOOST_JSON_UNLIKELY(
                is_control(c)))
            {
                // invalid character
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs;
            continue;
        }
        if(BOOST_JSON_LIKELY(more_))
        {
            if(BOOST_JSON_LIKELY(
                cs.data() > start))
            {
                if(is_key_)
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_key_part({start,
                            cs.used(start)}, ec_)))
                        return result::fail;
                }
                else
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_string_part({start,
                            cs.used(start)}, ec_)))
                        return result::fail;
                }
            }
            suspend(state::str1);
        }
        return result::partial;
    }

    //----------------------------------
    //
    // build a temporary buffer,
    // handling escapes and unicode.
    //
do_str2:
    // JSON escapes can never make the
    // transcoded utf8 string larger.
    cs.clip(temp.capacity());
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            if(BOOST_JSON_LIKELY(
                *cs == '\x22')) // '"'
            {
                if(is_key_)
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_key(temp, ec_)))
                        return result::fail;
                    is_key_ = false;
                }
                else
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_string(temp, ec_)))
                        return result::fail;
                }
                ++cs;
                return result::ok;
            }
            else if(*cs == '\\')
            {
                ++cs;
                goto do_str3;
            }
            else if(is_control(*cs))
            {
                // invalid character
                ec_ = error::syntax;
                return result::fail;
            }
            temp.push_back(*cs);
            ++cs;
            continue;
        }
        // flush
        if(BOOST_JSON_LIKELY(
            ! temp.empty()))
        {
            if(is_key_)
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h.on_key_part(temp, ec_)))
                    return result::fail;
            }
            else
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h.on_string_part(temp, ec_)))
                    return result::fail;
            }
            temp.clear();
        }
        cs.clip(temp.capacity());
        if(cs)
            continue;
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::str2);
        return result::partial;

        // handle escaped character
do_str3:
        if(BOOST_JSON_LIKELY(cs))
        {
            switch(*cs)
            {
            case '\x22': // '"'
                temp.push_back('\x22');
                break;
            case '\\':
                temp.push_back('\\');
                break;
            case '/':
                temp.push_back('/');
                break;
            case 'b':
                temp.push_back('\x08');
                break;
            case 'f':
                temp.push_back('\x0c');
                break;
            case 'n':
                temp.push_back('\x0a');
                break;
            case 'r':
                temp.push_back('\x0d');
                break;
            case 't':
                temp.push_back('\x09');
                break;
            case 'u':
                // utf16 escape
                //
                // fast path only when the buffer
                // is large enough for 2 surrogates
                if(BOOST_JSON_LIKELY(
                    cs.remain() >= 11))
                {
                    int32_t d;
                    std::memcpy(
                        &d, cs.data() + 1, 4);
                #if ! BOOST_JSON_BIG_ENDIAN
                    int d4 = hex_digit(static_cast<
                        unsigned char>(d >> 24));
                    int d3 = hex_digit(static_cast<
                        unsigned char>(d >> 16));
                    int d2 = hex_digit(static_cast<
                        unsigned char>(d >> 8));
                    int d1 = hex_digit(static_cast<
                        unsigned char>(d));
                #else
                    int d1 = hex_digit(static_cast<
                        unsigned char>(d >> 24));
                    int d2 = hex_digit(static_cast<
                        unsigned char>(d >> 16));
                    int d3 = hex_digit(static_cast<
                        unsigned char>(d >> 8));
                    int d4 = hex_digit(static_cast<
                        unsigned char>(d));
                #endif
                    if(BOOST_JSON_UNLIKELY(
                        (d1 | d2 | d3 | d4) == -1))
                    {
                        if(d1 != -1)
                            ++cs;
                        if(d2 != -1)
                            ++cs;
                        if(d3 != -1)
                            ++cs;
                        ec_ = error::expected_hex_digit;
                        return result::fail;
                    }
                    unsigned const u1 =
                        (d1 << 12) + (d2 << 8) +
                        (d3 << 4) + d4;
                    if(BOOST_JSON_LIKELY(
                        u1 < 0xd800 || u1 > 0xdfff))
                    {
                        cs.skip(5);
                        temp.append_utf8(u1);
                        continue;
                    }
                    if(u1 > 0xdbff)
                    {
                        ec_ = error::illegal_leading_surrogate;
                        return result::fail;
                    }
                    cs.skip(5);
                    if(BOOST_JSON_UNLIKELY(*cs != '\\'))
                    {
                        ec_ = error::syntax;
                        return result::fail;
                    }
                    ++cs;
                    if(BOOST_JSON_UNLIKELY(*cs != 'u'))
                    {
                        ec_ = error::syntax;
                        return result::fail;
                    }
                    ++cs;
                    std::memcpy(&d, cs.data(), 4);
                #if ! BOOST_JSON_BIG_ENDIAN
                    d4 = hex_digit(static_cast<
                        unsigned char>(d >> 24));
                    d3 = hex_digit(static_cast<
                        unsigned char>(d >> 16));
                    d2 = hex_digit(static_cast<
                        unsigned char>(d >> 8));
                    d1 = hex_digit(static_cast<
                        unsigned char>(d));
                #else
                    d1 = hex_digit(static_cast<
                        unsigned char>(d >> 24));
                    d2 = hex_digit(static_cast<
                        unsigned char>(d >> 16));
                    d3 = hex_digit(static_cast<
                        unsigned char>(d >> 8));
                    d4 = hex_digit(static_cast<
                        unsigned char>(d));
                #endif
                    if(BOOST_JSON_UNLIKELY(
                        (d1 | d2 | d3 | d4) == -1))
                    {
                        if(d1 != -1)
                            ++cs;
                        if(d2 != -1)
                            ++cs;
                        if(d3 != -1)
                            ++cs;
                        ec_ = error::expected_hex_digit;
                        return result::fail;
                    }
                    unsigned const u2 =
                        (d1 << 12) + (d2 << 8) +
                        (d3 << 4) + d4;
                    if(BOOST_JSON_UNLIKELY(
                        u2 < 0xdc00 || u2 > 0xdfff))
                    {
                        ec_ = error::illegal_trailing_surrogate;
                        return result::fail;
                    }
                    cs.skip(4);
                    unsigned cp =
                        ((u1 - 0xd800) << 10) +
                         (u2  - 0xdc00) +
                         0x10000;
                    temp.append_utf8(cp);
                    continue;
                }
                // flush
                if(BOOST_JSON_LIKELY(
                    ! temp.empty()))
                {
                    if(is_key_)
                    {
                        if(BOOST_JSON_UNLIKELY(
                            ! h.on_key_part(temp, ec_)))
                            return result::fail;
                    }
                    else
                    {
                        if(BOOST_JSON_UNLIKELY(
                            ! h.on_string_part(temp, ec_)))
                            return result::fail;
                    }
                    temp.clear();
                    cs.clip(temp.capacity());
                }
                ++cs;
                goto do_str4;

            default:
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs;
            continue;
        }
        if(BOOST_JSON_LIKELY(more_))
        {
            // flush
            if(BOOST_JSON_LIKELY(
                ! temp.empty()))
            {
                if(is_key_)
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_key_part(temp, ec_)))
                        return result::fail;
                }
                else
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h.on_string_part(temp, ec_)))
                        return result::fail;
                }
                temp.clear();
            }
            suspend(state::str3);
        }
        return result::partial;

        // utf16 escape
    do_str4:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u1_ = d << 12;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str4);
            return result::partial;
        }
    do_str5:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u1_ += d << 8;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str5);
            return result::partial;
        }
    do_str6:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u1_ += d << 4;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str6);
            return result::partial;
        }
    do_str7:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u1_ += d;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str7);
            return result::partial;
        }
        if( u1_ < 0xd800 ||
            u1_ > 0xdfff)
        {
            BOOST_ASSERT(temp.empty());
            temp.append_utf8(u1_);
            continue;
        }
        if(u1_ > 0xdbff)
        {
            ec_ = error::illegal_leading_surrogate;
            return result::fail;
        }
    do_sur1:
        if(BOOST_JSON_LIKELY(cs))
        {
            if(BOOST_JSON_UNLIKELY(*cs != '\\'))
            {
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur1);
            return result::partial;
        }
    do_sur2:
        if(BOOST_JSON_LIKELY(cs))
        {
            if(BOOST_JSON_UNLIKELY(*cs != 'u'))
            {
                ec_ = error::syntax;
                return result::fail;
            }
            ++cs;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur2);
            return result::partial;
        }
    do_sur3:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u2_ = d << 12;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur3);
            return result::partial;
        }
    do_sur4:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u2_ += d << 8;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur4);
            return result::partial;
        }
    do_sur5:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u2_ += d << 4;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur5);
            return result::partial;
        }
    do_sur6:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return result::fail;
            }
            ++cs;
            u2_ += d;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur6);
            return result::partial;
        }
        if(BOOST_JSON_UNLIKELY(
            u2_ < 0xdc00 || u2_ > 0xdfff))
        {
            ec_ = error::illegal_trailing_surrogate;
            return result::fail;
        }
        unsigned cp =
            ((u1_ - 0xd800) << 10) +
             (u2_ - 0xdc00) +
              0x10000;
        BOOST_ASSERT(temp.empty());
        temp.append_utf8(cp);
    }
}

//----------------------------------------------------------

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_object(
    Handler& h,
    const_stream& cs0) ->
        result
{
    char c;
    std::size_t n;
    detail::local_const_stream cs(cs0);
    if(StackEmpty || st_.empty())
    {
        BOOST_ASSERT(*cs == '{');
        ++depth_;
        if(BOOST_JSON_UNLIKELY(
            ! h.on_object_begin(ec_)))
            return result::fail;
        ++cs;
        n = 0;
    }
    else
    {
        state st;
        st_.pop(st);
        st_.pop(n);
        switch(st)
        {
        default:
        case state::obj1: goto do_obj1;
        case state::obj2: goto do_obj2;
        case state::obj3: goto do_obj3;
        case state::obj4: goto do_obj4;
        case state::obj5: goto do_obj5;
        case state::obj6: goto do_obj6;
        case state::obj7: goto do_obj7;
        }
    }
do_obj1:
    if(BOOST_JSON_UNLIKELY(
        ! skip_white(cs)))
    {
        if(more_)
            suspend(state::obj1, n);
        return result::partial;
    }
    c = *cs;
    if(BOOST_JSON_UNLIKELY(c == '}'))
    {
        if(BOOST_JSON_UNLIKELY(
            ! h.on_object_end(n, ec_)))
            return result::fail;
        --depth_;
        ++cs;
        return result::ok;
    }
    for(;;)
    {
        is_key_ = true;
do_obj2:
        {
            result r = parse_string<StackEmpty>(h, cs);
            if(BOOST_JSON_UNLIKELY(r))
            {
                if(more_ && r == result::partial)
                    suspend(state::obj2, n);
                return r;
            }
        }
do_obj3:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::obj3, n);
            return result::partial;
        }
        if(BOOST_JSON_UNLIKELY(*cs != ':'))
        {
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
do_obj4:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::obj4, n);
            return result::partial;
        }
do_obj5:
        {
            result r = parse_value<StackEmpty>(h, cs);
            if(BOOST_JSON_UNLIKELY(r))
            {
                if(more_ && r == result::partial)
                    suspend(state::obj5, n);
                return r;
            }
            ++n;
        }
do_obj6:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::obj6, n);
            return result::partial;
        }
        if(BOOST_JSON_UNLIKELY(*cs != ','))
        {
            if(BOOST_JSON_LIKELY(*cs == '}'))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h.on_object_end(n, ec_)))
                    return result::fail;
                --depth_;
                ++cs;
                return result::ok;
            }
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
do_obj7:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::obj7, n);
            return result::partial;
        }
    }
}

//----------------------------------------------------------

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_array(
    Handler& h,
    const_stream& cs0) ->
        result
{
    char c;
    std::size_t n;
    detail::local_const_stream cs(cs0);
    if(StackEmpty || st_.empty())
    {
        BOOST_ASSERT(*cs == '[');
        ++depth_;
        if(BOOST_JSON_UNLIKELY(
            ! h.on_array_begin(ec_)))
            return result::fail;
        ++cs;
        n = 0;
    }
    else
    {
        state st;
        st_.pop(st);
        st_.pop(n);
        switch(st)
        {
        default:
        case state::arr1: goto do_arr1;
        case state::arr2: goto do_arr2;
        case state::arr3: goto do_arr3;
        case state::arr4: goto do_arr4;
        }
    }
do_arr1:
    if(BOOST_JSON_UNLIKELY(
        ! skip_white(cs)))
    {
        if(more_)
            suspend(state::arr1, n);
        return result::partial;
    }
    c = *cs;
    if(c == ']')
    {
        if(BOOST_JSON_UNLIKELY(
            ! h.on_array_end(n, ec_)))
            return result::fail;
        --depth_;
        ++cs;
        return result::ok;
    }
    for(;;)
    {
do_arr2:
        {
            result r = parse_value<StackEmpty>(h, cs);
            if(BOOST_JSON_UNLIKELY(r))
            {
                if(more_ && r == result::partial)
                    suspend(state::arr2, n);
                return r;
            }
            ++n;
        }
do_arr3:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::arr3, n);
            return result::partial;
        }
        if(*cs != ',')
        {
            if(*cs == ']')
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h.on_array_end(n, ec_)))
                    return result::fail;
                --depth_;
                ++cs;
                return result::ok;
            }
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
do_arr4:
        if(BOOST_JSON_UNLIKELY(
            ! skip_white(cs)))
        {
            if(more_)
                suspend(state::arr4, n);
            return result::partial;
        }
    }
}

//----------------------------------------------------------

template<bool StackEmpty, class Handler>
auto
basic_parser::
parse_number(
    Handler& h,
    const_stream& cs0) ->
    result
{
    number num;
    detail::local_const_stream cs(cs0);
    if(StackEmpty || st_.empty())
    {
        num.bias = 0;
        num.exp = 0;
        num.frac = false;
    }
    else
    {
        num = num_;
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::num1: goto do_num1;
        case state::num2: goto do_num2;
        case state::num3: goto do_num3;
        case state::num4: goto do_num4;
        case state::num5: goto do_num5;
        case state::num6: goto do_num6;
        case state::num7: goto do_num7;
        case state::num8: goto do_num8;
        case state::exp1: goto do_exp1;
        case state::exp2: goto do_exp2;
        case state::exp3: goto do_exp3;
        }
    }

    //----------------------------------
    //
    // '-'
    // leading minus sign
    //
    BOOST_ASSERT(cs);
    if(*cs == '-')
    {
        ++cs;
        num.neg = true;
    }
    else
    {
        num.neg = false;
    }

    // fast path

    if( cs.remain() >= 16 + 1 + 16 ) // digits . digits
    {
        int n1;

        if( *cs != '0' )
        {
            n1 = detail::count_digits( cs.data() );
            BOOST_ASSERT(n1 >= 0 && n1 <= 16);

            if( n1 == 0 )
            {
                // digit required
                ec_ = error::syntax;
                return result::fail;
            }

            num.mant = detail::parse_unsigned( 0, cs.data(), n1 );

            cs.skip( n1 );

            if( n1 == 16 )
            {
                goto do_num2;
            }
        }
        else
        {
            // 0. floating point
            num.mant = 0;
            n1 = 0;
            ++cs;
        }

        if( *cs != '.' )
        {
            goto do_num6;
        }

        ++cs;

        int n2 = detail::count_digits( cs.data() );
        BOOST_ASSERT(n2 >= 0 && n2 <= 16);

        if( n2 == 0 )
        {
            // digit required
            ec_ = error::syntax;
            return result::fail;
        }

        if( n1 + n2 >= 19 ) // int64 overflow
        {
            goto do_num6;
        }

        num.mant = detail::parse_unsigned( num.mant, cs.data(), n2 );

        BOOST_ASSERT(num.bias == 0);
        num.bias -= n2;

        cs.skip( n2 );

        char ch = *cs;

        if( (ch | 32) == 'e' )
        {
            ++cs;
            goto do_exp1;
        }
        else if( ch >= '0' && ch <= '9' )
        {
            goto do_num8;
        }

        goto finish_dub;
    }

    //----------------------------------
    //
    // DIGIT
    // first digit
    //
do_num1:
    if(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(BOOST_JSON_LIKELY(
            c >= '1' && c <= '9'))
        {
            ++cs;
            num.mant = c - '0';
        }
        else if(BOOST_JSON_UNLIKELY(
            c == '0'))
        {
            ++cs;
            num.mant = 0;
            goto do_num6;
        }
        else
        {
            ec_ = error::syntax;
            return result::fail;
        }
    }
    else
    {
        if(more_)
            suspend(state::num1, num);
        return result::partial;
    }

    //----------------------------------
    //
    // 1*DIGIT
    // significant digits left of decimal
    //
do_num2:
    if(num.neg)
    {
        for(;;)
        {
            if(BOOST_JSON_LIKELY(cs))
            {
                char const c = *cs;
                if(BOOST_JSON_LIKELY(
                    c >= '0' && c <= '9'))
                {
                    ++cs;
                    //              9223372036854775808 INT64_MIN
                    if( num.mant  > 922337203685477580 || (
                        num.mant == 922337203685477580 && c > '8'))
                        break;
                    num.mant = 10 * num.mant + c - '0';
                    continue;
                }
                goto do_num6; // [.eE]
            }
            else if(BOOST_JSON_UNLIKELY(more_))
            {
                suspend(state::num2, num);
                return result::partial;
            }
            goto finish_int;
        }
    }
    else
    {
        for(;;)
        {
            if(BOOST_JSON_LIKELY(cs))
            {
                char const c = *cs;
                if(BOOST_JSON_LIKELY(
                    c >= '0' && c <= '9'))
                {
                    ++cs;
                    //              18446744073709551615 UINT64_MAX
                    if( num.mant  > 1844674407370955161 || (
                        num.mant == 1844674407370955161 && c > '5'))
                        break;
                    num.mant = 10 * num.mant + c - '0';
                }
                else
                {
                    goto do_num6; // [.eE]
                }
            }
            else
            {
                if(BOOST_JSON_UNLIKELY(more_))
                {
                    suspend(state::num2, num);
                    return result::partial;
                }
                goto finish_int;
            }
        }
    }
    ++num.bias;

    //----------------------------------
    //
    // 1*DIGIT
    // non-significant digits left of decimal
    //
do_num3:
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            char const c = *cs;
            if(BOOST_JSON_UNLIKELY(
                c >= '0' && c <= '9'))
            {
                ++cs;
                // VFALCO check overflow
                ++num.bias;
            }
            else if(BOOST_JSON_LIKELY(
                c == '.'))
            {
                ++cs;
                break;
            }
            else if((c | 32) == 'e')
            {
                ++cs;
                goto do_exp1;
            }
            else
            {
                goto finish_dub;
            }
        }
        else
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                suspend(state::num3, num);
                return result::partial;
            }
            goto finish_dub;
        }
    }

    //----------------------------------
    //
    // DIGIT
    // first non-significant digit
    // to the right of decimal
    //
do_num4:
    if(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(BOOST_JSON_LIKELY(
            //static_cast<unsigned char>(c - '0') < 10))
            c >= '0' && c <= '9'))
        {
            ++cs;
        }
        else
        {
            // digit required
            ec_ = error::syntax;
            return result::fail;
        }
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
            suspend(state::num4, num);
        return result::partial;
    }

    //----------------------------------
    //
    // 1*DIGIT
    // non-significant digits
    // to the right of decimal
    //
do_num5:
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            char const c = *cs;
            if(BOOST_JSON_LIKELY(
                c >= '0' && c <= '9'))
            {
                ++cs;
            }
            else if((c | 32) == 'e')
            {
                ++cs;
                goto do_exp1;
            }
            else
            {
                goto finish_dub;
            }
        }
        else
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                suspend(state::num5, num);
                return result::partial;
            }
            goto finish_dub;
        }
    }

    //----------------------------------
    //
    // [.eE]
    //
do_num6:
    if(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(BOOST_JSON_LIKELY(
            c == '.'))
        {
            ++cs;
        }
        else if((c | 32) == 'e')
        {
            ++cs;
            goto do_exp1;
        }
        else
        {
            goto finish_int;
        }
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
        {
            suspend(state::num6, num);
            return result::partial;
        }
        goto finish_int;
    }

    //----------------------------------
    //
    // DIGIT
    // first significant digit
    // to the right of decimal
    //
do_num7:
    if(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(BOOST_JSON_UNLIKELY(
            c < '0' || c > '9'))
        {
            // digit required
            ec_ = error::syntax;
            return result::fail;
        }
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::num7, num);
            return result::partial;
        }
        // digit required
        ec_ = error::syntax;
        return result::fail;
    }

    //----------------------------------
    //
    // 1*DIGIT
    // significant digits
    // to the right of decimal
    //
do_num8:
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            char const c = *cs;
            if(BOOST_JSON_LIKELY(
                c >= '0' && c <= '9'))
            {
                ++cs;
                if(BOOST_JSON_LIKELY(
                    num.mant <= 9007199254740991)) // 2^53-1
                {
                    --num.bias;
                    num.mant = 10 * num.mant + c - '0';
                }
                else
                {
                    goto do_num5;
                }
            }
            else if((c | 32) == 'e')
            {
                ++cs;
                goto do_exp1;
            }
            else
            {
                goto finish_dub;
            }
        }
        else
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                suspend(state::num8, num);
                return result::partial;
            }
            goto finish_dub;
        }
    }

    //----------------------------------
    //
    // *[+-]
    //
do_exp1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(*cs == '+')
        {
            ++cs;
        }
        else if(*cs == '-')
        {
            ++cs;
            num.frac = true;
        }
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::exp1, num);
        return result::partial;
    }

    //----------------------------------
    //
    // DIGIT
    // first digit of the exponent
    //
do_exp2:
    if(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(BOOST_JSON_UNLIKELY(
            c < '0' || c > '9'))
        {
            // digit required
            ec_ = error::syntax;
            return result::fail;
        }
        ++cs;
        num.exp = c - '0';
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::exp2, num);
            return result::partial;
        }
        // digit required
        ec_ = error::syntax;
        return result::fail;
    }

    //----------------------------------
    //
    // 1*DIGIT
    // subsequent digits in the exponent
    //
do_exp3:
    for(;;)
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            char const c = *cs;
            if(BOOST_JSON_LIKELY(
                c >= '0' && c <= '9'))
            {
                if(BOOST_JSON_UNLIKELY
                //              2147483647 INT_MAX
                    (num.exp  > 214748364 || (
                     num.exp == 214748364 && c > '7')))
                {
                    ec_ = error::exponent_overflow;
                    return result::fail;
                }
                ++cs;
                num.exp = 10 * num.exp + c - '0';
                continue;
            }
        }
        else if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::exp3, num);
            return result::partial;
        }
        goto finish_dub;
    }

finish_int:
    if(num.neg)
    {
        if(BOOST_JSON_UNLIKELY(
            ! h.on_int64(static_cast<
                int64_t>(~num.mant + 1), ec_)))
            return result::fail;
        return result::ok;
    }
    if(num.mant <= INT64_MAX)
    {
        if(BOOST_JSON_UNLIKELY(
            ! h.on_int64(static_cast<
                int64_t>(num.mant), ec_)))
            return result::fail;
        return result::ok;
    }
    if(BOOST_JSON_UNLIKELY(
        ! h.on_uint64(num.mant, ec_)))
        return result::fail;
    return result::ok;

finish_dub:
    double const d = detail::dec_to_float(
        num.mant,
        num.bias + (num.frac ?
            -num.exp : num.exp),
        num.neg);
    if(BOOST_JSON_UNLIKELY(
        ! h.on_double(d, ec_)))
        return result::fail;
    return result::ok;
}

//----------------------------------------------------------

void
basic_parser::
reset() noexcept
{
    done_ = false;
    more_ = true;
    st_.clear();
}

//----------------------------------------------------------

template<class Handler>
std::size_t
basic_parser::
write_some(
    Handler& h,
    bool more,
    char const* data,
    std::size_t size,
    error_code& ec)
{
    // If this goes off, it means you forgot to
    // check is_done() before presenting more data
    // to the parser.
    BOOST_ASSERT(! done_);

    ec_ = {};
    result r;
    more_ = more;
    const_stream cs = { data, size };
    if(BOOST_JSON_LIKELY(
        st_.empty()))
    {
        // first time
        depth_ = 0;
        is_key_ = false;
        if(BOOST_JSON_UNLIKELY(
            ! h.on_document_begin(ec_)))
        {
            ec = ec_;
            return 0;
        }
        r = parse_element<true>(h, cs);
    }
    else
    {
        r = parse_element<false>(h, cs);
    }
    if(BOOST_JSON_LIKELY(! r))
    {
        BOOST_ASSERT(! ec_);
        done_ = true;
        h.on_document_end(ec_);
    }
    else if(r == result::partial)
    {
        BOOST_ASSERT(! ec_);
        if(! more_)
            ec_ = error::incomplete;
    }
    ec = ec_;
    return cs.data() - data;
}

#endif

} // json
} // boost

#endif
