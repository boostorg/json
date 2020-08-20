//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_BASIC_PARSER_HPP
#define BOOST_JSON_BASIC_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/detail/basic_parser.hpp>
#include <boost/json/detail/buffer.hpp>
#include <boost/json/detail/sse2.hpp>
#include <cmath>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

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

    if(exp < -308 || exp > 308)
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

inline
uint32_t
little_endian(uint32_t v)
{
#ifdef BOOST_JSON_BIG_ENDIAN
    v = (((v & 0xFF000000) >> 24) |
        ((v & 0x00FF0000) >> 8) |
        ((v & 0x0000FF00) << 8) |
        ((v & 0x000000FF) << 24))
#endif
    return v;
}

} // detail

//----------------------------------------------------------

template<class Handler>
bool
basic_parser<Handler>::
is_control(char c) noexcept
{
    return static_cast<unsigned char>(c) < 32;
}

template<class Handler>
char
basic_parser<Handler>::
hex_digit(char c) noexcept
{
    // by Peter Dimov
    if( c >= '0' && c <= '9' )
        return c - '0';
    c &= ~0x20;
    if( c >= 'A' && c <= 'F' )
        return 10 + c - 'A';
    return -1;
}

template<class Handler>
void
basic_parser<Handler>::
reserve()
{
    if(BOOST_JSON_LIKELY(
        ! st_.empty()))
        return;
    // Reserve the largest stack we need,
    // to avoid reallocation during suspend.
    st_.reserve(
        sizeof(state) + // document parsing state
        (sizeof(state) * depth()) + // array and object state
        sizeof(state) + // value parsing state
        sizeof(state)); // comment/utf8 state
}

//----------------------------------------------------------
//
// The canary value is returned by parse functions
// to indicate that the parser failed, or suspended.
// this is used as it is distinct from all valid values
// for data in write_some

template<class Handler>
const char*
basic_parser<Handler>::
canary()
{
    return reinterpret_cast<
        const char*>(this);
}

template<class Handler>
bool
basic_parser<Handler>::
incomplete(
    const detail::const_stream_wrapper& cs)
{
    return cs.begin() == canary();
}

//----------------------------------------------------------
//
// These functions are declared with the BOOST_NOINLINE
// attribute to avoid polluting the parsers hot-path.
// They return the canary value to indicate suspension
// or failure.

