//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STACK_HPP
#define BOOST_JSON_DETAIL_STACK_HPP

#include <boost/json/config.hpp>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

class stack
{
    char buf_[1024];
    char* top_;

public:
    stack() noexcept
        : top_(buf_)
    {
    }

    bool
    empty() const noexcept
    {
        return top_ == buf_;
    }

    void
    clear() noexcept
    {
        top_ = buf_;
    }

    template<class T>
    void
    push(T const& t)
    {
        auto const n = sizeof(t);
        std::memcpy(top_, &t, n);
        top_ += n;
    }

    template<class T>
    void
    peek(T& t)
    {
        auto const n = sizeof(t);
        BOOST_ASSERT(top_ >= buf_ + n);
        std::memcpy(&t, top_ - n, n);
    }

    template<class T>
    void
    pop(T& t)
    {
        auto const n = sizeof(t);
        BOOST_ASSERT(top_ >= buf_ + n);
        top_ -= n;
        std::memcpy(&t, top_, n);
    }
};

} // detail
} // json
} // boost

#endif
