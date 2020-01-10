//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_NUMBER_HPP
#define BOOST_JSON_DETAIL_NUMBER_HPP

#include <boost/json/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/kind.hpp>
#include <cmath>
#include <type_traits>

namespace boost {
namespace json {
namespace detail {

struct number
{
    union
    {
        double d;
        int64_t i;
        uint64_t u;
    };

    json::kind kind;
};

//----------------------------------------------------------

class number_parser
{
    enum class state
    {
        init,   init0,  init1,
        mantf,
        zeroes,
        mant,   mantn,  mantd,
        exp1,   exp2,   exp3,
        end
    };

    number n_;
    short exp_; // exponent string as integer
    short dig_; // digits in mantissa
    short pos_; // position of decimal point
    short sig_; // significant digits in mantissa
    bool neg_;
    bool eneg_;
    state st_;

    inline
    void
    finish_double() noexcept;

public:
    number_parser()
        : st_(state::init)
    {
    }

    number
    get() const noexcept
    {
        return n_;
    }

    bool
    is_done() const noexcept
    {
        return st_ == state::end;
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
    finish(
        error_code& ec) noexcept;
};

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/number.ipp>
#endif

#endif