template<class Handler>
const char*
basic_parser<Handler>::
suspend_or_fail(state st)
{
    if(BOOST_JSON_LIKELY(
        ! ec_ && more_))
    {
        // suspend
        reserve();
        st_.push_unchecked(st);
    }
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
fail(const char* p) noexcept
{
    end_ = p;
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
fail(
    const char* p, 
    error ev) noexcept
{
    end_ = p;
    ec_ = ev;
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
maybe_suspend(
    const char* p, 
    state st)
{
    end_ = p;
    if(BOOST_JSON_LIKELY(more_))
    {
        // suspend
        reserve();
        st_.push_unchecked(st);
    }
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
maybe_suspend(
    const char* p, 
    state st,
    const number& num)
{
    end_ = p;
    if(BOOST_JSON_LIKELY(more_))
    {
        // suspend
        num_ = num;
        reserve();
        st_.push_unchecked(st);;
    }
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
suspend(
    const char* p, 
    state st)
{
    end_ = p;
    // suspend
    reserve();
    st_.push_unchecked(st);
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
suspend(
    const char* p, 
    state st,
    const number& num)
{
    end_ = p;
    // suspend
    num_ = num;
    reserve();
    st_.push_unchecked(st);
    return canary();
}

template<class Handler>
const char*
basic_parser<Handler>::
syntax_error(
    const char* p)
{
    end_ = p;
    ec_ = error::syntax;
    return canary();
}

template<class Handler>
template<
    bool StackEmpty,
    bool ReturnValue,
    bool Terminal,
    bool AllowTrailing,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_comment(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    const char* start = cs.begin();
    std::size_t remain;
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
            default:
            case state::com1: goto do_com1;
            case state::com2: goto do_com2;
            case state::com3: goto do_com3;
            case state::com4: goto do_com4;
            case state::com5: goto do_com5;
        }
    }
    BOOST_ASSERT(*cs == '/');
    ++cs;
do_com1:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::com1);
    switch(*cs)
    {
    case '/':
        ++cs;
do_com2:
        // KRYSTIAN TODO: this is a mess, we have to fix this
        remain = cs.remain();
        cs = remain ? static_cast<const char*>(
            std::memchr(cs.begin(), '\n', remain)) : canary();
        if(! cs.begin())
            cs = canary();
        if(BOOST_JSON_UNLIKELY(incomplete(cs)))
        {
            // if the doc does not terminate
            // with a newline, treat it as the
            // end of the comment
            if(Terminal && ! more_)
            {
                if(BOOST_JSON_UNLIKELY(! h_.on_comment(
                    {start, cs.remain(start)}, ec_)))
                    return fail(cs.end());
                return cs.end();
            }
            if(BOOST_JSON_UNLIKELY(! h_.on_comment_part(
                {start, cs.remain(start)}, ec_)))
                return fail(cs.end());
            if(Terminal)
                return suspend(cs.end(), state::com2);
            return maybe_suspend(cs.end(), state::com2);
        }
        break;
    case '*':
        do
        {
            ++cs;
do_com3:
            // KRYSTIAN TODO: this is a mess, we have to fix this
            remain = cs.remain();
            cs = remain ? static_cast<const char*>(
                std::memchr(cs.begin(), '*', remain)) : canary();
            if(! cs.begin())
                cs = canary();
            // stopped inside a c comment
            if(BOOST_JSON_UNLIKELY(incomplete(cs)))
            {
                if(BOOST_JSON_UNLIKELY(! h_.on_comment_part(
                    {start, cs.remain(start)}, ec_)))
                    return fail(cs.end());
                return maybe_suspend(cs.end(), state::com3);
            }
            // found a asterisk, check if the next char is a slash
            ++cs;
do_com4:
            if(BOOST_JSON_UNLIKELY(! cs))
            {
                if(BOOST_JSON_UNLIKELY(! h_.on_comment_part(
                    {start, cs.used(start)}, ec_)))
                    return fail(cs.begin());
                return maybe_suspend(cs.begin(), state::com4);
            }
        }
        while(*cs != '/');
        break;
    default:
        return fail(cs.begin(), error::syntax);
    }
    ++cs;
    if(BOOST_JSON_UNLIKELY(! h_.on_comment(
        {start, cs.used(start)}, ec_)))
        return fail(cs.begin());
    if(! ReturnValue)
        return cs.begin();
do_com5:
    cs = detail::count_whitespace(cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::com5);
    return parse_value<StackEmpty, true, 
        AllowTrailing, AllowBadUTF8>(cs.begin());
}

template<class Handler>
template<bool StackEmpty>
const char*
basic_parser<Handler>::
validate_utf8(const char* p, const char* end)
{
    // 0 = invalid
    // 1 = 2 bytes, second byte [80, BF]
    // 2 = 3 bytes, second byte [A0, BF]
    // 3 = 3 bytes, second byte [80, BF]
    // 4 = 3 bytes, second byte [80, 9F]
    // 5 = 4 bytes, second byte [90, BF]
    // 6 = 4 bytes, second byte [80, BF]
    // 7 = 4 bytes, second byte [80, 8F]
    static constexpr char first[128]
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 
        5, 6, 6, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    };
    detail::const_stream_wrapper cs(p, end);
    unsigned char c;
    if(StackEmpty || st_.empty())
    {
        // fast path
        if(BOOST_JSON_LIKELY(
            cs.remain() >= 4))
        {
            BOOST_ASSERT(static_cast<
                unsigned char>(*cs) > 0x7F);
            uint32_t v;
            std::memcpy(&v, cs.begin(), 4);
            v = detail::little_endian(v);
            switch(first[v & 0x0000007F])
            {
            // 2 bytes, second byte [80, BF]
            case 1:
                if(BOOST_JSON_LIKELY(
                    (v & 0x0000C000) == 0x00008000))
                {
                    cs += 2;
                    return cs.begin();
                }
                break;
            // 3 bytes, second byte [A0, BF]
            case 2:
                if(BOOST_JSON_LIKELY(
                    (v & 0x00C0E000) == 0x0080A000))
                {
                    cs += 3;
                    return cs.begin();
                }
                break;
            // 3 bytes, second byte [80, BF]
            case 3:
                if(BOOST_JSON_LIKELY(
                    (v & 0x00C0C000) == 0x00808000))
                {
                    cs += 3;
                    return cs.begin();
                }
                break;
            // 3 bytes, second byte [80, 9F]
            case 4:
                if(BOOST_JSON_LIKELY(
                    (v & 0x00C0E000) == 0x00808000))
                {
                    cs += 3;
                    return cs.begin();
                }
                break;
            // 4 bytes, second byte [90, BF]
            case 5:
                if(BOOST_JSON_LIKELY(
                    (v & 0xC0C0FF00) + 
                    0x7F7F7000 <= 0x00002F00))
                {
                    cs += 4;
                    return cs.begin();
                }
                break;
            // 4 bytes, second byte [80, BF]
            case 6:
                if(BOOST_JSON_LIKELY(
                    (v & 0xC0C0C000) == 0x80808000))
                {
                    cs += 4;
                    return cs.begin();
                }
                break;
            // 4 bytes, second byte [80, 8F]
            case 7:
                if(BOOST_JSON_LIKELY(
                    (v & 0xC0C0F000) == 0x80808000))
                {
                    cs += 4;
                    return cs.begin();
                }
                break;
            }
            return fail(cs.begin(), error::syntax);
        }
    }
    else
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:;
        case state::utf1: goto do_utf1;
        case state::utf2: goto do_utf2;
        case state::utf3: goto do_utf3;
        case state::utf4: goto do_utf4;
        case state::utf5: goto do_utf5;
        case state::utf6: goto do_utf6;
        case state::utf7: goto do_utf7;
        case state::utf8: goto do_utf8;
        case state::utf9: goto do_utf9;
        case state::utf10: goto do_utf10;
        case state::utf11: goto do_utf11;
        case state::utf12: goto do_utf12;
        case state::utf13: goto do_utf13;
        case state::utf14: goto do_utf14;
        case state::utf15: goto do_utf15;
        case state::utf16: goto do_utf16;
        }
    }
    c = static_cast<unsigned char>(*cs);
    BOOST_ASSERT(c > 0x7F);
    ++cs;
    switch(first[c & 0x7F])
    {
    // 2 bytes
    case 1:
do_utf1:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf1);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
                
    // 3 bytes, second byte [A0, BF]
    case 2:
do_utf2:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf2);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xE0) != 0xA0))
            break;
        ++cs;
do_utf3:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf3);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
                
    // 3 bytes, second byte [80, BF]
    case 3:
do_utf4:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf4);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
do_utf5:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf5);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
                
    // 3 bytes, second byte [80, 9F]
    case 4:
do_utf6:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf6);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xE0) != 0x80))
            break;
        ++cs;
do_utf7:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf7);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();

    // 4 bytes, second byte [90, BF]
    case 5:
do_utf8:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf8);
        if(BOOST_JSON_UNLIKELY(
            (*cs + 0x70) > 0x2F))
            break;
        ++cs;
do_utf9:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf9);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
do_utf10:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf10);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
                
    // 4 bytes, second byte [80, BF]
    case 6:
do_utf11:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf11);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
do_utf12:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf12);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
do_utf13:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf13);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
                
    // 4 bytes, second byte [80, 8F]
    case 7:
do_utf14:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf14);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xF0) != 0x80))
            break;
        ++cs;
do_utf15:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf15);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
do_utf16:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::utf16);
        if(BOOST_JSON_UNLIKELY(
            (*cs & 0xC0) != 0x80))
            break;
        ++cs;
        return cs.begin();
    }
    return fail(cs.begin(), error::syntax);
}

template<class Handler>
template<bool StackEmpty>
const char*
basic_parser<Handler>::
parse_document(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::doc1: goto do_doc1;
        case state::doc2: goto do_doc2;
        case state::doc3: goto do_doc3;
        case state::doc4: goto do_doc4;
        }
    }
do_doc1:
    cs = detail::count_whitespace(cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::doc1);
