//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_STACK_IPP
#define BOOST_JSON_DETAIL_IMPL_STACK_IPP

#include <boost/json/detail/stack.hpp>

namespace boost {
namespace json {
namespace detail {

stack::
non_trivial::
~non_trivial() = default;

stack::
~stack()
{
    clear();
    if(base_ != buf_)
        sp_->deallocate(
            base_, cap_);
}

stack::
stack(
    storage_ptr sp,
    unsigned char* buf,
    std::size_t buf_size) noexcept
    : sp_(std::move(sp))
    , cap_(buf_size)
    , base_(buf)
    , buf_(buf)
{
}

void
stack::
clear() noexcept
{
    while(head_)
    {
        auto const next = head_->next;
        head_->~non_trivial();
        head_ = next;
    }
    size0_ = 0;
    size1_ = 0;
}

void
stack::
reserve_impl(
    std::size_t n)
{
    // caller checks this
    BOOST_ASSERT(n > cap_);

    auto const base =
        static_cast<unsigned char*>(
            sp_->allocate(n));
    if(base_)
    {
        // copy trivials
        if(size1_ > 0)
            std::memcpy(
                base + n - size1_,
                base_ + cap_ - size1_,
                size1_);

        // copy non-trivials
        if(head_)
        {
            non_trivial* head = nullptr;
            auto dest = reinterpret_cast<
                non_trivial*>(base);
            auto next = head_->next;
            auto prev = dest;
            dest = head_->copy(dest);
            prev->next = head;
            head = prev;
            head_->~non_trivial();
            head_ = next;
            while(head_)
            {
                next = head_->next;
                prev = dest;
                dest = head_->copy(dest);
                prev->next = head;
                head = prev;
                head_->~non_trivial();
                head_ = next;
            }
            head_ = head;
        }

        if(base_ != buf_)
            sp_->deallocate(base_, cap_);
    }
    base_ = base;
    cap_ = n;
}

} // detail
} // json
} // boost

#endif