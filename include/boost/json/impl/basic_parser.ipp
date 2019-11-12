//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
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
#include <boost/json/detail/assert.hpp>
#include <boost/json/detail/buffer.hpp>
#include <boost/json/detail/sse2.hpp>

namespace boost {
namespace json {

/*  References:

    https://www.json.org/

    RFC 7159: The JavaScript Object Notation (JSON) Data Interchange Format
    https://tools.ietf.org/html/rfc7159

    character
        '0020' . '10ffff' - '"' - '\'
        '\' escape

    escape
        '"' '\' '/' 'b'
        'f' 'n' 'r' 't'
        'u' hex hex hex hex

    hex
        digit
        'A' . 'F'
        'a' . 'f'

    number
        int frac exp

    ws
        ""
        '0009' ws
        '000A' ws
        '000D' ws
        '0020' ws
*/

enum class basic_parser::state : char
{
    end = 0, // must be 0
    begin,
    maybe_end,

    val,
    obj1, obj2, obj3,
    arr1, arr2,
    str0, str1, str2, str3, str4,
    esc1, esc2, esc3, esc4,
    sur1, sur2, sur3, 
    num,
    lit,
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
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    return -1;
}

//----------------------------------------------------------

basic_parser::
basic_parser()
{
    st_.push(state::begin);
}

void
basic_parser::
reset() noexcept
{
    st_.clear();
    st_.push(state::begin);
}

//----------------------------------------------------------

std::size_t
basic_parser::
write_some(
    char const* const data,
    std::size_t const size,
    error_code& ec)
{
    auto p = data;
    auto const p0 = p;
    auto const p1 = p0 + size;
    detail::buffer<2048> temp;
    ec = {};

    auto const maybe_flush =
        [this, &temp](error_code& ec)
        {
            // need 4 chars for largest utf8 code point
            if(temp.max_size() - temp.size() >= 4)
                return true;
            if(is_key_)
                this->on_key_part(temp, ec);
            else
                this->on_string_part(temp, ec);
            temp.clear();
            if(ec)
                return false;
            return true;
        };

    // `true` if p < p1
    auto const skip_white =
        [&p, &p1]
        {
            while(p < p1)
            {
                if( *p == ' ' ||
                    *p == '\t' ||
                    *p == '\r' ||
                    *p == '\n')
                {
                    ++p;
                    continue;
                }
                return true;
            }
            return false;
        };

loop:
    switch(*st_)
    {
    case state::begin:
        u0_ = -1;
        depth_ = 0;
        is_key_ = false;
        this->on_document_begin(ec);
        if(ec)
            goto yield;
        *st_ = state::maybe_end;
        st_.push(state::val);
        goto loop_val;

    case state::end:
        ec = error::illegal_extra_chars;
        goto yield;

    case state::maybe_end:
        if(! skip_white())
            goto yield;
        write_eof(ec);
        goto yield;

    //------------------------------------------------------

    case state::val:
loop_val:
        if(! skip_white())
            goto yield;
        switch(*p)
        {
        // object
        case '{':
            if(depth_ >= max_depth_)
            {
                ec = error::too_deep;
                goto yield;
            }
            st_.pop();
            this->on_object_begin(ec);
            if(ec)
                goto yield;
            ++p;
            ++depth_;
            st_.push(state::obj1);
            goto loop_obj;

        // array
        case '[':
            if(depth_ >= max_depth_)
            {
                ec = error::too_deep;
                goto yield;
            }
            st_.pop();
            this->on_array_begin(ec);
            if(ec)
                goto yield;
            ++p;
            ++depth_;
            st_.push(state::arr1);
            goto loop_arr;

        // string
        case '"':
            *st_ = state::str1;
            goto loop_str1;

        // true
        case 't':
            if(p + 4 <= p1)
            {
                if( p[1] == 'r' &&
                    p[2] == 'u' &&
                    p[3] == 'e')
                {
                    this->on_bool(true, ec);
                    if(ec)
                        goto yield;
                    p = p + 4;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_true;
                goto yield;
            }
            ++p;
            lit_ = "rue";
            ev_ = error::expected_true;
            *st_ = state::lit;
            goto loop;

        // false
        case 'f':
            if(p + 5 <= p1)
            {
                if( p[1] == 'a' &&
                    p[2] == 'l' &&
                    p[3] == 's' &&
                    p[4] == 'e')
                {
                    this->on_bool(false, ec);
                    if(ec)
                        goto yield;
                    p = p + 5;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_false;
                goto yield;
            }
            ++p;
            lit_ = "alse";
            ev_ = error::expected_false;
            *st_ = state::lit;
            goto loop;

        // null
        case 'n':
            if(p + 4 <= p1)
            {
                if( p[1] == 'u' &&
                    p[2] == 'l' &&
                    p[3] == 'l')
                {
                    this->on_null(ec);
                    if(ec)
                        goto yield;
                    p = p + 4;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_null;
                goto yield;
            }
            ++p;
            lit_ = "ull";
            ev_ = error::expected_null;
            *st_ = state::lit;
            goto loop;

        default:
            if(iep_.maybe_init(*p))
            {
                ++p;
                *st_ = state::num;
                goto loop_num;
            }
            ec = error::illegal_char;
            goto yield;
        }
        goto yield;

    //------------------------------------------------------

    //
    // object
    //

    // first key or end of object
    case state::obj1:
loop_obj:
        if(! skip_white())
            goto yield;
        if(*p == '}')
        {
            --depth_;
            st_.pop();
            this->on_object_end(ec);
            if(ec)
                goto yield;
            ++p;
            goto loop;
        }
        *st_ = state::obj3;
        st_.push(state::obj2);
        st_.push(state::str1);
        is_key_ = true;
        goto loop;

    case state::obj2:
        if(! skip_white())
            goto yield;
        if(*p != ':')
        {
            ec = error::expected_colon;
            goto yield;
        }
        ++p;
        *st_ = state::val;
        goto loop_val;

    case state::obj3:
        if(! skip_white())
            goto yield;
        if(*p == '}')
        {
            --depth_;
            st_.pop();
            this->on_object_end(ec);
            if(ec)
                goto yield;
            ++p;
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto yield;
        }
        ++p;
        is_key_ = true;
        st_.push(state::obj2);
        st_.push(state::str0);
        goto loop_str0;

    //------------------------------------------------------

    //
    // array
    //

    case state::arr1:
loop_arr:
        if(! skip_white())
            goto yield;
        if(*p == ']')
        {
            --depth_;
            st_.pop();
            this->on_array_end(ec);
            if(ec)
                goto yield;
            ++p;
            goto loop;
        }
        *st_ = state::arr2;
        st_.push(state::val);
        goto loop;

    case state::arr2:
        if(! skip_white())
            goto yield;
        if(*p == ']')
        {
            --depth_;
            st_.pop();
            this->on_array_end(ec);
            if(ec)
                goto yield;
            ++p;
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto yield;
        }
        ++p;
        st_.push(state::val);
        goto loop;

    //------------------------------------------------------

    //
    // string
    //

    case state::str0:
loop_str0:
        if(! skip_white())
            goto yield;
        *st_ = state::str1;
        BOOST_FALLTHROUGH;

    // string, opening quotes
    case state::str1:
loop_str1:
        BOOST_JSON_ASSERT(p < p1);
        if(*p != '\"')
        {
            ec = error::expected_quotes;
            goto yield;
        }
        ++p;
        *st_ = state::str2;
        BOOST_FALLTHROUGH;

    // string, no-copy loop
    case state::str2:
    {
        if(p >= p1)
            goto yield;
        auto const start = p;
        while(p < p1)
        {
            p += detail::count_unescaped(p, p1 - p);
            if(*p == '\"')
            {
                if(is_key_)
                    this->on_key({start,
                        static_cast<std::size_t>(
                            p - start)}, ec);
                else
                    this->on_string({start,
                        static_cast<std::size_t>(
                            p - start)}, ec);
                if(ec)
                    goto yield;
                ++p;
                is_key_ = false;
                st_.pop();
                goto loop;
            }
            if(*p == '\\')
            {
                if(p > start)
                {
                    if(is_key_)
                        this->on_key_part({start,
                            static_cast<std::size_t>(
                                p - start)}, ec);
                    else
                        this->on_string_part({start,
                            static_cast<std::size_t>(
                                p - start)}, ec);
                }
                if(ec)
                    goto yield;
                ++p;
                *st_ = state::str4;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::illegal_control_char;
                goto yield;
            }
            // VFALCO UTF-8 validation here?
            ++p;
        }
        BOOST_JSON_ASSERT(p != start);
        if(is_key_)
            this->on_key_part({start,
                static_cast<std::size_t>(
                    p - start)}, ec);
        else
            this->on_string_part({start,
                static_cast<std::size_t>(
                    p - start)}, ec);
        if(ec)
            goto yield;
        goto yield;
    }

    // string
    // handles escapes
    case state::str3:
    {
        while(p < p1)
        {
            if(*p == '\"')
            {
                if(is_key_)
                    this->on_key(temp, ec);
                else
                    this->on_string(temp, ec);
                if(ec)
                    goto yield;
                ++p;
                st_.pop();
                temp.clear();
                is_key_ = false;
                goto loop;
            }
            if(*p == '\\')
            {
                ++p;
                *st_ = state::str4;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::illegal_control_char;
                goto yield;
            }
            // VFALCO We can move this check to
            // an outer loop by calculating
            // (p1 - p) / 4
            //
            if(! maybe_flush(ec))
                goto yield;
            // VFALCO could batch this with memcpy 
            temp.push_back(*p++);
        }
        goto yield;
    }

    // char escape
    case state::str4:
        if(p >= p1)
            goto yield;
        switch(*p)
        {
        case '\"':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\"');
            break;

        case '\\':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\\');
            break;

        case '/':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('/');
            break;

        case 'b':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\x08');
            break;

        case 'f':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\x0c');
            break;

        case 'n':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\x0a');
            break;

        case 'r':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\x0d');
            break;

        case 't':
            if(! maybe_flush(ec))
                goto yield;
            temp.push_back('\x09');
            break;

        case 'u':
            ++p;
            *st_ = state::str3;
            st_.push(state::esc1);
            goto loop;

        default:
            ec = error::illegal_escape_char;
            goto yield;
        }
        ++p;
        *st_ = state::str3;
        goto loop;

    //----------------------------------

    // utf16 escape, got "\u" already
    case state::esc1:
    {
        if(p >= p1)
            goto yield;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto yield;
        }
        u_ = d << 12;
        if(p + 3 <= p1)
        {
            // fast path
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto yield;
            }
            u_ += d << 8;
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto yield;
            }
            u_ += d << 4;
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto yield;
            }
            u_ += d;
            *st_ = state::sur1;
            goto loop;
        }
        *st_ = state::esc2;
        goto loop;
    }

    case state::esc2:
    {
        if(p >= p1)
            goto yield;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto yield;
        }
        u_ += d << 8;
        *st_ = state::esc3;
        goto loop;
    }