do_doc2:
    switch(+opt_.allow_comments |
        (opt_.allow_trailing_commas << 1) |
        (opt_.allow_invalid_utf8 << 2))
    {
    // no extensions
    default:
        cs = parse_value<StackEmpty, false, false, false>(cs.begin());
        break;
    // comments
    case 1:
        cs = parse_value<StackEmpty, true, false, false>(cs.begin());
        break;
    // trailing
    case 2:
        cs = parse_value<StackEmpty, false, true, false>(cs.begin());
        break;
    // comments & trailing
    case 3:
        cs = parse_value<StackEmpty, true, true, false>(cs.begin());
        break;
    // skip validation
    case 4:
        cs = parse_value<StackEmpty, false, false, true>(cs.begin());
        break;
    // comments & skip validation
    case 5:
        cs = parse_value<StackEmpty, true, false, true>(cs.begin());
        break;
    // trailing & skip validation
    case 6:
        cs = parse_value<StackEmpty, false, true, true>(cs.begin());
        break;
    // comments & trailing & skip validation
    case 7:
        cs = parse_value<StackEmpty, true, true, true>(cs.begin());
        break;
    }
    if(BOOST_JSON_UNLIKELY(incomplete(cs)))
        return suspend_or_fail(state::doc2);
do_doc3:
    cs = detail::count_whitespace(cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
    {
        if(more_)
            return suspend(cs.begin(), state::doc3);
    }
    else if(opt_.allow_comments && *cs == '/')
    {
do_doc4:
        switch(+opt_.allow_trailing_commas |
            (opt_.allow_invalid_utf8 << 1))
        {
        // only comments
        default:
            cs = parse_comment<StackEmpty, false, true, false, false>(cs.begin());
            break;
        // trailing
        case 1:
            cs = parse_comment<StackEmpty, false, true, true, false>(cs.begin());
            break;
        // skip validation
        case 2:
            cs = parse_comment<StackEmpty, false, true, false, true>(cs.begin());
            break;
        // trailing & skip validation
        case 3:
            cs = parse_comment<StackEmpty, false, true, true, true>(cs.begin());
            break;
        }
        if(BOOST_JSON_UNLIKELY(incomplete(cs)))
            return suspend_or_fail(state::doc4);
        goto do_doc3;
    }
    return cs.begin();
}

template<class Handler>
template<
    bool StackEmpty, 
    bool AllowComments, 
    bool AllowTrailing, 
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_value(const char* p)
{
    if(StackEmpty || st_.empty())
    {
        static constexpr auto num = &basic_parser::parse_number<true, '+'>;
        static constexpr auto err = &basic_parser::syntax_error;

        static constexpr const char* (basic_parser::* jump_table[256])(const char*) = {
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err,
                &basic_parser::parse_unescaped<true, false, AllowBadUTF8>,
                err, err, err, err, err, err, err, err, err, err,
                &basic_parser::parse_number<true, '-'>,
                err,
                AllowComments ? &basic_parser::parse_comment<true, true, false, AllowTrailing, AllowBadUTF8> : err,
            &basic_parser::parse_number<true, '0'>,
                num, num, num, num, num, num, num, num, num, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err,
                &basic_parser::parse_array<true, AllowComments, AllowTrailing, AllowBadUTF8>,
                err, err, err, err,
            err, err, err, err, err, err,
                &basic_parser::parse_false<true>,
                err, err, err, err, err, err, err,
                &basic_parser::parse_null<true>,
                err,
            err, err, err, err,
                &basic_parser::parse_true<true>,
                err, err, err, err, err, err,
                &basic_parser::parse_object<true, AllowComments, AllowTrailing, AllowBadUTF8>,
                err, err, err, err,

            // negative values are converted to unsigned char, they are handled here
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, err, err, err, err, err, err, err, err, err, err, err, err, err, err
        };
        return (this->*jump_table
            [static_cast<unsigned char>(*p)])(p);
    }
    return resume_value<StackEmpty, AllowComments, 
        AllowTrailing, AllowBadUTF8>(p);
}

template<class Handler>
template<
    bool StackEmpty,
    bool AllowComments,
    bool AllowTrailing,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
resume_value(const char* p)
{
    state st;
    st_.peek(st);
    switch(st)
    {
    default:
    case state::nul1: case state::nul2:
    case state::nul3:
        return parse_null<StackEmpty>(p);

    case state::tru1: case state::tru2:
    case state::tru3:
        return parse_true<StackEmpty>(p);

    case state::fal1: case state::fal2:
    case state::fal3: case state::fal4:
        return parse_false<StackEmpty>(p);

    case state::str1: case state::utf17:
        return parse_unescaped<StackEmpty, 
            false, AllowBadUTF8>(p);
    
    case state::str2: case state::str3: 
    case state::str4: case state::str5: 
    case state::str6: case state::str7:
    case state::sur1: case state::sur2:
    case state::sur3: case state::sur4:
    case state::sur5: case state::sur6:
    case state::utf18:
        return parse_escaped<StackEmpty, 
            false, AllowBadUTF8>(p);

    case state::arr1: case state::arr2:
    case state::arr3: case state::arr4:
    case state::arr5: case state::arr6:
        return parse_array<StackEmpty, AllowComments, 
            AllowTrailing, AllowBadUTF8>(p);
        
    case state::obj1: case state::obj2:
    case state::obj3: case state::obj4:
    case state::obj5: case state::obj6:
    case state::obj7: case state::obj8: 
    case state::obj9: case state::obj10: 
    case state::obj11:
        return parse_object<StackEmpty, AllowComments, 
            AllowTrailing, AllowBadUTF8>(p);
        
    case state::num1: case state::num2:
    case state::num3: case state::num4:
    case state::num5: case state::num6:
    case state::num7: case state::num8:
    case state::exp1: case state::exp2:
    case state::exp3:
        return parse_number<StackEmpty, 0>(p);

    case state::com1: case state::com2:
    case state::com3: case state::com4:
    case state::com5:
        return parse_comment<StackEmpty, true, false,
            AllowTrailing, AllowBadUTF8>(p);
    }
}

template<class Handler>
template<bool StackEmpty>
const char*
basic_parser<Handler>::
parse_null(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(StackEmpty || st_.empty())
    {
        if(BOOST_JSON_LIKELY(cs.remain() >= 4))
        {
            if(BOOST_JSON_UNLIKELY(
                std::memcmp(cs.begin(), "null", 4) != 0))
                return fail(cs.begin(), error::syntax);
            if(BOOST_JSON_UNLIKELY(
                ! h_.on_null(ec_)))
                return fail(cs.begin());
            cs += 4;
            return cs.begin();
        }
    }
    else
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
    ++cs;
do_nul1:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::nul1);
    if(BOOST_JSON_UNLIKELY(*cs != 'u'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_nul2:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::nul2);
    if(BOOST_JSON_UNLIKELY(*cs != 'l'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_nul3:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::nul3);
    if(BOOST_JSON_UNLIKELY(*cs != 'l'))
        return fail(cs.begin(), error::syntax);
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_null(ec_)))
        return fail(cs.begin());
    ++cs;
    return cs.begin();
}

