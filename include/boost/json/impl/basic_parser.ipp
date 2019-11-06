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
        '"'
        '\'
        '/'
        'b'
        'f'
        'n'
        'r'
        't'
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
    ws,

    value,

    object1, object2, colon,
    array1,  array2,
    string1, string2, string3, string4,
    true1,   true2,   true3,
    false1,  false2,  false3,  false4,
    null1,   null2,   null3,

    u_esc1,  u_esc2,  u_esc3,  u_esc4,
    u_pair1, u_pair2,
    u_surr,

    num,

    done0, done
};

//----------------------------------------------------------

basic_parser::
basic_parser()
{
}

void
basic_parser::
reset() noexcept
{
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
    auto p = data;
    auto const p0 = p;
    auto const p1 = p0 + size;
    detail::buffer<2048> temp;
    ec = {};

    auto const maybe_flush =
        [this, &temp](error_code& ec)
        {
            // need 4 chars for largest utf8 code point
            if(temp.size() < temp.max_size() - 4)
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

    // begin document
    if(st_.empty())
    {
        u0_ = -1;
        depth_ = 1;
        is_key_ = false;
        this->on_document_begin(ec);
        if(ec)
            goto finish;
        st_.push(state::done0);
        st_.push(state::ws);
        st_.push(state::value);
        st_.push(state::ws);
        goto loop_ws;
    }

loop:
    switch(st_.front())
    {
    case state::ws:
loop_ws:
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
            st_.pop();
            goto loop;
        }
        break;

    case state::value:
    {
        BOOST_JSON_ASSERT(p < p1);
        switch(*p)
        {
        // object
        case '{':
            if(depth_ > max_depth_)
            {
                ec = error::too_deep;
                goto finish;
            }
            ++depth_;
            ++p;
            st_.front() = state::object1;
            st_.push(state::ws);
            this->on_object_begin(ec);
            if(ec)
                goto finish;
            //prepare_stack(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // array
        case '[':
            if(depth_ > max_depth_)
            {
                ec = error::too_deep;
                goto finish;
            }
            ++depth_;
            ++p;
            st_.front() = state::array1;
            st_.push(state::ws);
            this->on_array_begin(ec);
            if(ec)
                goto finish;
            //prepare_stack(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // string
        case '"':
            st_.front() = state::string1;
            goto loop_string;

        // true
        case 't':
            if(p + 4 <= p1)
            {
                if( p[1] == 'r' &&
                    p[2] == 'u' &&
                    p[3] == 'e')
                {
                    p = p + 4;
                    this->on_bool(true, ec);
                    if(ec)
                        goto finish;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_true;
                goto finish;
            }
            ++p;
            st_.front() = state::true1;
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
                    p = p + 5;
                    this->on_bool(false, ec);
                    if(ec)
                        goto finish;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_false;
                goto finish;
            }
            ++p;
            st_.front() = state::false1;
            goto loop;

        // null
        case 'n':
            if(p + 4 <= p1)
            {
                if( p[1] == 'u' &&
                    p[2] == 'l' &&
                    p[3] == 'l')
                {
                    p = p + 4;
                    this->on_null(ec);
                    if(ec)
                        goto finish;
                    st_.pop();
                    goto loop;
                }
                ec = error::expected_null;
                goto finish;
            }
            ++p;
            st_.front() = state::null1;
            goto loop;

        default:
            if(iep_.maybe_init(*p))
            {
                ++p;
                st_.front() = state::num;
                goto loop_num;
            }
            ec = error::illegal_char;
            goto finish;
        }
        break;
    }

    //------------------------------------------------------
    //
    // object
    //

    // first key or end of object
    case state::object1:
        BOOST_JSON_ASSERT(p < p1);
        if(*p == '}')
        {
            ++p;
            this->on_object_end(ec);
            if(ec)
                goto finish;
            --depth_;
            st_.pop();
            goto loop;
        }
        st_.pop();
        st_.push(state::object2);
        st_.push(state::ws);
        st_.push(state::value);
        st_.push(state::ws);
        st_.push(state::colon);
        st_.push(state::ws);
        st_.push(state::string1);
        is_key_ = true;
        goto loop;

    case state::object2:
        BOOST_JSON_ASSERT(p < p1);
        if(*p == '}')
        {
            ++p;
            this->on_object_end(ec);
            if(ec)
                goto finish;
            --depth_;
            st_.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        st_.push(state::ws);
        st_.push(state::value);
        st_.push(state::ws);
        st_.push(state::colon);
        st_.push(state::ws);
        st_.push(state::string1);
        st_.push(state::ws);
        is_key_ = true;
        goto loop_ws;

    case state::colon:
        BOOST_JSON_ASSERT(p < p1);
        if(*p != ':')
        {
            ec = error::expected_colon;
            goto finish;
        }
        ++p;
        st_.pop();
        goto loop;

    //------------------------------------------------------
    //
    // array
    //

    case state::array1:
        BOOST_JSON_ASSERT(p < p1);
        if(*p == ']')
        {
            ++p;
            this->on_array_end(ec);
            if(ec)
                goto finish;
            --depth_;
            st_.pop();
            goto loop;
        }
        st_.pop();
        st_.push(state::array2);
        st_.push(state::ws);
        st_.push(state::value);
        st_.push(state::ws);
        goto loop;

    case state::array2:
        BOOST_JSON_ASSERT(p < p1);
        if(*p == ']')
        {
            ++p;
            this->on_array_end(ec);
            if(ec)
                goto finish;
            --depth_;
            st_.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        st_.push(state::ws);
        st_.push(state::value);
        st_.push(state::ws);
        goto loop;

    //------------------------------------------------------
    //
    // string
    //

    // double quote opening string
    case state::string1:
loop_string:
        BOOST_JSON_ASSERT(p < p1);
        if(*p != '\"')
        {
            ec = error::expected_quotes;
            goto finish;
        }
        ++p;
        st_.pop();
        st_.push(state::string2);
        BOOST_FALLTHROUGH;

    // characters
    // No copies here
    case state::string2:
    {
        if(p >= p1)
            break;
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
                ++p;
                if(ec)
                    goto finish;
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
                    goto finish;
                ++p;
                st_.front() = state::string4;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::illegal_control_char;
                goto finish;
            }
            // TODO UTF-8?
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
            goto finish;
        break;
    }

    // characters, including escapes
    // This algorithm copies unescaped chars to a buffer
    case state::string3:
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
                    goto finish;
                ++p;
                st_.pop();
                temp.clear();
                is_key_ = false;
                goto loop;
            }
            if(*p == '\\')
            {
                ++p;
                st_.front() = state::string4;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::illegal_control_char;
                goto finish;
            }
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back(*p++);
        }
        break;
    }

    // escape
    case state::string4:
        if(p >= p1)
           break;
        switch(*p)
        {
        case '\"':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\"');
            break;

        case '\\':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\\');
            break;

        case '/':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('/');
            break;

        case 'b':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\x08');
            break;

        case 'f':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\x0c');
            break;

        case 'n':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\x0a');
            break;

        case 'r':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\x0d');
            break;

        case 't':
            if(! maybe_flush(ec))
                goto finish;
            temp.push_back('\x09');
            break;

        case 'u':
            ++p;
            st_.front() = state::string3;
            st_.push(state::u_esc1);
            goto loop;

        default:
            ec = error::illegal_escape_char;
            goto finish;
        }
        ++p;
        st_.front() = state::string3;
        goto loop;

    // utf16 escape, got "\u" already
    case state::u_esc1:
    {
        if(p >= p1)
           break;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto finish;
        }
        u_ = d << 12;
        if(p + 3 <= p1)
        {
            // fast path
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto finish;
            }
            u_ += d << 8;
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto finish;
            }
            u_ += d << 4;
            d = hex_digit(*p++);
            if(d == -1)
            {
                ec = error::expected_hex_digit;
                goto finish;
            }
            u_ += d;
            st_.front() = state::u_surr;
            goto loop;
        }
        st_.front() = state::u_esc2;
        goto loop;
    }

    case state::u_esc2:
    {
        if(p >= p1)
           break;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto finish;
        }
        u_ += d << 8;
        st_.front() = state::u_esc3;
        goto loop;
    }

    case state::u_esc3:
    {
        if(p >= p1)
           break;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto finish;
        }
        u_ += d << 4;
        st_.front() = state::u_esc4;
        goto loop;
    }

    case state::u_esc4:
    {
        if(p >= p1)
           break;
        auto d = hex_digit(*p++);
        if(d == -1)
        {
            ec = error::expected_hex_digit;
            goto finish;
        }
        u_ += d;
        st_.front() = state::u_surr;
        goto loop;
    }

    // handles 1 or 2 surrogates
    case state::u_surr:
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
                    st_.front() = state::u_pair1;
                    goto loop;
                }
                else if(u_ <= 0xdfff)
                {
                    ec = error::illegal_leading_surrogate;
                    goto finish;
                }
            }

            if(! maybe_flush(ec))
                goto finish;
            temp.append_utf8(u_);
            st_.pop();
            goto loop;
        }
        // both code units
        if( u_ <  0xdc00 ||
            u_ >  0xdfff)
        {
            ec = error::illegal_trailing_surrogate;
            goto finish;
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
    case state::u_pair1:
        if(p >= p1)
           break;
        if(*p != '\\')
        {
            ec = error::expected_utf16_escape;
            goto finish;
        }
        ++p;
        st_.front() = state::u_pair2;
        goto loop;
    
    case state::u_pair2:
        if(p >= p1)
           break;
        if(*p != 'u')
        {
            ec = error::expected_utf16_escape;
            goto finish;
        }
        ++p;
        st_.front() = state::u_esc1;
        goto loop;

    //------------------------------------------------------
    //
    // number
    //
    case state::num:
    {
loop_num:
        if(p >= p1)
            break;
        p += iep_.write_some(
            p, p1 - p, ec);
        if(ec)
            goto finish;
        if(p < p1)
        {
            iep_.write_eof(ec);
            if(ec)
                goto finish;
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
                goto finish;
            st_.pop();
            goto loop;
        }
        break;
    }

    //------------------------------------------------------

    //
    // true
    //

    case state::true1:
        if(p >= p1)
            break;
        if(*p != 'r')
        {
            ec = error::expected_true;
            goto finish;
        }
        ++p;
        st_.front() = state::true2;
        BOOST_FALLTHROUGH;

    case state::true2:
        if(p >= p1)
            break;
        if(*p != 'u')
        {
            ec = error::expected_true;
            goto finish;
        }
        ++p;
        st_.front() = state::true3;
        BOOST_FALLTHROUGH;

    case state::true3:
        if(p >= p1)
            break;
        if(*p != 'e')
        {
            ec = error::expected_true;
            goto finish;
        }
        ++p;
        this->on_bool(true, ec);
        if(ec)
            goto finish;
        st_.pop();
        goto loop;

    //
    // false
    //

    case state::false1:
        if(p >= p1)
            break;
        if(*p != 'a')
        {
            ec = error::expected_false;
            goto finish;
        }
        ++p;
        st_.front() = state::false2;
        BOOST_FALLTHROUGH;

    case state::false2:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::expected_false;
            goto finish;
        }
        ++p;
        st_.front() = state::false3;
        BOOST_FALLTHROUGH;

    case state::false3:
        if(p >= p1)
            break;
        if(*p != 's')
        {
            ec = error::expected_false;
            goto finish;
        }
        ++p;
        st_.front() = state::false4;
        BOOST_FALLTHROUGH;

    case state::false4:
        if(p >= p1)
            break;
        if(*p != 'e')
        {
            ec = error::expected_false;
            goto finish;
        }
        ++p;
        this->on_bool(false, ec);
        if(ec)
            goto finish;
        st_.pop();
        goto loop;

    //
    // null
    //

    case state::null1:
        if(p >= p1)
            break;
        if(*p != 'u')
        {
            ec = error::expected_null;
            goto finish;
        }
        ++p;
        st_.front() = state::null2;
        BOOST_FALLTHROUGH;

    case state::null2:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::expected_null;
            goto finish;
        }
        ++p;
        st_.front() = state::null3;
        BOOST_FALLTHROUGH;

    case state::null3:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::expected_null;
            goto finish;
        }
        ++p;
        this->on_null(ec);
        if(ec)
            goto finish;
        st_.pop();
        goto loop;

    //
    // done
    //

    case state::done0:
        st_.front() = state::done;
        break;

    case state::done:
        ec = error::illegal_extra_chars;
        break;
    }

