//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_BASIC_PARSER_IPP
#define BOOST_JSON_IMPL_BASIC_PARSER_IPP

#include <boost/json/basic_parser.hpp>
#include <boost/json/error.hpp>
#include <boost/json/detail/buffer.hpp>
#include <boost/json/detail/number.hpp>
#include <boost/json/detail/impl/number.ipp>
#include <boost/json/detail/sse2.hpp>

namespace boost {
namespace json {

/*  Reference:

    https://www.json.org/

    RFC 7159: The JavaScript Object Notation (JSON) Data Interchange Format
    https://tools.ietf.org/html/rfc7159
*/

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

void
basic_parser::
parse_element(char_stream& cs)
{
    if(BOOST_JSON_UNLIKELY(! st_.empty()))
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
    parse_white(cs);
    if(BOOST_JSON_UNLIKELY(ec_))
    {
        BOOST_ASSERT(ec_ ==
            error::incomplete);
        if(more_)
            suspend(state::ele1);
        return;
    }
do_ele2:
    parse_value(cs);
    if(BOOST_JSON_UNLIKELY(ec_))
    {
        if(more_ && ec_ ==
            error::incomplete)
            suspend(state::ele2);
        return;
    }
do_ele3:
    parse_white(cs);
    if(BOOST_JSON_UNLIKELY(ec_))
    {
        BOOST_ASSERT(ec_ ==
            error::incomplete);
        if(more_)
        {
            suspend(state::ele3);
            return;
        }
        ec_ = {};
    }
}

void
basic_parser::
parse_white(char_stream& cs)
{
    while(BOOST_JSON_LIKELY(cs))
    {
        char const c = *cs;
        if( c != ' '  &&
            c != '\n' &&
            c != '\r' &&
            c != '\t')
            return;
        ++cs;
    }
    ec_ = error::incomplete;
}

void
basic_parser::
parse_value(char_stream& cs)
{
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        switch(*cs)
        {
        case 'n':
            parse_null(cs);
            break;
        case 't':
            parse_true(cs);
            break;
        case 'f':
            parse_false(cs);
            break;
        case '\x22': // '"'
            parse_string(cs);
            break;
        case '{':
            parse_object(cs);
            break;
        case '[':
            parse_array(cs);
            break;
        default:
            parse_number(cs);
            break;
        }
    }
    else
    {
        state st;
        st_.peek(st);
        switch(st)
        {
        default:
        case state::nul1: case state::nul2:
        case state::nul3:
            parse_null(cs);
            break;

        case state::tru1: case state::tru2:
        case state::tru3:
            parse_true(cs);
            break;

        case state::fal1: case state::fal2:
        case state::fal3: case state::fal4:
            parse_false(cs);
            break;

        case state::str1: case state::str2:
        case state::str3: case state::str4:
        case state::str5: case state::str6:
        case state::str7:
        case state::sur1: case state::sur2:
        case state::sur3: case state::sur4:
        case state::sur5: case state::sur6:
            parse_string(cs);
            break;

        case state::arr1: case state::arr2:
        case state::arr3: case state::arr4:
            parse_array(cs);
            break;
        
        case state::obj1: case state::obj2:
        case state::obj3: case state::obj4:
        case state::obj5: case state::obj6:
        case state::obj7:
            parse_object(cs);
            break;
        
        case state::num1: case state::num2:
        case state::num3: case state::num4:
        case state::num5: case state::num6:
        case state::num7: case state::num8:
        case state::exp1: case state::exp2:
        case state::exp3:
            parse_number(cs);
            break;
        }
    }
}

void
basic_parser::
parse_null(char_stream& cs0)
{
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        BOOST_ASSERT(*cs == 'n');
        if(BOOST_JSON_LIKELY(cs.remain() >= 4))
        {
            if(BOOST_JSON_LIKELY(std::memcmp(
                cs.data(), "null", 4) == 0))
            {
                this->on_null(ec_);
                if(BOOST_JSON_LIKELY(! ec_))
                    cs.skip(4);
                return;
            }
            ec_ = error::syntax;
            return;
        }
        ++cs;
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
do_nul1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'u'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul1);
        ec_ = error::incomplete;
        return;
    }