template<class Handler>
template<bool StackEmpty>
const char*
basic_parser<Handler>::
parse_true(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(StackEmpty || st_.empty())
    {
        if(BOOST_JSON_LIKELY(cs.remain() >= 4))
        {
            if(BOOST_JSON_UNLIKELY(
                std::memcmp(cs.begin(), "true", 4) != 0))
                return fail(cs.begin(), error::syntax);
            if(BOOST_JSON_UNLIKELY(
                ! h_.on_bool(true, ec_)))
                return fail(cs.begin());
            cs += 4;
            return cs.begin();
        }
    }
    else
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
    ++cs;
do_tru1:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::tru1);
    if(BOOST_JSON_UNLIKELY(*cs != 'r'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_tru2:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::tru2);
    if(BOOST_JSON_UNLIKELY(*cs != 'u'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_tru3:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::tru3);
    if(BOOST_JSON_UNLIKELY(*cs != 'e'))
        return fail(cs.begin(), error::syntax);
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_bool(true, ec_)))
        return fail(cs.begin());
    ++cs;
    return cs.begin();
}

template<class Handler>
template<bool StackEmpty>
const char*
basic_parser<Handler>::
parse_false(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(StackEmpty || st_.empty())
    {
        if(BOOST_JSON_LIKELY(cs.remain() >= 5))
        {
            if(BOOST_JSON_UNLIKELY(
                std::memcmp(cs.begin() + 1, "alse", 4) != 0))
                return fail(cs.begin(), error::expected_false);
            if(BOOST_JSON_UNLIKELY(
                ! h_.on_bool(false, ec_)))
                return fail(cs.begin());
            cs += 5;
            return cs.begin();
        }
    }
    else
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
    ++cs;
do_fal1:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::fal1);
    if(BOOST_JSON_UNLIKELY(*cs != 'a'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_fal2:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::fal2);
    if(BOOST_JSON_UNLIKELY(*cs != 'l'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_fal3:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::fal3);
    if(BOOST_JSON_UNLIKELY(*cs != 's'))
        return fail(cs.begin(), error::syntax);
    ++cs;
do_fal4:
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::fal4);
    if(BOOST_JSON_UNLIKELY(*cs != 'e'))
        return fail(cs.begin(), error::syntax);
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_bool(false, ec_)))
        return fail(cs.begin());
    ++cs;
    return cs.begin();
}

//----------------------------------------------------------

template<class Handler>
template<
    bool StackEmpty,
    bool IsKey,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_string(const char* p)
{
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.peek(st);
        switch(st)
        {
        default:
        case state::str1: case state::utf17:
        return parse_unescaped<StackEmpty, 
            IsKey, AllowBadUTF8>(p);
    
        case state::str2: case state::str3: 
        case state::str4: case state::str5: 
        case state::str6: case state::str7:
        case state::sur1: case state::sur2:
        case state::sur3: case state::sur4:
        case state::sur5: case state::sur6:
        case state::utf18:
            return parse_escaped<StackEmpty, 
                IsKey, AllowBadUTF8>(p);
        }
    }
    return parse_unescaped<true, 
        IsKey, AllowBadUTF8>(p);
}

template<class Handler>
template<
    bool StackEmpty,
    bool IsKey,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_unescaped(const char* p)
{
    constexpr auto on_full = IsKey ? 
        &Handler::on_key : &Handler::on_string;
    constexpr auto on_part = IsKey ? 
        &Handler::on_key_part : &Handler::on_string_part;
    detail::const_stream_wrapper cs(p, end_);
    char const* start;
    if(! StackEmpty && ! st_.empty())
    {
        start = cs.begin();
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::str1: goto do_str1;
        case state::utf17: goto do_utf17;
        }
    }
    BOOST_ASSERT(*cs == '\x22'); // '"'
    ++cs;
    start = cs.begin();
do_str1:
    cs = detail::count_valid<AllowBadUTF8>(
        cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
    {
        // call handler if the string isn't empty
        if(BOOST_JSON_LIKELY(cs.begin() > start))
        {
            if(BOOST_JSON_UNLIKELY(! (h_.*on_part)(
                {start, cs.used(start)}, ec_)))
                return fail(cs.begin());
        }
        return maybe_suspend(cs.begin(), state::str1);
    }
    if(BOOST_JSON_UNLIKELY(*cs != '\x22')) // '"'
    {
        if(! AllowBadUTF8 && (*cs & 0x80))
        {
do_utf17:
            cs = validate_utf8<StackEmpty>(cs.begin(), cs.end());
            if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                return suspend_or_fail(state::utf17);
            goto do_str1;
        }
        else if(BOOST_JSON_LIKELY(*cs == '\\'))
        {
            // flush unescaped run from input
            if(BOOST_JSON_LIKELY(cs.begin() > start))
            {
                if(BOOST_JSON_UNLIKELY(! (h_.*on_part)(
                    {start, cs.used(start)}, ec_)))
                    return fail(cs.begin());
            }
            return parse_escaped<StackEmpty, IsKey,
                AllowBadUTF8>(cs.begin());
        }
        // illegal control
        return fail(cs.begin(), error::syntax);
    }
    if(BOOST_JSON_UNLIKELY(! (h_.*on_full)(
        {start, cs.used(start)}, ec_)))
        return fail(cs.begin());
    ++cs;
    return cs.begin();
}

template<class Handler>
template<
    bool StackEmpty, 
    bool IsKey,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_escaped(const char* p)
{
    //---------------------------------------------------------------
    //
    // To handle escapes, a local temporary buffer accumulates
    // the unescaped result. The algorithm attempts to fill the
    // buffer to capacity before invoking the handler.
    // In some cases the temporary buffer needs to be flushed
    // before it is full:
    // * When the closing double quote is seen
    // * When there in no more input (and more is expected later)
    // A goal of the algorithm is to call the handler as few times
    // as possible. Thus, when the first escape is encountered,
    // the algorithm attempts to fill the temporary buffer first.
    //
    constexpr auto on_full = IsKey ? 
        &Handler::on_key : &Handler::on_string;
    constexpr auto on_part = IsKey ? 
        &Handler::on_key_part : &Handler::on_string_part;
    detail::clipped_const_stream cs(p, end_);
    detail::buffer<BOOST_JSON_PARSER_BUFFER_SIZE> temp;
    int32_t digit;
    char c;
    cs.clip(temp.max_size());
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
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
        case state::utf18: goto do_utf18;
        }
    }
    // Unescaped JSON is never larger than its escaped version.
    // To efficiently process only what will fit in the temporary buffer,
    // the size of the input stream is temporarily "clipped" to the size
    // of the temporary buffer.
    // handle escaped character
    BOOST_ASSERT(*cs == '\\');
    ++cs;
