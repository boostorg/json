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
#include <boost/json/detail/buffer.hpp>
#include <boost/assert.hpp>

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
    json,
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
    
    number_mant1,   number_mant2,
    number_fract1,  number_fract2,      number_fract3,
    number_exp,     number_exp_sign,
    number_exp_digits1, number_exp_digits2, number_end,

    end
};

basic_parser::
basic_parser()
{
    reset();
}

bool
basic_parser::
is_done() const noexcept
{
    return stack_.front() == state::end;
}

void
basic_parser::
reset()
{
    stack_.clear();
    stack_.push(state::end);
    stack_.push(state::json);
    is_key_ = false;
    n_neg_ = false;
    u0_ = -1;
}

//----------------------------------------------------------

// Append the digit to the
// value, which must be unsigned.
// Returns `false` on overflow.
bool
basic_parser::
append_digit(
    number::mantissa_type* value,
    char digit)
{
    number::mantissa_type temp =
        *value * 10;
    if(temp < *value)
        return false;
    number::mantissa_type result =
        temp + digit;
    if(result < temp)
        return false;
    *value = result;
    return true;
}

// Append the digit to the signed exponent
bool
basic_parser::
append_digit(
    number::exponent_type* value,
    char digit, bool neg)
{
    if(neg)
    {
        if(! *value)
        {
            *value = -digit;
        }
        else
        {
            *value *= 10;
            *value -= digit;
        }
    }
    else
    {
        *value *= 10;
        *value += digit;
    }
    return true;
}

//----------------------------------------------------------