finish:
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
    if(is_done() && size > 0) // state::done
    {
        ec = error::illegal_extra_chars;
        return 0;
    }
    auto n =
        write_some(data, size, ec);
    if(! ec)
    {
        write_eof(ec);
        if(! ec)
        {
            if( n < size ||
                ec == error::illegal_char)
                ec = error::illegal_extra_chars;
        }
    }
    return n;
}

//----------------------------------------------------------

void
basic_parser::
write_eof(error_code& ec)
{
    if(st_.empty())
    {
        // document never started
        ec = error::syntax;
        return;
    }
    for(;;)
    {
        // pop all states that
        // allow "" (empty string)
        switch(st_.front())
        {
        case state::ws:
            st_.pop();
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

        case state::done0:
            this->on_document_end(ec);
            if(ec)
                return;
            st_.front() = state::done;
            break;

        case state::value:
        case state::object1:
        case state::object2:
        case state::colon:
        case state::array1:
        case state::array2:
        case state::string1:
        case state::string2:
        case state::string3:
        case state::string4:
        case state::true1:
        case state::true2:
        case state::true3:
        case state::false1:
        case state::false2:
        case state::false3:
        case state::false4:
        case state::null1:
        case state::null2:
        case state::null3:
        case state::u_esc1:
        case state::u_esc2:
        case state::u_esc3:
        case state::u_esc4:
        case state::u_pair1:
        case state::u_pair2:
        case state::u_surr:
            ec = error::syntax;
            return;

        case state::done:
            goto finish;
        }
    }
finish:
    ec = {};
}

} // json
} // boost

#endif