do_str3:
    if(BOOST_JSON_UNLIKELY(! cs))
    {
        if(BOOST_JSON_LIKELY(! temp.empty()))
        {
            if(BOOST_JSON_UNLIKELY(
                ! (h_.*on_part)(temp, ec_)))
                return fail(cs.begin());
            temp.clear();
        }
        cs.clip(temp.max_size());
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::str3);
    }
    switch(*cs)
    {
    default:
        return fail(cs.begin(), error::syntax);
    case '\x22': // '"'
        temp.push_back('\x22');
        ++cs;
        break;
    case '\\':
        temp.push_back('\\');
        ++cs;
        break;
    case '/':
        temp.push_back('/');
        ++cs;
        break;
    case 'b':
        temp.push_back('\x08');
        ++cs;
        break;
    case 'f':
        temp.push_back('\x0c');
        ++cs;
        break;
    case 'n':
        temp.push_back('\x0a');
        ++cs;
        break;
    case 'r':
        temp.push_back('\x0d');
        ++cs;
        break;
    case 't':
        temp.push_back('\x09');
        ++cs;
        break;
    case 'u':
        // utf16 escape
        //
        // fast path only when the buffer
        // is large enough for 2 surrogates
        if(BOOST_JSON_LIKELY(cs.remain() > 10))
        {
            // KRYSTIAN TODO: this could be done
            // with fewer instructions
            std::memcpy(&digit, cs.begin() + 1, 4);
            digit = detail::little_endian(digit);
            int d4 = hex_digit(static_cast<
                unsigned char>(digit >> 24));
            int d3 = hex_digit(static_cast<
                unsigned char>(digit >> 16));
            int d2 = hex_digit(static_cast<
                unsigned char>(digit >> 8));
            int d1 = hex_digit(static_cast<
                unsigned char>(digit));
            if(BOOST_JSON_UNLIKELY(
                (d1 | d2 | d3 | d4) == -1))
            {
                if(d1 != -1)
                    ++cs;
                if(d2 != -1)
                    ++cs;
                if(d3 != -1)
                    ++cs;
                return fail(cs.begin(), 
                    error::expected_hex_digit);
            }
            // 32 bit unicode scalar value
            unsigned const u1 =
                (d1 << 12) + (d2 << 8) +
                (d3 << 4) + d4;
            // valid unicode scalar values are
            // [0, D7FF] and [E000, 10FFFF]
            // values within this range are valid utf-8
            // code points and invalid leading surrogates.
            if(BOOST_JSON_LIKELY(
                u1 < 0xd800 || u1 > 0xdfff))
            {
                cs += 5;
                temp.append_utf8(u1);
                break;
            }
            if(BOOST_JSON_UNLIKELY(u1 > 0xdbff))
                return fail(cs.begin(), 
                    error::illegal_leading_surrogate);
            cs += 5;
            // KRYSTIAN TODO: this can be a two byte load
            // and a single comparison. We lose error information,
            // but it's faster.
            if(BOOST_JSON_UNLIKELY(*cs != '\\'))
                return fail(cs.begin(), error::syntax);
            ++cs;
            if(BOOST_JSON_UNLIKELY(*cs != 'u'))
                return fail(cs.begin(), error::syntax);
            ++cs;
            std::memcpy(&digit, cs.begin(), 4);
            digit = detail::little_endian(digit);
            d4 = hex_digit(static_cast<
                unsigned char>(digit >> 24));
            d3 = hex_digit(static_cast<
                unsigned char>(digit >> 16));
            d2 = hex_digit(static_cast<
                unsigned char>(digit >> 8));
            d1 = hex_digit(static_cast<
                unsigned char>(digit));
            if(BOOST_JSON_UNLIKELY(
                (d1 | d2 | d3 | d4) == -1))
            {
                if(d1 != -1)
                    ++cs;
                if(d2 != -1)
                    ++cs;
                if(d3 != -1)
                    ++cs;
                return fail(cs.begin(), 
                    error::expected_hex_digit);
            }
            unsigned const u2 =
                (d1 << 12) + (d2 << 8) +
                (d3 << 4) + d4;
            // valid trailing surrogates are [DC00, DFFF]
            if(BOOST_JSON_UNLIKELY(
                u2 < 0xdc00 || u2 > 0xdfff))
                return fail(cs.begin(), 
                    error::illegal_leading_surrogate);
            cs += 4;
            unsigned cp =
                ((u1 - 0xd800) << 10) +
                    (u2  - 0xdc00) +
                    0x10000;
            // utf-16 surrogate pair
            temp.append_utf8(cp);
            break;
        }
        // flush
        if(BOOST_JSON_LIKELY(! temp.empty()))
        {
            if(BOOST_JSON_UNLIKELY(
                ! (h_.*on_part)(temp, ec_)))
                return fail(cs.begin());
            temp.clear();
            cs.clip(temp.max_size());
        }
        ++cs;
        // utf-16 escape
do_str4:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::str4);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u1_ = digit << 12;
do_str5:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::str5);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u1_ += digit << 8;
do_str6:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::str6);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u1_ += digit << 4;
do_str7:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::str7);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u1_ += digit;
        if(BOOST_JSON_LIKELY(
            u1_ < 0xd800 || u1_ > 0xdfff))
        {
            BOOST_ASSERT(temp.empty());
            // utf-8 codepoint
            temp.append_utf8(u1_);
            break;
        }
        if(BOOST_JSON_UNLIKELY(u1_ > 0xdbff))
            return fail(cs.begin(), 
                error::illegal_leading_surrogate);
do_sur1:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur1);
        if(BOOST_JSON_UNLIKELY(*cs != '\\'))
            return fail(cs.begin(), error::syntax);
        ++cs;
do_sur2:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur2);
        if(BOOST_JSON_UNLIKELY(*cs != 'u'))
            return fail(cs.begin(), error::syntax);
        ++cs;