do_nul2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul2);
        ec_ = error::incomplete;
        return;
    }
do_nul3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::nul3);
        ec_ = error::incomplete;
        return;
    }
    this->on_null(ec_);
}

void
basic_parser::
parse_true(char_stream& cs0)
{
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        BOOST_ASSERT(*cs == 't');
        if(BOOST_JSON_LIKELY(cs.remain() >= 4))
        {
            if(BOOST_JSON_LIKELY(std::memcmp(
                cs.data(), "true", 4) == 0))
            {
                this->on_bool(true, ec_);
                if(BOOST_JSON_LIKELY(! ec_))
                    cs.skip(4);
                return;
            }
            ec_ = error::syntax;
            return;
        }
        ++cs;
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
do_tru1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'r'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru1);
        ec_ = error::incomplete;
        return;
    }
do_tru2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'u'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru2);
        ec_ = error::incomplete;
        return;
    }
do_tru3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'e'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::tru3);
        ec_ = error::incomplete;
        return;
    }
    this->on_bool(true, ec_);
}

void
basic_parser::
parse_false(char_stream& cs0)
{
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        BOOST_ASSERT(*cs == 'f');
        if(BOOST_JSON_LIKELY(cs.remain() >= 5))
        {
            if(BOOST_JSON_LIKELY(std::memcmp(
                cs.data() + 1, "alse", 4) == 0))
            {
                this->on_bool(false, ec_);
                if(BOOST_JSON_LIKELY(! ec_))
                    cs.skip(5);
                return;
            }
            ec_ = error::expected_false;
            return;
        }
        ++cs;
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
do_fal1:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'a'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal1);
        ec_ = error::incomplete;
        return;
    }
do_fal2:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'l'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal2);
        ec_ = error::incomplete;
        return;
    }
do_fal3:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 's'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal3);
        ec_ = error::incomplete;
        return;
    }
do_fal4:
    if(BOOST_JSON_LIKELY(cs))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != 'e'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
    }
    else
    {
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::fal4);
        ec_ = error::incomplete;
        return;
    }
    this->on_bool(false, ec_);
}

//----------------------------------------------------------