std::size_t
basic_parser::
write_some(
    void const* data,
    std::size_t size,
    error_code& ec)
{
    auto p = static_cast<char const*>(data);
    auto n = size;
    auto const p0 = p;
    auto const p1 = p0 + n;
    detail::buffer<2048> temp;
    ec = {};
    BOOST_ASSERT(stack_.front() != state::end);
    auto const maybe_flush =
        [&]
        {
            // need 4 chars for largest utf8 code point
            if(temp.size() < temp.max_size() - 4)
                return;
            if(is_key_)
                this->on_key_data(temp, ec);
            else
                this->on_string_data(temp, ec);
            temp.clear();
        };
loop:
    switch(stack_.front())
    {
    case state::json:
        this->on_document_begin(ec);
        if(ec)
            goto finish;
        stack_.pop();
        stack_.push(state::ws);
        stack_.push(state::value);
        stack_.push(state::ws);
        BOOST_FALLTHROUGH;

    case state::ws:
loop_ws:
        while(p < p1)
        {
            if(! is_ws(*p))
            {
                stack_.pop();
                goto loop;
            }
            ++p;
        }
        break;

    case state::value:
    {
        BOOST_ASSERT(p < p1);
        switch(*p)
        {
        // object
        case '{':
            ++p;
            stack_.front() = state::object1;
            stack_.push(state::ws);
            this->on_object_begin(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // array
        case '[':
            ++p;
            stack_.front() = state::array1;
            stack_.push(state::ws);
            this->on_array_begin(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // string
        case '"':
            stack_.front() = state::string1;
            goto loop_string;

        // number
        case '-':
            ++p;
            n_neg_ = true;
            BOOST_FALLTHROUGH;
        case '0':
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
            n_mant_ = 0;
            n_exp_ = 0;
            stack_.front() = state::number_mant1;
            goto loop_number;

        // true
        case 't':
            if(p + 4 <= p1)
            {
                if(
                    p[1] == 'r' &&
                    p[2] == 'u' &&
                    p[3] == 'e')
                {
                    p = p + 4;
                    this->on_bool(true, ec);
                    if(ec)
                        goto finish;
                    stack_.pop();
                    goto loop;
                }
                ec = error::expected_true;
                goto finish;
            }
            ++p;
            stack_.front() = state::true1;
            goto loop;

        // false
        case 'f':
            if(p + 5 <= p1)
            {
                if(
                    p[1] == 'a' &&
                    p[2] == 'l' &&
                    p[3] == 's' &&
                    p[4] == 'e')
                {
                    p = p + 5;
                    this->on_bool(false, ec);
                    if(ec)
                        goto finish;
                    stack_.pop();
                    goto loop;
                }
                ec = error::expected_false;
                goto finish;
            }
            ++p;
            stack_.front() = state::false1;
            goto loop;

        // null
        case 'n':
            if(p + 4 <= p1)
            {
                if(
                    p[1] == 'u' &&
                    p[2] == 'l' &&
                    p[3] == 'l')
                {
                    p = p + 4;
                    this->on_null(ec);
                    if(ec)
                        goto finish;
                    stack_.pop();
                    goto loop;
                }
                ec = error::expected_null;
                goto finish;
            }
            ++p;
            stack_.front() = state::null1;
            goto loop;

        default:
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
        BOOST_ASSERT(p < p1);
        if(*p == '}')
        {
            ++p;
            this->on_object_end(ec);
            if(ec)
                goto finish;
            stack_.pop();
            goto loop;
        }
        stack_.pop();
        stack_.push(state::object2);
        stack_.push(state::ws);
        stack_.push(state::value);
        stack_.push(state::ws);
        stack_.push(state::colon);
        stack_.push(state::ws);
        stack_.push(state::string1);
        is_key_ = true;
        goto loop;

    case state::object2:
        BOOST_ASSERT(p < p1);
        if(*p == '}')
        {
            ++p;
            this->on_object_end(ec);
            if(ec)
                goto finish;
            stack_.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        stack_.push(state::ws);
        stack_.push(state::value);
        stack_.push(state::ws);
        stack_.push(state::colon);
        stack_.push(state::ws);
        stack_.push(state::string1);
        stack_.push(state::ws);
        is_key_ = true;
        goto loop_ws;

    case state::colon:
        BOOST_ASSERT(p < p1);
        if(*p != ':')
        {
            ec = error::expected_colon;
            goto finish;
        }
        ++p;
        stack_.pop();
        goto loop;

    //------------------------------------------------------
    //
    // array
    //

    case state::array1:
        BOOST_ASSERT(p < p1);
        if(*p == ']')
        {
            ++p;
            this->on_array_end(ec);
            if(ec)
                goto finish;
            stack_.pop();
            goto loop;
        }
        stack_.pop();
        stack_.push(state::array2);
        stack_.push(state::ws);
        stack_.push(state::value);
        stack_.push(state::ws);
        goto loop;

    case state::array2:
        if(p >= p1)
            break;
        if(*p == ']')
        {
            ++p;
            this->on_array_end(ec);
            if(ec)
                goto finish;
            stack_.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        stack_.push(state::ws);
        stack_.push(state::value);
        stack_.push(state::ws);
        goto loop;

    //------------------------------------------------------
    //
    // string
    //

    // double quote opening string
    case state::string1:
loop_string:
        if(p >= p1)
            break;
        if(*p != '\"')
        {
            ec = error::expected_quotes;
            goto finish;
        }
        ++p;
        stack_.pop();
        stack_.push(state::string2);
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
            if(*p == '\"')
            {
                if(is_key_)
                    this->on_key_end({start,
                        static_cast<std::size_t>(
                            p - start)}, ec);
                else
                    this->on_string_end({start,
                        static_cast<std::size_t>(
                            p - start)}, ec);
                ++p;
                if(ec)
                    goto finish;
                is_key_ = false;
                stack_.pop();
                goto loop;
            }
            if(*p == '\\')
            {
                if(p > start)
                {
                    if(is_key_)
                        this->on_key_data({start,
                            static_cast<std::size_t>(
                                p - start)}, ec);
                    else
                        this->on_string_data({start,
                            static_cast<std::size_t>(
                                p - start)}, ec);
                }
                if(ec)
                    goto finish;
                ++p;
                stack_.front() = state::string4;
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
        BOOST_ASSERT(p != start);
        if(is_key_)
            this->on_key_data({start,
                static_cast<std::size_t>(
                    p - start)}, ec);
        else
            this->on_string_data({start,
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
                    this->on_key_end(temp, ec);
                else
                    this->on_string_end(temp, ec);
                ++p;
                if(ec)
                    goto finish;
                temp.clear();
                is_key_ = false;
                stack_.pop();
                goto loop;
            }
            if(*p == '\\')
            {
                ++p;
                stack_.front() = state::string4;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::illegal_control_char;
                goto finish;
            }
            maybe_flush();
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
            maybe_flush();
            temp.push_back('\"');
            break;

        case '\\':
            maybe_flush();
            temp.push_back('\\');
            break;

        case '/':
            maybe_flush();
            temp.push_back('/');
            break;

        case 'b':
            maybe_flush();
            temp.push_back('\x08');
            break;

        case 'f':
            maybe_flush();
            temp.push_back('\x08');
            break;

        case 'n':
            maybe_flush();
            temp.push_back('\x0a');
            break;

        case 'r':
            maybe_flush();
            temp.push_back('\x0d');
            break;

        case 't':
            maybe_flush();
            temp.push_back('\x09');
            break;

        case 'u':
            ++p;
            stack_.front() = state::string3;
            stack_.push(state::u_esc1);
            goto loop;

        default:
            ec = error::illegal_escape_char;
            goto finish;
        }
        ++p;
        stack_.front() = state::string3;
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
            stack_.front() = state::u_surr;
            goto loop;
        }
        stack_.front() = state::u_esc2;
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
        stack_.front() = state::u_esc3;
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
        stack_.front() = state::u_esc4;
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
        stack_.front() = state::u_surr;
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
                    stack_.front() = state::u_pair1;
                    goto loop;
                }
                else if(u_ <= 0xdfff)
                {
                    ec = error::illegal_leading_surrogate;
                    goto finish;
                }
            }

            maybe_flush();
            temp.append_utf8(u_);
            stack_.pop();
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
        stack_.pop();
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
        stack_.front() = state::u_pair2;
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
        stack_.front() = state::u_esc1;
        goto loop;

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
        stack_.front() = state::true2;
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
        stack_.front() = state::true3;
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
        stack_.pop();
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
        stack_.front() = state::false2;
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
        stack_.front() = state::false3;
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
        stack_.front() = state::false4;
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
        stack_.pop();
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
        stack_.front() = state::null2;
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
        stack_.front() = state::null3;
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
        stack_.pop();
        goto loop;

    //
    // number
    //

    case state::number_mant1:
loop_number:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa digit
            ec = error::expected_mantissa;
            goto finish;
        }
        if(*p != '0')
        {
            stack_.front() = state::number_mant2;
            goto loop;
        }
        ++p;
        stack_.front() = state::number_fract1;
        goto loop;

    case state::number_mant2:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                stack_.front() = state::number_fract1;
                goto loop;
            }
            if(! append_digit(&n_mant_, *p++ - '0'))
            {
                ec = error::mantissa_overflow;
                goto finish;
            }
        }
        break;

    case state::number_fract1:
        if(p >= p1)
            break;
        if(*p == '.')
        {
            ++p;
            stack_.front() = state::number_fract2;
            goto loop;
        }
        if(is_digit(*p))
        {
            // unexpected digit after zero
            ec = error::illegal_extra_digits;
            goto finish;
        }
        stack_.front() = state::number_exp;
        goto loop;

    case state::number_fract2:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa fraction digit
            ec = error::expected_fraction;
            goto finish;
        }
        stack_.front() = state::number_fract3;
        goto loop;

    case state::number_fract3:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                stack_.front() = state::number_exp;
                goto loop;
            }
            if(! append_digit(&n_mant_, *p++ - '0'))
            {
                ec = error::mantissa_overflow;
                goto finish;
            }
            --n_exp_;
        }
        break;

    case state::number_exp:
        if(p >= p1)
            break;
        if(*p == 'e' || *p == 'E')
        {
            ++p;
            stack_.front() = state::number_exp_sign;
            goto loop;
        }
        stack_.front() = state::number_end;
        goto loop;

    case state::number_exp_sign:
        if(p >= p1)
            break;
        if(*p == '+')
        {
            ++p;
            n_exp_neg_ = false;
        }
        else if(*p == '-')
        {
            ++p;
            n_exp_neg_ = true;
        }
        else
        {
            n_exp_neg_ = false;
        }
        stack_.front() = state::number_exp_digits1;
        goto loop;

    case state::number_exp_digits1:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected exponent digit
            ec = error::expected_exponent;
            goto finish;
        }
        stack_.front() = state::number_exp_digits2;
        goto loop;

    case state::number_exp_digits2:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                stack_.front() = state::number_end;
                goto loop;
            }
            if(! append_digit(&n_exp_,
                *p++ - '0', n_exp_neg_))
            {
                ec = error::exponent_overflow;
                goto finish;
            }
        }
        break;

    case state::number_end:
        n_neg_ = false;
        this->on_number(number(
            n_mant_, n_exp_, n_neg_), ec);
        if(ec)
            goto finish;
        stack_.pop();
        goto loop;

    //
    // (end)
    //

    case state::end:
        if(p < p1)
        {
            // unexpected extra characters
            ec = error::illegal_extra_chars;
            goto finish;
        }
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
    void const* data,
    std::size_t size,
    error_code& ec)
{
    auto bytes_used =
        write_some(data, size, ec);
    if(! ec)
    {
        write_eof(ec);
        if(! ec)
        {
            if( bytes_used < size ||
                ec == error::illegal_char)
                ec = error::illegal_extra_chars;
        }
    }
    return bytes_used;
}

//----------------------------------------------------------

void
basic_parser::
write_eof(error_code& ec)
{
    // write a null, this is invalid no matter
    // what state we are in, to get a descriptive
    // error.
    //
    // VFALCO we might want to return error::partial_data

    auto const fail =
        [this, &ec]
        {
            char c = 0;
            write_some(&c, 1, ec);
            BOOST_ASSERT(ec);
        };

    while(stack_.front() != state::end)
    {
        // pop all states that
        // allow "" (empty string)
        switch(stack_.front())
        {
        case state::number_mant2:
        case state::number_fract1:
        case state::number_fract3:
        case state::number_exp:
        case state::number_exp_digits2:
            stack_.front() = state::number_end;
            write_some(nullptr, 0, ec);
            if(ec)
                return;
            break;

        case state::ws:
            stack_.pop();
            break;

        default:
            return fail();
        }
    }
    ec = {};
}


} // json
} // boost

#endif
