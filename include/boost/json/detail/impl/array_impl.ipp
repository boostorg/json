//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_ARRAY_IMPL_IPP
#define BOOST_JSON_DETAIL_IMPL_ARRAY_IMPL_IPP

#include <boost/json/detail/array_impl.hpp>

namespace boost {
namespace json {
namespace detail {

array_impl::
array_impl(
    std::size_t capacity,
    storage_ptr const& sp)
{
    if(capacity > max_size())
        array_too_large::raise();
    if(capacity > 0)
    {
        tab_ = ::new(sp->allocate(
            (sizeof(table) +
             capacity * sizeof(value) +
             sizeof(table) + 1)
                / sizeof(table)
                * sizeof(table),
            (std::max)(
                alignof(table),
                alignof(value)))) table;
        tab_->capacity = static_cast<
            std::uint32_t>(capacity);
        tab_->size = 0;
    }
    else
    {
        tab_ = nullptr;
    }
}


array_impl::
array_impl(array_impl&& other) noexcept
    : tab_(detail::exchange(
        other.tab_, nullptr))
{
}

auto
array_impl::
operator=(
    array_impl&& other) noexcept ->
        array_impl&
{
    ::new(this) array_impl(
        std::move(other));
    return *this;
}

void
array_impl::
swap(array_impl& rhs) noexcept
{
    auto tmp = tab_;
    tab_ = rhs.tab_;
    rhs.tab_ = tmp;
}

void
array_impl::
destroy_impl(
    storage_ptr const& sp) noexcept
{
    auto it = data() + tab_->size;
    while(it != data())
        (*--it).~value();
    sp->deallocate(tab_,
        (sizeof(table) +
            capacity() * sizeof(value) +
            sizeof(table) + 1)
            / sizeof(table)
            * sizeof(table),
        (std::max)(
            alignof(table),
            alignof(value)));
}

void
array_impl::
destroy(
    storage_ptr const& sp) noexcept
{
    if(tab_ && sp->need_free())
        destroy_impl(sp);
}

} // detail
} // json
} // boost

#endif