do_sur3:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur3);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u2_ = digit << 12;
do_sur4:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur4);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u2_ += digit << 8;
do_sur5:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur5);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u2_ += digit << 4;
do_sur6:
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::sur6);
        digit = hex_digit(*cs);
        if(BOOST_JSON_UNLIKELY(digit == -1))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        ++cs;
        u2_ += digit;
        if(BOOST_JSON_UNLIKELY(
            u2_ < 0xdc00 || u2_ > 0xdfff))
            return fail(cs.begin(), 
                error::expected_hex_digit);
        unsigned cp =
            ((u1_ - 0xd800) << 10) +
                (u2_ - 0xdc00) +
                0x10000;
        BOOST_ASSERT(temp.empty());
        // utf-16 surrogate pair
        temp.append_utf8(cp);
    }
do_str2:
    // KRYSTIAN TODO: we can append the characters
    // all at once instead of one at a time
    for(;;)
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        { 
            // flush
            if(BOOST_JSON_LIKELY(! temp.empty()))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! (h_.*on_part)(temp, ec_)))
                    return fail(cs.begin());
                temp.clear();
            }
            cs.clip(temp.max_size());
            if(BOOST_JSON_UNLIKELY(! cs))
                return maybe_suspend(cs.begin(), state::str2);
        }
        c = *cs;
        if(BOOST_JSON_LIKELY(c == '\x22')) // '"'
        {
            if(BOOST_JSON_UNLIKELY(
                ! (h_.*on_full)(temp, ec_)))
                return fail(cs.begin());
            ++cs;
            return cs.begin();
        }
        else if(! AllowBadUTF8 && (c & 0x80))
        {
do_utf18:
            char const* start = cs.begin();
            cs = validate_utf8<StackEmpty>(cs.begin(), cs.end());
            if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                return suspend_or_fail(state::utf18);
            temp.append(start, cs.used(start));
            continue;
        }
        else if(BOOST_JSON_LIKELY(c == '\\'))
        {
            ++cs;
            goto do_str3;
        }
        else if(BOOST_JSON_UNLIKELY(
            is_control(c)))
            return fail(cs.begin(), error::syntax);
        temp.push_back(c);
        ++cs;
    }
}

//----------------------------------------------------------

template<class Handler>
template<
    bool StackEmpty,
    bool AllowComments,
    bool AllowTrailing,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_object(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(! StackEmpty && ! st_.empty())
    {
        // resume
        state st;
        st_.pop(st);
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
        case state::obj8: goto do_obj8;
        case state::obj9: goto do_obj9;
        case state::obj10: goto do_obj10;
        case state::obj11: goto do_obj11;
        }
    }
    BOOST_ASSERT(*cs == '{');
    if(BOOST_JSON_UNLIKELY(! depth_))
        return fail(cs.begin(), error::too_deep);
    --depth_;
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_object_begin(ec_)))
        return fail(cs.begin());
    ++cs;
    // object:
    //     '{' *ws '}'
    //     '{' *ws string *ws ':' *ws value *ws *[ ',' *ws string *ws ':' *ws value *ws ] '}'
do_obj1:
    cs = detail::count_whitespace(cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::obj1);
    if(BOOST_JSON_LIKELY(*cs != '}'))
    {
        if(BOOST_JSON_UNLIKELY(*cs != '\x22'))
        {
            if(AllowComments && *cs == '/')
            {
do_obj2:
                cs = parse_comment<StackEmpty, false, false,
                    AllowTrailing, AllowBadUTF8>(cs.begin());
                if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                    return suspend_or_fail(state::obj2);
                goto do_obj1;
            }
            return fail(cs.begin(), error::syntax);
        }
do_obj3:
        cs = parse_string<StackEmpty, true,
            AllowBadUTF8>(cs.begin());
        if(BOOST_JSON_UNLIKELY(incomplete(cs)))
            return suspend_or_fail(state::obj3);
do_obj4:
        cs = detail::count_whitespace(cs.begin(), cs.end());
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::obj4);
        if(BOOST_JSON_UNLIKELY(*cs != ':'))
        {
            if(AllowComments && *cs == '/')
            {
do_obj5:
                cs = parse_comment<StackEmpty, false, false,
                    AllowTrailing, AllowBadUTF8>(cs.begin());
                if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                    return suspend_or_fail(state::obj5);
                goto do_obj4;
            }
            return fail(cs.begin(), error::syntax);
        }
        ++cs;
do_obj6:
        cs = detail::count_whitespace(cs.begin(), cs.end());
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::obj6);
do_obj7:
        cs = parse_value<StackEmpty, AllowComments, 
            AllowTrailing, AllowBadUTF8>(cs.begin());
        if(BOOST_JSON_UNLIKELY(incomplete(cs)))
            return suspend_or_fail(state::obj7);
do_obj8:
        cs = detail::count_whitespace(cs.begin(), cs.end());
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::obj8);
        if(BOOST_JSON_LIKELY(*cs == ','))
        {
            ++cs;
do_obj9:
            cs = detail::count_whitespace(cs.begin(), cs.end());
            if(BOOST_JSON_UNLIKELY(! cs))
                return maybe_suspend(cs.begin(), state::obj9);

            // loop for next element
            if(BOOST_JSON_LIKELY(*cs == '\x22'))
                goto do_obj3;
            if(! AllowTrailing || *cs != '}')
            {
                if(AllowComments && *cs == '/')
                {
do_obj10:
                    cs = parse_comment<StackEmpty, false, false,
                        AllowTrailing, AllowBadUTF8>(cs.begin());
                    if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                        return suspend_or_fail(state::obj10);
                    goto do_obj9;
                }
                return fail(cs.begin(), error::syntax);
            }
        }
        else if(BOOST_JSON_UNLIKELY(*cs != '}'))
        {
            if(AllowComments && *cs == '/')
            {
do_obj11:
                cs = parse_comment<StackEmpty, false, false,
                    AllowTrailing, AllowBadUTF8>(cs.begin());
                if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                    return suspend_or_fail(state::obj11);
                goto do_obj8;
            }
            return fail(cs.begin(), error::syntax);
        }
        // got closing brace, fall through
    }
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_object_end(ec_)))
        return fail(cs.begin());
    --depth_;
    ++cs;
    return cs.begin();
}

//----------------------------------------------------------

template<class Handler>
template<
    bool StackEmpty,
    bool AllowComments,
    bool AllowTrailing,
    bool AllowBadUTF8>
