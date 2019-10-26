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
#include <boost/json/detail/assert.hpp>

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
    
    num_mant1,      num_mant2,
    num_frac1,      num_frac2,      num_frac3,
    num_exp,        num_exp_sign,
    num_exp_dig1,   num_exp_dig2,   num_end,

    end
};

//----------------------------------------------------------

class basic_parser::stack_impl
{
    state* base_;
    unsigned* size_;
    unsigned capacity_;

public:
    stack_impl() = default;
    stack_impl(
        stack_impl const&) = default;

    stack_impl(
        stack const& s,
        unsigned& size) noexcept
        : base_(reinterpret_cast<
            state*>(s.base))
        , size_(&size)
        , capacity_(s.capacity)
    {
    }

    operator stack() const noexcept
    {
        return {base_, capacity_};
    }

    void*
    base() noexcept
    {
        return base_;
    }

    unsigned
    size() const noexcept
    {
        return *size_;
    }

    unsigned
    capacity() const noexcept
    {
        return capacity_;
    }

    state&
    front() noexcept
    {
        return base_[*size_ - 1];
    }

    bool
    available(
        unsigned n) const noexcept
    {
        return n <= capacity_ - *size_;
    }

    void
    pop() noexcept
    {
        --*size_;
    }

    void
    push(state v) noexcept
    {
        base_[(*size_)++] = v;
    }
};

//----------------------------------------------------------

basic_parser::
basic_parser()
{
}

bool
basic_parser::
is_done() const noexcept
{
    return top_ == 1;
}

void
basic_parser::
reset()
{
    top_ = 0;
}

//----------------------------------------------------------

// Append the digit to the
// value, which must be unsigned.
// Returns `false` on overflow.
bool
basic_parser::
append_mantissa(
    char digit) noexcept
{
    decltype(num_.mantissa) temp =
        num_.mantissa * 10;
    if(temp < num_.mantissa)
        return false;
    decltype(num_.mantissa) result =
        temp + digit;
    if(result < temp)
        return false;
    num_.mantissa = result;
    return true;
}

// Append the digit to the signed exponent
bool
basic_parser::
append_exponent(
    char digit) noexcept
{
    if(! esign_)
    {
        decltype(num_.exponent) temp =
            num_.exponent * 10 + digit;
        if(temp >= num_.exponent)
        {
            num_.exponent = temp;
            return true;
        }
        return false;
    }
    decltype(num_.exponent) temp =
        num_.exponent * 10 - digit;
    // VFALCO TODO check for overflow
    num_.exponent = temp;
    return true;
}

//----------------------------------------------------------