void
basic_parser::
parse_string(char_stream& cs0)
{
    detail::local_char_stream cs(cs0);
    detail::buffer<BOOST_JSON_PARSER_BUFFER_SIZE> temp;
    auto const flush =
        [this, &temp]()
        {
            if(temp.empty())
                return false;
            if(is_key_)
                on_key_part(temp, ec_);
            else
                on_string_part(temp, ec_);
            temp.clear();
            return true;
        };
    char const* start;
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        if(BOOST_JSON_UNLIKELY(
            *cs != '\x22')) // '"'
        {
            ec_ = error::syntax;
            return;
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
                    this->on_key({ start,
                        cs.used(start)}, ec_);
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                    is_key_ = false;
                }
                else
                {
                    this->on_string({ start,
                        cs.used(start)}, ec_);
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                }
                ++cs;
                return;
            }
            else if(BOOST_JSON_LIKELY(
                c == '\\'))
            {
                if(BOOST_JSON_LIKELY(
                    cs.data() > start))
                {
                    if(is_key_)
                        this->on_key_part({start,
                            cs.used(start)}, ec_);
                    else
                        this->on_string_part({start,
                            cs.used(start)}, ec_);
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                }
                goto do_str2;
            }
            else if(BOOST_JSON_UNLIKELY(
                is_control(c)))
            {
                // invalid character
                ec_ = error::syntax;
                return;
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
                    this->on_key_part({start,
                        cs.used(start)}, ec_);
                else
                    this->on_string_part({start,
                        cs.used(start)}, ec_);
                if(BOOST_JSON_UNLIKELY(ec_))
                    return;
            }
            suspend(state::str1);
        }
        ec_ = error::incomplete;
        return;
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
                    this->on_key(temp, ec_);
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                    is_key_ = false;
                }
                else
                {
                    this->on_string(temp, ec_);
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                }
                ++cs;
                return;
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
                return;
            }
            temp.push_back(*cs);
            ++cs;
            continue;
        }
        if( BOOST_JSON_LIKELY(flush()) &&
            BOOST_JSON_UNLIKELY(ec_))
            return;
        cs.clip(temp.capacity());
        if(cs)
            continue;
        if(BOOST_JSON_LIKELY(more_))
            suspend(state::str2);
        ec_ = error::incomplete;
        return;

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
                    int d1 = hex_digit(cs[1]);
                    int d2 = hex_digit(cs[2]);
                    int d3 = hex_digit(cs[3]);
                    int d4 = hex_digit(cs[4]);
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
                        return;
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
                        return;
                    }
                    cs.skip(5);
                    if(BOOST_JSON_UNLIKELY(*cs != '\\'))
                    {
                        ec_ = error::syntax;
                        return;
                    }
                    ++cs;
                    if(BOOST_JSON_UNLIKELY(*cs != 'u'))
                    {
                        ec_ = error::syntax;
                        return;
                    }
                    ++cs;
                    d1 = hex_digit(cs[0]);
                    d2 = hex_digit(cs[1]);
                    d3 = hex_digit(cs[2]);
                    d4 = hex_digit(cs[3]);
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
                        return;
                    }
                    unsigned const u2 =
                        (d1 << 12) + (d2 << 8) +
                        (d3 << 4) + d4;
                    if(BOOST_JSON_UNLIKELY(
                        u2 < 0xdc00 || u2 > 0xdfff))
                    {
                        ec_ = error::illegal_trailing_surrogate;
                        return;
                    }
                    cs.skip(4);
                    unsigned cp =
                        ((u1 - 0xd800) << 10) +
                         (u2  - 0xdc00) +
                         0x10000;
                    temp.append_utf8(cp);
                    continue;
                }
                if( BOOST_JSON_LIKELY(flush()))
                {
                    if(BOOST_JSON_UNLIKELY(ec_))
                        return;
                    cs.clip(temp.capacity());
                }
                ++cs;
                goto do_str4;

            default:
                ec_ = error::syntax;
                return;
            }
            ++cs;
            continue;
        }
        if(BOOST_JSON_LIKELY(more_))
        {
            if( BOOST_JSON_LIKELY(flush()) &&
                BOOST_JSON_UNLIKELY(ec_))
                return;
            suspend(state::str3);
        }
        ec_ = error::incomplete;
        return;

        // utf16 escape
    do_str4:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u1_ = d << 12;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str4);
            ec_ = error::incomplete;
            return;
        }
    do_str5:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u1_ += d << 8;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str5);
            ec_ = error::incomplete;
            return;
        }
    do_str6:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u1_ += d << 4;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str6);
            ec_ = error::incomplete;
            return;
        }
    do_str7:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u1_ += d;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::str7);
            ec_ = error::incomplete;
            return;
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
            return;
        }
    do_sur1:
        if(BOOST_JSON_LIKELY(cs))
        {
            if(BOOST_JSON_UNLIKELY(*cs != '\\'))
            {
                ec_ = error::syntax;
                return;
            }
            ++cs;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur1);
            ec_ = error::incomplete;
            return;
        }
    do_sur2:
        if(BOOST_JSON_LIKELY(cs))
        {
            if(BOOST_JSON_UNLIKELY(*cs != 'u'))
            {
                ec_ = error::syntax;
                return;
            }
            ++cs;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur2);
            ec_ = error::incomplete;
            return;
        }
    do_sur3:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u2_ = d << 12;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur3);
            ec_ = error::incomplete;
            return;
        }
    do_sur4:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u2_ += d << 8;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur4);
            ec_ = error::incomplete;
            return;
        }
    do_sur5:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u2_ += d << 4;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur5);
            ec_ = error::incomplete;
            return;
        }
    do_sur6:
        if(BOOST_JSON_LIKELY(cs))
        {
            int const d = hex_digit(*cs);
            if(d == -1)
            {
                ec_ = error::expected_hex_digit;
                return;
            }
            ++cs;
            u2_ += d;
        }
        else
        {
            if(BOOST_JSON_LIKELY(more_))
                suspend(state::sur6);
            ec_ = error::incomplete;
            return;
        }
        if(BOOST_JSON_UNLIKELY(
            u2_ < 0xdc00 || u2_ > 0xdfff))
        {
            ec_ = error::illegal_trailing_surrogate;
            return;
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

void
basic_parser::
parse_object(char_stream& cs0)
{
    char c;
    std::size_t n;
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        BOOST_ASSERT(*cs == '{');
        if(depth_ >= max_depth_)
        {
            ec_ = error::too_deep;
            return;
        }
        this->on_object_begin(ec_);
        if(BOOST_JSON_UNLIKELY(ec_))
            return;
        ++depth_;
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
    parse_white(cs);
    if(BOOST_JSON_UNLIKELY(ec_))
    {
        BOOST_ASSERT(ec_ ==
            error::incomplete);
        if(more_)
            suspend(state::obj1, n);
        return;
    }
    c = *cs;
    if(c == '}')
    {
        this->on_object_end(n, ec_);
        --depth_;
        ++cs;
        return;
    }
    for(;;)
    {
        is_key_ = true;
do_obj2:
        parse_string(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            if(more_ && ec_ ==
                error::incomplete)
                suspend(state::obj2, n);
            return;
        }
do_obj3:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::obj3, n);
            return;
        }
        if(BOOST_JSON_UNLIKELY(*cs != ':'))
        {
            ec_ = error::syntax;
            return;
        }
        ++cs;
do_obj4:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::obj4, n);
            return;
        }
