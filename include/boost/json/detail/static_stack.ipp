//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STATIC_STACK_IPP
#define BOOST_JSON_DETAIL_STATIC_STACK_IPP

#include <boost/json/detail/static_stack.hpp>

namespace boost {
namespace json {
namespace detail {

template<class T, std::size_t N>
static_stack<T, N>::
~static_stack()
{
    if(begin_ != t_)
        sp_->deallocate(begin_,
            capacity() * sizeof(T),
            alignof(T));
}

template<class T, std::size_t N>
void
static_stack<T, N>::
grow()
{
    auto const n = 2 * capacity();
    auto const begin =
        reinterpret_cast<T*>(
            sp_->allocate(
                n * sizeof(T),
                alignof(T)));
    if(! empty())
    {
        std::memcpy(begin, begin_,
            size() * sizeof(T));
        top_ = begin + (top_ - begin_);
        if(begin_ != t_)
            sp_->deallocate(begin_,
                capacity() * sizeof(T),
                alignof(T));
    }
    else
    {
        top_ = begin;
    }
    begin_ = begin;
    end_ = begin_ + n;
}

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/static_stack.ipp>
#endif

#endif