    case state::esc3:
    {
        if(p >= p1)
            goto yield;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto yield;
        }
        u_ += d << 4;
        *st_ = state::esc4;
        goto loop;
    }

    case state::esc4:
    {
        if(p >= p1)
            goto yield;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto yield;
        }
        u_ += d;
        *st_ = state::sur1;
        goto loop;
    }
    
    //----------------------------------

    // handles 1 or 2 surrogates
    case state::sur1:
    {
        // one code unit
        if(u0_ == -1)
        {
            if( u_ >= 0xd800)
            {
                if(u_ <=  0xdbff)
                {
                    // need 2nd surrogate
                    u0_ = u_;
                    *st_ = state::sur2;
                    goto loop;
                }
                else if(u_ <= 0xdfff)
                {
                    ec = error::illegal_leading_surrogate;
                    goto yield;
                }
            }

            if(! maybe_flush(ec))
                goto yield;
            temp.append_utf8(u_);
            st_.pop();
            goto loop;
        }
        // both code units
        if( u_ <  0xdc00 ||
            u_ >  0xdfff)
        {
            ec = error::illegal_trailing_surrogate;
            goto yield;
        }
        unsigned long cp =
            ((u0_ - 0xd800) << 10) +
             (u_  - 0xdc00);
        temp.append_utf8(cp);
        u0_ = -1;
        st_.pop();
        goto loop;
    }

    // second utf16 surrogate
    case state::sur2:
        if(p >= p1)
            goto yield;
        if(*p != '\\')
        {
            ec = error::expected_utf16_escape;
            goto yield;
        }
        ++p;
        *st_ = state::sur3;
        goto loop;
    
    case state::sur3:
        if(p >= p1)
            goto yield;
        if(*p != 'u')
        {
            ec = error::expected_utf16_escape;
            goto yield;
        }
        ++p;
        *st_ = state::esc1;
        goto loop;

    //------------------------------------------------------

    // number

    case state::num:
    {
loop_num:
        if(p >= p1)
            goto yield;
        p += iep_.write_some(
            p, p1 - p, ec);
        if(ec)
            goto yield;
        // VFALCO number_parser needs to handle
        //        is_done inside write_some better
        if(p < p1)
        {
            iep_.write_eof(ec);
            if(ec)
                goto yield;
            BOOST_JSON_ASSERT(iep_.is_done());
            auto const num = iep_.get();
            switch(num.kind)
            {
            default:
            case kind::int64:
                this->on_int64(num.i, ec);
                break;
            case kind::uint64:
                this->on_uint64(num.u, ec);
                break;
            case kind::double_:
                this->on_double(num.d, ec);
                break;
            }
            if(ec)
                goto yield;
            st_.pop();
            goto loop;
        }
        goto yield;
    }

    //------------------------------------------------------

    // string literal (true, false, null)

    case state::lit:
        BOOST_JSON_ASSERT(lit_ != nullptr);
        while(p < p1)
        {
            if(*p != *lit_)
            {
                ec = ev_;
                goto yield;
            }
            ++p;
            if(*++lit_ == 0)
            {
                st_.pop();
                goto loop;
            }
        }
        goto yield;

    //------------------------------------------------------

    }
    // never get here