do_obj5:
        parse_value(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            if(more_ && ec_ ==
                error::incomplete)
                suspend(state::obj5, n);
            return;
        }
        ++n;
do_obj6:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::obj6, n);
            return;
        }
        if(BOOST_JSON_UNLIKELY(*cs != ','))
        {
            if(BOOST_JSON_LIKELY(*cs == '}'))
            {
                this->on_object_end(n, ec_);
                --depth_;
                ++cs;
                return;
            }
            ec_ = error::syntax;
            return;
        }
        ++cs;
do_obj7:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::obj7, n);
            return;
        }
    }
}

//----------------------------------------------------------

void
basic_parser::
parse_array(char_stream& cs0)
{
    char c;
    std::size_t n;
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        BOOST_ASSERT(*cs == '[');
        if(depth_ >= max_depth_)
        {
            ec_ = error::too_deep;
            return;
        }
        this->on_array_begin(ec_);
        if(BOOST_JSON_UNLIKELY(ec_))
            return;
        ++depth_;
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
    parse_white(cs);
    if(BOOST_JSON_UNLIKELY(ec_))
    {
        BOOST_ASSERT(ec_ ==
            error::incomplete);
        if(more_)
            suspend(state::arr1, n);
        return;
    }
    c = *cs;
    if(c == ']')
    {
        this->on_array_end(n, ec_);
        --depth_;
        ++cs;
        return;
    }
    for(;;)
    {
do_arr2:
        parse_value(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            if(more_ && ec_ ==
                error::incomplete)
                suspend(state::arr2, n);
            return;
        }
        ++n;
do_arr3:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::arr3, n);
            return;
        }
        if(BOOST_JSON_UNLIKELY(*cs != ','))
        {
            if(BOOST_JSON_LIKELY(*cs == ']'))
            {
                this->on_array_end(n, ec_);
                --depth_;
                ++cs;
                return;
            }
            ec_ = error::syntax;
            return;
        }
        ++cs;
do_arr4:
        parse_white(cs);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            BOOST_ASSERT(ec_ ==
                error::incomplete);
            if(more_)
                suspend(state::arr4, n);
            return;
        }
    }
}

//----------------------------------------------------------

