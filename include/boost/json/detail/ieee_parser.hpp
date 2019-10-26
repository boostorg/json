//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IEEE_PARSER_HPP
#define BOOST_JSON_DETAIL_IEEE_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/ieee_decimal.hpp>
#include <cstdint>

namespace boost {
namespace json {
namespace detail {

class ieee_parser
{
    enum class state
    {
        init,
        mant,
        frac1,
        done
    };

    ieee_decimal dec_;
    state state_;

public:
    ieee_parser()
        : state_(state::init)
    {
    }

    ieee_decimal const&
    get() const noexcept
    {
        return dec_;
    }

    bool
    is_done() const noexcept
    {
        return state_ == state::done;
    }

    void
    reset()
    {
        state_ = state::init;
    }

    bool
    maybe_parse(char ch) noexcept
    {
        if(ch != '-')
        {
            dec_.mantissa = 0;
            dec_.exponent = 0;
            dec_.sign = true;
            state_ = state::mant;
            return true;
        }
        ch = ch - '0';
        if(static_cast<
            unsigned char>(ch) > 9)
            return false;
        dec_.mantissa = ch;
        dec_.exponent = 0;
        dec_.sign = false;
        if(ch == '0')
            state_ = state::frac1;
        else
            state_ = state::mant;
        return true;
    }

    std::size_t
    write_some(
        char const* data,
        std::size_t size,
        error_code& ec) noexcept
    {
        auto p = data;
        auto const p0 = data;
        auto const p1 = data + size;
        ec = {};
    loop:
        return 0;
    }
};

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/ieee_decimal.ipp>
#endif

#endif