yield:
    return p - p0;
}

//----------------------------------------------------------

// Called to parse the rest of the document, this
// can be optimized by assuming no more data is coming.
std::size_t
basic_parser::
write(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    auto n =
        write_some(data, size, ec);
    if(! ec)
    {
        if(n < size)
            n += write_some(
                data + n, size - n, ec);
    }
    if(! ec)
        write_eof(ec);
    return n;
}

//----------------------------------------------------------

void
basic_parser::
write_eof(error_code& ec)
{
    for(;;)
    {
        // pop all states that
        // allow "" (empty string)
        switch(*st_)
        {
        case state::end:
            ec = {};
            return;

        case state::begin:
            ec = error::syntax;
            return;

        case state::maybe_end:
            this->on_document_end(ec);
            if(ec)
                return;
            *st_ = state::end;
            break;

        case state::num:
        {
            iep_.write_eof(ec);
            if(ec)
                return;
            auto const num = iep_.get();
            switch(num.kind)
            {
            default:
            case kind::int64:
                this->on_int64(num.i, ec);
                break;
            case kind::uint64:
                this->on_uint64(num.u, ec);
                break;
            case kind::double_:
                this->on_double(num.d, ec);
                break;
            }
            if(ec)
                return;
            st_.pop();
            break;
        }

        case state::lit:
            ec = ev_;
            return;

        case state::val:
        case state::obj1:
        case state::obj2:
        case state::obj3:
        case state::arr1:
        case state::arr2:
        case state::str0:
        case state::str1:
        case state::str2:
        case state::str3:
        case state::str4:
        case state::esc1:
        case state::esc2:
        case state::esc3:
        case state::esc4:
        case state::sur2:
        case state::sur3:
        case state::sur1:
            ec = error::syntax;
            return;
        }
    }
}

} // json
} // boost

#endif
