//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IEEE_PARSER_IPP
#define BOOST_JSON_DETAIL_IEEE_PARSER_IPP

#include <boost/json/detail/ieee_parser.hpp>
#include <boost/json/detail/assert.hpp>

namespace boost {
namespace json {
namespace detail {

bool
ieee_parser::
maybe_init(char ch) noexcept
{
    if(ch == '-')
    {
        dec_.mantissa = 0;
        dec_.exponent = 0;
        dec_.sign = true;
        off_ = 0;
        st_ = state::mant1;
        return true;
    }
    unsigned char const d = ch - '0';
    if(d >= 10)
        return false;
    dec_.mantissa = d;
    dec_.exponent = 0;
    dec_.sign = false;
    off_ = 0;
    if(ch == '0')
        st_ = state::frac1;
    else
        st_ = state::mant2;
    return true;
}

std::size_t
ieee_parser::
write_some(
    char const* const data,
    std::size_t const size,
    error_code& ec) noexcept
{
    auto p = data;
    auto const p0 = data;
    auto const p1 = data + size;
    ec = {};
loop:
    switch(st_)
    {
    case state::init:
        if(p >= p1)
            break;
        if(! maybe_init(*p))
        {
            ec = error::not_number;
            break;
        }
        ++p;
        goto loop;

    // one digit
    case state::mant1:
        if(p >= p1)
            break;
        {
            unsigned char const d = *p - '0';
            if(d >= 10)
            {
                ec = error::expected_mantissa;
                break;
            }
            ++p;
            if(d == 0)
            {
                st_ = state::frac1;
                goto loop;
            }
            dec_.mantissa = d;
            st_ = state::mant2;
            BOOST_FALLTHROUGH;
        }

    // zero or more digits
    case state::mant2:
        while(p < p1)
        {
            unsigned char const d = *p - '0';
            if(d < 10)
            {
                unsigned long long const m =
                    dec_.mantissa * 10 + d;
                if(m > dec_.mantissa)
                    dec_.mantissa = m;
                else
                    ++off_; // limit of precision
                ++p;
                continue;
            }
            if(*p == '.')
            {
                ++p;
                st_ = state::frac2;
                goto loop;
            }
            if(*p == 'e' || *p == 'E')
            {
                ++p;
                st_ = state::exp1;
                goto loop;
            }
            dec_.exponent = off_;
            st_ = state::done;
            break;
        }
        break;

    // one of [.eE]
    case state::frac1:
        if(p >= p1)
            break;
        if(*p == 'e' || *p == 'E')
        {
            ++p;
            st_ = state::exp1;
            goto loop;
        }
        if(*p != '.')
        {
            dec_.exponent = off_;
            st_ = state::done;
            break;
        }
        ++p;
        st_ = state::frac2;
        BOOST_FALLTHROUGH;

    // one fraction digit
    case state::frac2:
        if(p >= p1)
            break;
        {
            unsigned char const d = *p - '0';
            if(d >= 10)
            {
                ec = error::expected_fraction;
                break;
            }
            unsigned long long const m =
                dec_.mantissa * 10 + d;
            if(m > dec_.mantissa)
            {
                --off_;
                dec_.mantissa = m;
            }
            else
            {
                // limit of precision
            }
        }
        ++p;
        st_ = state::frac3;
        BOOST_FALLTHROUGH;

    // zero or more fraction digits
    case state::frac3:
        while(p < p1)
        {
            unsigned char const d = *p - '0';
            if(d < 10)
            {
                unsigned long long const m =
                    dec_.mantissa * 10 + d;
                if(m > dec_.mantissa)
                {
                    --off_;
                    dec_.mantissa = m;
                }
                else
                {
                    // limit of precision
                }
                ++p;
                continue;
            }
            if(*p != 'e' && *p != 'E')
            {
                dec_.exponent += off_;
                st_ = state::done;
                goto finish;
            }
            ++p;
            st_ = state::exp1;
            break;
        }
        BOOST_FALLTHROUGH;

    // plus or minus
    case state::exp1:
        if(p >= p1)
            break;
        if(*p == '-')
        {
            ++p;
            st_ = state::exp4;
            goto loop;
        }
        if(*p == '+')
            ++p;
        st_ = state::exp2;
        BOOST_FALLTHROUGH;

    // one digit
    case state::exp2:
    {
        if(p >= p1)
            break;
        unsigned char const d = *p - '0';
        if(d >= 10)
        {
            ec = error::expected_exponent;
            break;
        }
        ++p;
        dec_.exponent =
            static_cast<short>(d);
        st_ = state::exp3;
        BOOST_FALLTHROUGH;
    }

    // zero or more digits
    case state::exp3:
    {
        auto const lim = 308 - off_;
        while(p < p1)
        {
            unsigned char const d = *p - '0';
            if(d < 10)
            {
                ++p;
                int const e =
                    dec_.exponent * 10 + d;
                if(e <= lim)
                {
                    dec_.exponent =
                        static_cast<short>(e);
                    continue;
                }
                ec = error::exponent_overflow;
                break;
            }
            dec_.exponent += off_;
            st_ = state::done;
            break;
        }
        break;
    }

    // one digit
    case state::exp4:
    {
        if(p >= p1)
            break;
        unsigned char const d = *p - '0';
        if(d >= 10)
        {
            ec = error::expected_exponent;
            break;
        }
        ++p;
        dec_.exponent =
            - static_cast<short>(d);
        st_ = state::exp5;
        BOOST_FALLTHROUGH;
    }

    // zero or more digits
    case state::exp5:
    {
        auto const lim = -323 - off_;
        while(p < p1)
        {
            unsigned char const d = *p - '0';
            if(d < 10)
            {
                ++p;
                int const e =
                    dec_.exponent * 10 - d;
                if(e >= lim)
                {
                    dec_.exponent =
                        static_cast<short>(e);
                    continue;
                }
                ec = error::exponent_overflow;
                break;
            }
            dec_.exponent += off_;
            st_ = state::done;
            break;
        }
        break;
    }

    case state::done:
        ec = error::illegal_extra_chars;
        break;
    }
finish:
    return p - p0;
}

std::size_t
ieee_parser::
write(
    char const* data,
    std::size_t size,
    error_code& ec) noexcept
{
    auto n = write_some(data, size, ec);
    if(! ec)
    {
        if(n < size)
            ec = error::illegal_extra_chars;
    }
    if(! ec)
        write_eof(ec);
    return n;
}

void
ieee_parser::
write_eof(
    error_code& ec) noexcept
{
    switch(st_)
    {
    case state::done:
        ec = {};
        break;

    case state::mant2:
    case state::frac1:
    case state::frac3:
        ec = {};
        dec_.exponent = off_;
        break;

    case state::exp3:
    case state::exp5:
        ec = {};
        dec_.exponent += off_;
        break;

    case state::init:
    case state::mant1:
        ec = error::expected_mantissa;
        break;

    case state::frac2:
        ec = error::expected_fraction;
        break;

    case state::exp1:
    case state::exp2:
    case state::exp4:
        ec = error::expected_exponent;
        break;
    }
}

} // detail
} // json
} // boost

#endif
