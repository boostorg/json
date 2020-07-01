//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_RAW_STACK_IPP
#define BOOST_JSON_DETAIL_IMPL_RAW_STACK_IPP

#include <boost/json/detail/raw_stack.hpp>
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
        stack_overflow::raise();
    if( bytes < min_capacity_)
        bytes = min_capacity_;

    // 2x growth factor
    {
        if( capacity_ <=
            max_size() - capacity_)
        {
            auto hint =
                (capacity_ * 2) & ~1;
            if( bytes < hint)
                bytes = hint;
        }
        else
        {
            bytes = max_size();
        }
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
        stack_overflow::raise();
    reserve(capacity_ + n);
}

} // detail
} // json
} // boost

#endif