std::size_t
basic_parser::
write_some(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    auto p = static_cast<char const*>(data);
    auto n = size;
    auto const p0 = p;
    auto const p1 = p0 + n;
    detail::buffer<2048> temp;
    stack_impl st;
    ec = {};

    auto const maybe_flush =
        [this, &temp](error_code& ec)
        {
            // need 4 chars for largest utf8 code point
            if(temp.size() < temp.max_size() - 4)
                return true;
            if(is_key_)
                this->on_key_data(temp, ec);
            else
                this->on_string_data(temp, ec);
            temp.clear();
            if(ec)
                return false;
            return true;
        };

    auto const prepare_stack =
        [this, &st](error_code& ec)
        {
            // check that the stack is big
            // enough for the biggest scalar
            if(st.available(10))
                return;
            stack si;
            si.base = st.base();
            si.capacity = st.capacity();
            this->on_stack_grow(
                si, st.capacity() + 10, ec);
            if(ec)
                return;
            st = stack_impl(si, top_);
        };

    {
        stack si;
        this->on_stack_info(si);
        st = stack_impl(si, top_);
    }

    // begin document
    if(top_ == 0)
    {
        this->on_document_begin(ec);
        if(ec)
            goto finish;
        st.push(state::end);
        st.push(state::ws);
        st.push(state::value);
        st.push(state::ws);
        is_key_ = false;
        u0_ = -1;
        goto loop_ws;
    }

    if(st.front() == state::end)
    {
        ec = error::illegal_extra_chars;
        goto finish;
    }
loop:
    switch(st.front())
    {
    case state::ws:
loop_ws:
        while(p < p1)
        {
            if(! is_ws(*p))
            {
                st.pop();
                goto loop;
            }
            ++p;
        }
        break;

    case state::value:
    {
        BOOST_JSON_ASSERT(p < p1);
        switch(*p)
        {
        // object
        case '{':
            ++p;
            st.front() = state::object1;
            st.push(state::ws);
            this->on_object_begin(ec);
            if(ec)
                goto finish;
            prepare_stack(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // array
        case '[':
            ++p;
            st.front() = state::array1;
            st.push(state::ws);
            this->on_array_begin(ec);
            if(ec)
                goto finish;
            prepare_stack(ec);
            if(ec)
                goto finish;
            goto loop_ws;

        // string
        case '"':
            st.front() = state::string1;
            goto loop_string;

        // number
        case '-':
            ++p;
            num_.mantissa = 0;
            num_.exponent = 0;
            num_.sign = true;
            efrac_ = 0;
            st.front() = state::num_mant1;
            goto loop_mant;

        case '0':
            ++p;
            num_.mantissa = 0;
            num_.exponent = 0;
            num_.sign = false;
            efrac_ = 0;
            st.front() = state::num_frac1;
            goto loop_frac;

        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
            num_.mantissa = 0;
            num_.exponent = 0;
            num_.sign = false;
            efrac_ = 0;
            st.front() = state::num_mant2;
            goto loop_mant;

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
                    st.pop();
                    goto loop;
                }
                ec = error::expected_true;
                goto finish;
            }
            ++p;
            st.front() = state::true1;
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
                    st.pop();
                    goto loop;
                }
                ec = error::expected_false;
                goto finish;
            }
            ++p;
            st.front() = state::false1;
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
                    st.pop();
                    goto loop;
                }
                ec = error::expected_null;
                goto finish;
            }
            ++p;
            st.front() = state::null1;
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
        BOOST_JSON_ASSERT(p < p1);
        if(*p == '}')
        {
            ++p;
            this->on_object_end(ec);
            if(ec)
                goto finish;
            st.pop();
            goto loop;
        }
        st.pop();
        st.push(state::object2);
        st.push(state::ws);
        st.push(state::value);
        st.push(state::ws);
        st.push(state::colon);
        st.push(state::ws);
        st.push(state::string1);
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
            st.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        st.push(state::ws);
        st.push(state::value);
        st.push(state::ws);
        st.push(state::colon);
        st.push(state::ws);
        st.push(state::string1);
        st.push(state::ws);
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
        st.pop();
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
            st.pop();
            goto loop;
        }
        st.pop();
        st.push(state::array2);
        st.push(state::ws);
        st.push(state::value);
        st.push(state::ws);
        goto loop;

    case state::array2:
        BOOST_JSON_ASSERT(p < p1);
        if(*p == ']')
        {
            ++p;
            this->on_array_end(ec);
            if(ec)
                goto finish;
            st.pop();
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::expected_comma;
            goto finish;
        }
        ++p;
        st.push(state::ws);
        st.push(state::value);
        st.push(state::ws);
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
        st.pop();
        st.push(state::string2);
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
                st.pop();
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
                st.front() = state::string4;
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
                if(ec)
                    goto finish;
                ++p;
                st.pop();
                temp.clear();
                is_key_ = false;
                goto loop;
            }
            if(*p == '\\')
            {
                ++p;
                st.front() = state::string4;
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
            temp.push_back('\x08');
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
            st.front() = state::string3;
            st.push(state::u_esc1);
            goto loop;

        default:
            ec = error::illegal_escape_char;
            goto finish;
        }
        ++p;
        st.front() = state::string3;
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
            st.front() = state::u_surr;
            goto loop;
        }
        st.front() = state::u_esc2;
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
        st.front() = state::u_esc3;
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
        st.front() = state::u_esc4;
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
        st.front() = state::u_surr;
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
                    st.front() = state::u_pair1;
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
            st.pop();
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
        st.pop();
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
        st.front() = state::u_pair2;
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
        st.front() = state::u_esc1;
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
        st.front() = state::true2;
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
        st.front() = state::true3;
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
        st.pop();
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
        st.front() = state::false2;
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
        st.front() = state::false3;
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
        st.front() = state::false4;
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
        st.pop();
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
        st.front() = state::null2;
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
        st.front() = state::null3;
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
        st.pop();
        goto loop;

    //
    // number
    //

    // 
    case state::num_mant1:
loop_mant:
        if(p >= p1)
            break;
        if(is_digit(*p))
        {
            if(*p != '0')
            {
                st.front() = state::num_mant2;
                goto loop;
            }
            ++p;
            st.front() = state::num_frac1;
            goto loop;
        }
        // expected mantissa digit
        ec = error::expected_mantissa;
        goto finish;

    case state::num_mant2:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                st.front() = state::num_frac1;
                goto loop;
            }
            if(! append_mantissa(*p++ - '0'))
            {
                ec = error::mantissa_overflow;
                goto finish;
            }
        }
        break;

    case state::num_frac1:
loop_frac:
        if(p >= p1)
            break;
        if(*p == '.')
        {
            ++p;
            st.front() = state::num_frac2;
            goto loop;
        }
        if(is_digit(*p))
        {
            // unexpected digit after zero
            ec = error::illegal_extra_digits;
            goto finish;
        }
        st.front() = state::num_exp;
        goto loop;

    case state::num_frac2:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa fraction digit
            ec = error::expected_fraction;
            goto finish;
        }
        st.front() = state::num_frac3;
        goto loop;

    case state::num_frac3:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                st.front() = state::num_exp;
                goto loop;
            }
            if(! append_mantissa(*p++ - '0'))
            {
                ec = error::mantissa_overflow;
                goto finish;
            }
            ++efrac_;
        }
        break;

    case state::num_exp:
        BOOST_JSON_ASSERT(p < p1);
        if(*p != 'e' && *p != 'E')
            goto loop_num_end;
        ++p;
        st.front() = state::num_exp_sign;
        BOOST_FALLTHROUGH;

    case state::num_exp_sign:
        if(p >= p1)
            break;
        if(*p == '-')
        {
            ++p;
            esign_ = true;
        }
        else
        {
            if(*p == '+')
                ++p;
            esign_ = false;
        }
        st.front() = state::num_exp_dig1;
        BOOST_FALLTHROUGH;

    case state::num_exp_dig1:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected exponent digit
            ec = error::expected_exponent;
            goto finish;
        }
        st.front() = state::num_exp_dig2;
        goto loop;

    case state::num_exp_dig2:
        while(p < p1)
        {
            if(! is_digit(*p))
                goto loop_num_end;
            if(! append_exponent(*p++ - '0'))
            {
                ec = error::exponent_overflow;
                goto finish;
            }
        }
        break;

    case state::num_end:
loop_num_end:
        // NOTE this code is
        // repeated in write_eof.
        num_.exponent -= efrac_;
        this->on_number(num_, ec);
        if(ec)
            goto finish;
        st.pop();
        goto loop;

    //
    // (end)
    //

    case state::end:
        goto finish;
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
    if(top_ == 1) // state::end
    {
        if(size == 0)
            ec = {};
        else
            ec = error::illegal_extra_chars;
        return 0;
    }
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
            BOOST_JSON_ASSERT(ec);
        };

    stack_impl st;
    {
        stack si;
        this->on_stack_info(si);
        st = stack_impl(si, top_);
    }
    if(top_ == 0)
    {
        // document never started
        ec = error::syntax;
        return;
    }
    while(st.front() != state::end)
    {
        // pop all states that
        // allow "" (empty string)
        switch(st.front())
        {
        case state::num_mant2:
        case state::num_frac1:
        case state::num_frac3:
        case state::num_exp:
        case state::num_exp_dig2:
            num_.exponent -= efrac_;
            this->on_number(num_, ec);
            if(ec)
                return;
            st.pop();
            break;

        case state::num_end:
            // should never get here
            BOOST_JSON_ASSERT(false);
            st.pop();
            break;

        case state::ws:
            st.pop();
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
        case state::num_mant1:
        //case state::num_mant2:
        //case state::num_frac1:
        case state::num_frac2:
        //case state::num_frac3:
        //case state::num_exp:
        case state::num_exp_sign:
        case state::num_exp_dig1:
        //case state::num_exp_dig2:
            ec = error::syntax;
            return;
        }
    }
    ec = {};
}

} // json
} // boost

#endif
