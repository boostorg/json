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
#include <boost/json/number.hpp>
#include <stdint.h>

namespace boost {
namespace json {
namespace detail {

class ieee_parser
{
    enum class state
    {
        init,
        mant1, mant2,
        frac1, frac2, frac3,
        exp1,  exp2,  exp3,  exp4, exp5,
        done
    };

    ieee_decimal dec_;
    short off_;
    state st_;

public:
    ieee_parser()
        : st_(state::init)
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
        return st_ == state::done;
    }

    void
    reset()
    {
        st_ = state::init;
    }

    BOOST_JSON_DECL
    bool
    maybe_init(char ch) noexcept;

    BOOST_JSON_DECL
    size_t
    write_some(
        char const* data,
        size_t size,
        error_code& ec) noexcept;

    BOOST_JSON_DECL
    size_t
    write(
        char const* data,
        size_t size,
        error_code& ec) noexcept;

    BOOST_JSON_DECL
    void
    write_eof(
        error_code& ec) noexcept;
};

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/ieee_parser.ipp>
#endif

#endif