const char*
basic_parser<Handler>::
parse_array(const char* p)
{
    detail::const_stream_wrapper cs(p, end_);
    if(! StackEmpty && ! st_.empty())
    {
        // resume
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::arr1: goto do_arr1;
        case state::arr2: goto do_arr2;
        case state::arr3: goto do_arr3;
        case state::arr4: goto do_arr4;
        case state::arr5: goto do_arr5;
        case state::arr6: goto do_arr6;
        }
    }
    BOOST_ASSERT(*cs == '[');
    if(BOOST_JSON_UNLIKELY(! depth_))
        return fail(cs.begin(), error::too_deep);
    --depth_;
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_array_begin(ec_)))
        return fail(cs.begin());
    ++cs;
    // array:
    //     '[' *ws ']'
    //     '[' *ws value *ws *[ ',' *ws value *ws ] ']'
do_arr1:
    cs = detail::count_whitespace(cs.begin(), cs.end());
    if(BOOST_JSON_UNLIKELY(! cs))
        return maybe_suspend(cs.begin(), state::arr1);
    if(BOOST_JSON_LIKELY(*cs != ']'))
    {
        if(AllowComments && *cs == '/')
        {
do_arr2:
            cs = parse_comment<StackEmpty, false, false,
                AllowTrailing, AllowBadUTF8>(cs.begin());
            if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                return suspend_or_fail(state::arr2);
            goto do_arr1;
        }
do_arr3:
        // array is not empty, value required
        cs = parse_value<StackEmpty, AllowComments, 
            AllowTrailing, AllowBadUTF8>(cs.begin());
        if(BOOST_JSON_UNLIKELY(incomplete(cs)))
            return suspend_or_fail(state::arr3);
do_arr4:
        cs = detail::count_whitespace(cs.begin(), cs.end());
        if(BOOST_JSON_UNLIKELY(! cs))
            return maybe_suspend(cs.begin(), state::arr4);
        if(BOOST_JSON_LIKELY(*cs == ','))
        {
            ++cs;
do_arr5:
            cs = detail::count_whitespace(cs.begin(), cs.end());
            if(BOOST_JSON_UNLIKELY(! cs))
                return maybe_suspend(cs.begin(), state::arr5);

            // loop for next element
            if(! AllowTrailing || *cs != ']')
                goto do_arr3;
        }
        else if(BOOST_JSON_UNLIKELY(*cs != ']'))
        {
            if(AllowComments && *cs == '/')
            {
do_arr6:
                cs = parse_comment<StackEmpty, false, false,
                    AllowTrailing, AllowBadUTF8>(cs.begin());
                if(BOOST_JSON_UNLIKELY(incomplete(cs)))
                    return suspend_or_fail(state::arr6);
                goto do_arr4;
            }
            return fail(cs.begin(), error::syntax);
        }
        // got closing bracket; fall through
    }
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_array_end(ec_)))
        return fail(cs.begin());
    --depth_;
    ++cs;
    return cs.begin();
}

//----------------------------------------------------------

template<class Handler>
template<bool StackEmpty, char First>
const char*
basic_parser<Handler>::
parse_number(const char* p)
{
    // only one of these will be true if we are not resuming
    // if negative then !zero_first && !nonzero_first
    // if zero_first then !nonzero_first && !negative
    // if nonzero_first then !zero_first && !negative
    constexpr bool negative = First == '-';
    constexpr bool zero_first = First == '0';
    constexpr bool nonzero_first = First == '+';
    detail::const_stream_wrapper cs(p, end_);
    number num;
    const char* begin = cs.begin();
    if(StackEmpty || st_.empty())
    {
        num.bias = 0;
        num.exp = 0;
        num.frac = false;

        //----------------------------------
        //
        // '-'
        // leading minus sign
        //
        BOOST_ASSERT(cs);
        if(negative)
            ++cs;

        num.neg = negative;
        num.frac = false;
        num.exp = 0;
        num.bias = 0;

        // fast path
        if( cs.remain() >= 16 + 1 + 16 ) // digits . digits
        {
            int n1;

            if( nonzero_first || 
                (negative && *cs != '0') )
            {
                n1 = detail::count_digits( cs.begin() );
                BOOST_ASSERT(n1 >= 0 && n1 <= 16);

                if( ! nonzero_first && n1 == 0 )
                {
                    // digit required
                    return fail(cs.begin(), error::syntax);
                }

                num.mant = detail::parse_unsigned( 0, cs.begin(), n1 );

                cs += n1;

                // integer or floating-point with 
                // >= 16 leading digits
                if( n1 == 16 )
                {
                    goto do_num2;
                }
            }
            else
            {
                // 0. floating-point or 0e integer
                num.mant = 0;
                n1 = 0;
                ++cs;
            }

            {
                const char c = *cs;
                if(c != '.')
                {
                    if((c | 32) == 'e')
                    {
                        ++cs;
                        goto do_exp1;
                    }
                    if(negative)
                        num.mant = ~num.mant + 1;
                    goto finish_signed;
                }
            }

            // floating-point number

            ++cs;

            int n2 = detail::count_digits( cs.begin() );
            BOOST_ASSERT(n2 >= 0 && n2 <= 16);

            if( n2 == 0 )
            {
                // digit required
                return fail(cs.begin(), error::syntax);
            }

            // floating-point mantissa overflow
            if( n1 + n2 >= 19 )
            {
                goto do_num4;
            }

            num.mant = detail::parse_unsigned( num.mant, cs.begin(), n2 );

            BOOST_ASSERT(num.bias == 0);

            num.bias -= n2;

            cs += n2;

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
    // DIGIT
    // first digit
    //
do_num1:
    if(zero_first || nonzero_first ||
        BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if(zero_first)
        {
            ++cs;
            num.mant = 0;
            goto do_num6;
        }
        else if(nonzero_first || BOOST_JSON_LIKELY(
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
            return fail(cs.begin(), error::syntax);
        }
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(
            ! h_.on_number_part(
                {begin, cs.used(begin)}, ec_)))
            return fail(cs.begin());
        return maybe_suspend(
            cs.begin(), state::num1, num);
    }

    //----------------------------------
    //
    // 1*DIGIT
    // significant digits left of decimal
    //
do_num2:
    if(negative || (!StackEmpty && num.neg))
    {
        for(;;)
        {
            if(BOOST_JSON_UNLIKELY(! cs))
            {
                if(BOOST_JSON_UNLIKELY(more_))
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h_.on_number_part(
                            {begin, cs.used(begin)}, ec_)))
                        return fail(cs.begin());
                    return suspend(cs.begin(), state::num2, num);
                }
                goto finish_int;
            }
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
    }
    else
    {
        for(;;)
        {
            if(BOOST_JSON_UNLIKELY(! cs))
            {
                if(BOOST_JSON_UNLIKELY(more_))
                {
                    if(BOOST_JSON_UNLIKELY(
                        ! h_.on_number_part(
                            {begin, cs.used(begin)}, ec_)))
                        return fail(cs.begin());
                    return suspend(cs.begin(), state::num2, num);
                }
                goto finish_int;
            }
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
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::num3, num);
            }
            goto finish_dub;
        }
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

    //----------------------------------
    //
    // DIGIT
    // first non-significant digit
    // to the right of decimal
    //