void
basic_parser::
parse_number(char_stream& cs0)
{
    number num;
    detail::local_char_stream cs(cs0);
    if(BOOST_JSON_LIKELY(st_.empty()))
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
            return;
        }
    }
    else
    {
        if(more_)
            suspend(state::num1, num);
        ec_ = error::incomplete;
        return;
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
                ec_ = error::incomplete;
                return;
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
                    ec_ = error::incomplete;
                    return;
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
                ec_ = error::incomplete;
                return;
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
            return;
        }
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
            suspend(state::num4, num);
        ec_ = error::incomplete;
        return;
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
                ec_ = error::incomplete;
                return;
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
            ec_ = error::incomplete;
            return;
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
            return;
        }
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::num7, num);
            ec_ = error::incomplete;
            return;
        }
        // digit required
        ec_ = error::syntax;
        return;
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
                ec_ = error::incomplete;
                return;
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
        ec_ = error::incomplete;
        return;
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
            return;
        }
        ++cs;
        num.exp = c - '0';
    }
    else
    {
        if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::exp2, num);
            ec_ = error::incomplete;
            return;
        }
        // digit required
        ec_ = error::syntax;
        return;
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
                    return;
                }
                ++cs;
                num.exp = 10 * num.exp + c - '0';
                continue;
            }
        }
        else if(BOOST_JSON_UNLIKELY(more_))
        {
            suspend(state::exp3, num);
            ec_ = error::incomplete;
            return;
        }
        goto finish_dub;
    }

finish_int:
    if(num.neg)
        this->on_int64(static_cast<
            int64_t>(~num.mant + 1), ec_);
    else if(num.mant <= INT64_MAX)
        this->on_int64(static_cast<
            int64_t>(num.mant), ec_);
    else
        this->on_uint64(num.mant, ec_);
    return;

finish_dub:
    double d = detail::dec_to_float(
        num.mant,
        num.bias + (num.frac ?
            -num.exp : num.exp),
        num.neg);
    this->on_double(d, ec_);
}

//----------------------------------------------------------

basic_parser::
basic_parser()
    : done_(false)
    , more_(true)
{
}

void
basic_parser::
reset() noexcept
{
    done_ = false;
    more_ = true;
    st_.clear();
}

//----------------------------------------------------------

std::size_t
basic_parser::
write_some(
    char const* const data,
    std::size_t const size,
    error_code& ec)
{
    // If this goes off, it means you forgot to
    // check is_done() before presenting more data
    // to the parser.
    BOOST_ASSERT(! done_);

    ec_ = {};
    if(BOOST_JSON_LIKELY(st_.empty()))
    {
        // first time
        depth_ = 0;
        is_key_ = false;
        this->on_document_begin(ec_);
        if(BOOST_JSON_UNLIKELY(ec_))
        {
            ec = ec_;
            return 0;
        }
    }
    char_stream cs = { data, size };
    parse_element(cs);
    if(BOOST_JSON_LIKELY(! ec_))
    {
        ec = {};
        done_ = true;
    }
    else if( more_ && ec_ ==
        error::incomplete)
    {
        ec = {};
    }
    else
    {
        ec = ec_;
    }
    return cs.data() - data;
}

std::size_t
basic_parser::
write_some(
    char const* const data,
    std::size_t const size)
{
    error_code ec;
    auto const n =
        write_some(data, size, ec);
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
    return n;
}

//----------------------------------------------------------

void
basic_parser::
write(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    auto n = write_some(data, size, ec);
    if(! ec)
    {
        if(n < size)
            ec = error::extra_data;
    }
}

void
basic_parser::
write(
    char const* data,
    std::size_t size)
{
    error_code ec;
    write(data, size, ec);
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
}

//----------------------------------------------------------

void
basic_parser::
finish(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    more_ = false;
    write(data, size, ec);
    if(ec)
        return;
    BOOST_ASSERT(done_);
    this->on_document_end(ec);
}

void
basic_parser::
finish(
    char const* data,
    std::size_t size)
{
    error_code ec;
    finish(data, size, ec);
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
}

//----------------------------------------------------------

void
basic_parser::
finish(error_code& ec)
{
    finish(nullptr, 0, ec);
}

void
basic_parser::
finish()
{
    error_code ec;
    finish(ec);
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
}

} // json
} // boost

#endif
