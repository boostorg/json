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
grow(std::size_t n)
{
    if(n > max_size() - capacity_)
        BOOST_JSON_THROW(
            stack_overflow_exception());
    auto new_capacity = capacity_ + n;
    if( new_capacity < min_capacity_)
        new_capacity = min_capacity_;
    // 2x growth
    auto const hint = (capacity_ * 2) & ~1;
    if( new_capacity < hint)
        new_capacity = hint;
    auto base = reinterpret_cast<
        char*>(sp_->allocate(new_capacity));
    if(base_)
    {
        std::memcpy(base, base_, size_);
        sp_->deallocate(base_, capacity_);
    }
    base_ = base;
    capacity_ = new_capacity;
}

} // detail
} // json
} // boost

#endif