do_num4:
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(
                ! h_.on_number_part(
                    {begin, cs.used(begin)}, ec_)))
                return fail(cs.begin());
            return maybe_suspend(
                cs.begin(), state::num4, num);
        }
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
            return fail(cs.begin(), error::syntax);
        }
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
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::num5, num);
            }
            goto finish_dub;
        }
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

    //----------------------------------
    //
    // [.eE]
    //
do_num6:
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::num6, num);
            }
            goto finish_int;
        }
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

    //----------------------------------
    //
    // DIGIT
    // first significant digit
    // to the right of decimal
    //
do_num7:
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::num7, num);
            }
            // digit required
            return fail(cs.begin(), error::syntax);
        }
        char const c = *cs;
        if(BOOST_JSON_UNLIKELY(
            c < '0' || c > '9'))
        {
            // digit required
            return fail(cs.begin(), error::syntax);
        }
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
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::num8, num);
            }
            goto finish_dub;
        }
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

    //----------------------------------
    //
    // *[+-]
    //
do_exp1:
    if(BOOST_JSON_UNLIKELY(! cs))
    {
        if(BOOST_JSON_UNLIKELY(
            ! h_.on_number_part(
                {begin, cs.used(begin)}, ec_)))
            return fail(cs.begin());
        return maybe_suspend(
            cs.begin(), state::exp1, num);
    }
    if(*cs == '+')
    {
        ++cs;
    }
    else if(*cs == '-')
    {
        ++cs;
        num.frac = true;
    }

    //----------------------------------
    //
    // DIGIT
    // first digit of the exponent
    //
do_exp2:
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::exp2, num);
            }
            // digit required
           return fail(cs.begin(), error::syntax);
        }
        char const c = *cs;
        if(BOOST_JSON_UNLIKELY(
            c < '0' || c > '9'))
        {
            // digit required
            return fail(cs.begin(), error::syntax);
        }
        ++cs;
        num.exp = c - '0';
    }

    //----------------------------------
    //
    // 1*DIGIT
    // subsequent digits in the exponent
    //
do_exp3:
    for(;;)
    {
        if(BOOST_JSON_UNLIKELY(! cs))
        {
            if(BOOST_JSON_UNLIKELY(more_))
            {
                if(BOOST_JSON_UNLIKELY(
                    ! h_.on_number_part(
                        {begin, cs.used(begin)}, ec_)))
                    return fail(cs.begin());
                return suspend(cs.begin(), state::exp3, num);
            }
            goto finish_dub;
        }
        char const c = *cs;
        if(BOOST_JSON_LIKELY(
            c >= '0' && c <= '9'))
        {
            if(BOOST_JSON_UNLIKELY
            //              2147483647 INT_MAX
                (num.exp  > 214748364 || (
                    num.exp == 214748364 && c > '7')))
                return fail(cs.begin(), error::exponent_overflow);
            ++cs;
            num.exp = 10 * num.exp + c - '0';
            continue;
        }
        goto finish_dub;
    }

finish_int:
    if(negative || (!StackEmpty && num.neg))
    {
        if(BOOST_JSON_UNLIKELY(
            ! h_.on_int64(static_cast<
                int64_t>(~num.mant + 1), {begin, cs.used(begin)}, ec_)))
            return fail(cs.begin());
        return cs.begin();
    }
    if(num.mant <= INT64_MAX)
    {
finish_signed:
        if(BOOST_JSON_UNLIKELY(
            ! h_.on_int64(static_cast<
                int64_t>(num.mant), {begin, cs.used(begin)}, ec_)))
            return fail(cs.begin());
        return cs.begin();
    }
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_uint64(num.mant, {begin, cs.used(begin)}, ec_)))
        return fail(cs.begin());
    return cs.begin();
finish_dub:
    double const d = detail::dec_to_float(
        num.mant,
        num.bias + (num.frac ?
            -num.exp : num.exp),
        num.neg);
    if(BOOST_JSON_UNLIKELY(
        ! h_.on_double(d, {begin, cs.used(begin)}, ec_)))
        return fail(cs.begin());
    return cs.begin();
}

//----------------------------------------------------------

template<class Handler>
template<class... Args>
basic_parser<Handler>::
basic_parser(
    parse_options const& opt,
    Args&&... args)
    : h_(std::forward<Args>(args)...)
    , opt_(opt) 
{
}

//----------------------------------------------------------

template<class Handler>
void
basic_parser<Handler>::
reset() noexcept
{
    complete_ = false;
    more_ = true;
    st_.clear();
}

//----------------------------------------------------------

template<class Handler>
std::size_t
basic_parser<Handler>::
write_some(
    bool more,
    char const* data,
    std::size_t size,
    error_code& ec)
{
    ec_ = {};
    more_ = more;
    end_ = data + size;
    const char* p;
    if(BOOST_JSON_LIKELY(
        st_.empty()))
    {
        // first time
        depth_ = max_depth_;
        if(BOOST_JSON_UNLIKELY(
            ! h_.on_document_begin(ec_)))
        {
            ec = ec_;
            return 0;
        }
        p = parse_document<true>(data);
    }
    else
    {
        p = parse_document<false>(data);
    }

    if(BOOST_JSON_LIKELY(p != canary()))
    {
        BOOST_ASSERT(! ec_);
        if(! complete_)
        {
            complete_ = true;
            h_.on_document_end(ec_);
        }
    }
    else
    {
        if(! ec_)
        {
            if(! more_)
            {
                ec_ = error::incomplete;
            }
            else if(! st_.empty())
            {
                // consume as much trailing whitespace in
                // the JSON document as possible, but still
                // consider the parse complete
                state st;
                st_.peek(st);
                if( st == state::doc3 &&
                    ! complete_)
                {
                    complete_ = true;
                    h_.on_document_end(ec_);
                }
            }
        }
        p = end_;
    }
    ec = ec_;
    return p - data;
}

#endif

} // json
} // boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
