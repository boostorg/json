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
#include <boost/beast/core/static_string.hpp>
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
    element,
    ws,
    value,

    object1, object2, object3, object4, colon,
    array1,  array2,  array3,  array4,
    string1, string2, string3,
    true1,   true2,   true3,   true4,
    false1,  false2,  false3,  false4,  false5,
    null1,   null2,   null3,   null4,

    number,         number_mant1,       number_mant2,
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
basic_parser::is_done() const noexcept
{
    return stack_.front() == state::end;
}

void
basic_parser::
reset()
{
    stack_.clear();
    stack_.push_front(state::end);
    stack_.push_front(state::json);
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

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
            write_some(
                boost::asio::const_buffer(
                &c, 1), ec);
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
            write_some(
                boost::asio::const_buffer{}, ec);
            if(ec)
                return;
            break;

        case state::ws:
            stack_.pop_front();
            break;

        default:
            return fail();
        }
    }
    ec = {};
}

//------------------------------------------------------------------------------

std::size_t
basic_parser::
write_some(
    boost::asio::const_buffer buffer,
    error_code& ec)
{
    auto p = static_cast<
        char const*>(buffer.data());
    auto n = buffer.size();
    auto const p0 = p;
    auto const p1 = p0 + n;
    beast::static_string<4096> temp;
    ec.assign(0, ec.category());
    BOOST_ASSERT(stack_.front() != state::end);
    auto const maybe_flush =
        [&]
        {
            if(temp.size() != temp.max_size())
                return;
            if(is_key_)
                this->on_key_data(
                    {temp.data(), temp.size()}, ec);
            else
                this->on_string_data(
                    {temp.data(), temp.size()}, ec);
        };
loop:
    switch(stack_.front())
    {
    case state::json:
        this->on_document_begin(ec);
        if(ec)
            goto finish;
        stack_.front() = state::element;
        temp.clear();
        is_key_ = false;
        goto loop;

    case state::element:
        stack_.front() = state::ws;
        stack_.push_front(state::value);
        stack_.push_front(state::ws);
        goto loop;

    case state::ws:
        while(p < p1)
        {
            if(! is_ws(*p))
            {
                stack_.pop_front();
                goto loop;
            }
            ++p;
        }
        break;

    case state::value:
    {
        if(p >= p1)
            break;
        switch(*p)
        {
        // object
        case '{':
            stack_.front() = state::object1;
            goto loop;

        // array
        case '[':
            ++p;
            stack_.front() = state::array1;
            this->on_array_begin(ec);
            goto loop;

        // string
        case '"':
            stack_.front() = state::string1;
            goto loop;

        // number
        case '0':
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
        case '-':
            stack_.front() = state::number;
            goto loop;

        // true
        case 't':
            if(p + 4 <= p1)
            {
                if(
                    p[1] != 'r' ||
                    p[2] != 'u' ||
                    p[3] != 'e')
                {
                    ec = error::syntax;
                    goto finish;
                }
                p = p + 4;
                stack_.front() = state::true4;
                goto loop;
            }
            ++p;
            stack_.front() = state::true1;
            goto loop;

        // false
        case 'f':
            if(p + 5 <= p1)
            {
                if(
                    p[1] != 'a' ||
                    p[2] != 'l' ||
                    p[3] != 's' ||
                    p[4] != 'e')
                {
                    ec = error::syntax;
                    goto finish;
                }
                p = p + 5;
                stack_.front() = state::false5;
                goto loop;
            }
            ++p;
            stack_.front() = state::false1;
            goto loop;

        // null
        case 'n':
            if(p + 4 <= p1)
            {
                if(
                    p[1] != 'u' ||
                    p[2] != 'l' ||
                    p[3] != 'l')
                {
                    ec = error::syntax;
                    goto finish;
                }
                p = p + 4;
                stack_.front() = state::null4;
                goto loop;
            }
            ++p;
            stack_.front() = state::null1;
            goto loop;

        default:
            ec = error::syntax;
            goto finish;
        }
        break;
    }

    //--------------------------------------------------------------------------
    //
    // object
    //

    // beginning of object
    case state::object1:
        BOOST_ASSERT(*p == '{');
        ++p;
        this->on_object_begin(ec);
        if(ec)
            goto finish;
        stack_.front() = state::object2;
        stack_.push_front(state::ws);
        goto loop;

    // first key or end of object
    case state::object2:
        if(p >= p1)
            break;
        if(*p == '}')
        {
            ++p;
            stack_.front() = state::object4;
            goto loop;
        }
        stack_.front() = state::object3;
        stack_.push_front(state::element);
        stack_.push_front(state::colon);
        stack_.push_front(state::ws);
        stack_.push_front(state::string1);
        is_key_ = true;
        goto loop;

    case state::object3:
        if(p >= p1)
            break;
        if(*p == '}')
        {
            ++p;
            stack_.front() = state::object4;
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::object3;
        stack_.push_front(state::element);
        stack_.push_front(state::colon);
        stack_.push_front(state::ws);
        stack_.push_front(state::string1);
        stack_.push_front(state::ws);
        is_key_ = true;
        goto loop;

    case state::object4:
        this->on_object_end(ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    case state::colon:
        if(p >= p1)
            break;
        if(*p != ':')
        {
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.pop_front();
        goto loop;

    //--------------------------------------------------------------------------
    //
    // array
    //

    case state::array1:
        stack_.front() = state::array2;
        stack_.push_front(state::ws);
        goto loop;

    case state::array2:
        if(p >= p1)
            break;
        if(*p == ']')
        {
            ++p;
            stack_.front() = state::array4;
            goto loop;
        }
        stack_.front() = state::array3;
        stack_.push_front(state::element);
        goto loop;

    case state::array3:
        if(p >= p1)
            break;
        if(*p == ']')
        {
            ++p;
            stack_.front() = state::array4;
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::array3;
        stack_.push_front(state::element);
        stack_.push_front(state::ws);
        goto loop;

    case state::array4:
        this->on_array_end(ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    //--------------------------------------------------------------------------
    //
    // string
    //

    // double quote opening string
    case state::string1:
        if(p >= p1)
            break;
        if(*p != '\"')
        {
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::string2;
        goto loop;

    // characters
    case state::string2:
        while(p < p1)
        {
            if(*p == '\"')
            {
                ++p;
                if(is_key_)
                    this->on_key_end({temp.data(),
                        temp.size()}, ec);
                else
                    this->on_string_end({temp.data(),
                        temp.size()}, ec);
                if(ec)
                    goto finish;
                temp.clear();
                is_key_ = false;
                stack_.pop_front();
                goto loop;
            }
            if(*p == '\\')
            {
                ++p;
                stack_.front() = state::string3;
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::syntax;
                goto finish;
            }
            // TODO UTF-8
            maybe_flush();
            temp.push_back(*p++);
        }
        break;

    // escape
    case state::string3:
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
            BOOST_ASSERT(false);
            break;

        default:
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front()=state::string2;
        goto loop;

    //--------------------------------------------------------------------------

    //
    // true
    //

    case state::true1:
        if(p >= p1)
            break;
        if(*p != 'r')
        {
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::true4;
        BOOST_FALLTHROUGH;

    case state::true4:
        this->on_bool(true, ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    //
    // false
    //

    case state::false1:
        if(p >= p1)
            break;
        if(*p != 'a')
        {
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::false5;
        BOOST_FALLTHROUGH;

    case state::false5:
        this->on_bool(false, ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    //
    // null
    //

    case state::null1:
        if(p >= p1)
            break;
        if(*p != 'u')
        {
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
            goto finish;
        }
        ++p;
        stack_.front() = state::null4;
        BOOST_FALLTHROUGH;

    case state::null4:
        this->on_null(ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    //
    // number
    //

    case state::number:
        BOOST_ASSERT(p < p1);
        n_mant_ = 0;
        n_exp_ = 0;
        if(*p == '-')
        {
            ++p;
            n_neg_ = true;
        }
        else
        {
            n_neg_ = false;
        }
        stack_.front() = state::number_mant1;
        goto loop;

    case state::number_mant1:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa digit
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
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
            ec = error::syntax;
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
        this->on_number(number(
            n_mant_, n_exp_, n_neg_), ec);
        if(ec)
            goto finish;
        stack_.pop_front();
        goto loop;

    //
    // (end)
    //

    case state::end:
        /*
        if(p < p1)
        {
            // unexpected extra characters
            ec = error::syntax;
            goto finish;
        }
        */
        break;
    }

finish:
    return p - p0;
}

//------------------------------------------------------------------------------

// Called to parse the rest of the document, this
// can be optimized by assuming no more data is coming.
std::size_t
basic_parser::
write(
    boost::asio::const_buffer buffer,
    error_code& ec)
{
    auto bytes_used =
        write_some(buffer, ec);
    if(! ec)
        write_eof(ec);
    return bytes_used;
}

} // json
} // boost

#endif
