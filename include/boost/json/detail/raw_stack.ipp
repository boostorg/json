//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_RAW_STACK_IPP
#define BOOST_JSON_DETAIL_RAW_STACK_IPP

#include <boost/json/detail/raw_stack.hpp>
#include <boost/json/detail/except.hpp>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

void
raw_stack::
reserve(std::size_t bytes)
{
    if(bytes <= capacity_)
        return;
    if(bytes > max_size())
        BOOST_THROW_EXCEPTION(
            stack_overflow_exception());
    if( bytes < min_capacity_)
        bytes = min_capacity_;
    if( capacity_ >
        max_size() - capacity_)
    {
        bytes = max_size();
    }
    else
    {
        // 2x growth factor
        auto hint =
            (capacity_ * 2) & ~1;
        if( bytes < hint)
            bytes = hint;
    }
    auto base = reinterpret_cast<
        char*>(sp_->allocate(bytes));
    if(base_)
    {
        std::memcpy(base, base_, size_);
        sp_->deallocate(base_, capacity_);
    }
    base_ = base;
    capacity_ = bytes;
}

void
raw_stack::
grow(std::size_t n)
{
    if(n > max_size() - capacity_)
        BOOST_THROW_EXCEPTION(
            stack_overflow_exception());
    reserve(capacity_ + n);
}

} // detail
} // json
} // boost

#endif
