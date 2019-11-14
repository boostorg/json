//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_IPP
#define BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_IPP

namespace boost {
namespace json {
namespace detail {

void
object_impl::
do_destroy(storage_ptr const& sp) noexcept
{
    if(tab_)
    {
        value_type::destroy(
            begin(), size());
        sp->deallocate(tab_,
            sizeof(table) +
            capacity() * sizeof(value_type) +
            buckets() * sizeof(value_type*));
    }
}

inline
object_impl::
object_impl(
    std::size_t capacity,
    std::size_t buckets,
    storage_ptr const& sp)
{
    tab_ = ::new(sp->allocate(
        sizeof(table) +
        capacity * sizeof(value_type) +
        buckets * sizeof(value_type*))) table{
            0, capacity, buckets };
    std::memset(bucket_begin(), 0,
        buckets * sizeof(value_type*));
}

object_impl::
object_impl(object_impl&& other) noexcept
    : tab_(detail::exchange(
        other.tab_, nullptr))
{
}

void
object_impl::
clear() noexcept
{
    if(! tab_)
        return;
    value_type::destroy(
        begin(), size());
    std::memset(bucket_begin(), 0,
        buckets() * sizeof(value_type*));
    tab_->size = 0;
}

// checks for dupes
void
object_impl::
build() noexcept
{
    auto end = this->end();
    for(auto p = begin(); p != end;)
    {
        {
            auto& head = bucket(p->key());
            auto it = head;
            while(it && it->key() != p->key())
                it = next(*it);
            if(! it)
            {
                next(*p) = head;
                head = p;
                ++p;
                continue;
            }
        }
        p->~value_type();
        --tab_->size;
        --end;
        if(p != end)
        {
            std::memcpy(
                reinterpret_cast<void*>(p),
                reinterpret_cast<void const*>(end),
                sizeof(*p));
            auto& head = bucket(p->key());
            next(*p) = head;
            head = p;
        }
    }
}

// does not check for dupes
void
object_impl::
rebuild() noexcept
{
    auto const end = this->end();
    for(auto it = begin();
        it != end; ++it)
    {
        auto& head = bucket(it->key());
        next(*it) = head;
        head = it;
    }
}

void
object_impl::
swap(object_impl& rhs) noexcept
{
    auto tmp = tab_;
    tab_ = rhs.tab_;
    rhs.tab_ = tmp;
}

} // detail
} // json
} // boost

#endif
