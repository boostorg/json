//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_HPP
#define BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_HPP

#include <boost/json/detail/object_impl.hpp>

namespace boost {
namespace json {
namespace detail {

struct next_access
{
    static
    inline
    object_value_type*&
    get(object_value_type& e) noexcept
    {
        return e.next_;
    }

    static
    inline
    object_value_type const*
    get(object_value_type const& e) noexcept
    {
        return e.next_;
    }
};

void
object_impl::
remove(
    value_type*& head,
    value_type* p) noexcept
{
    if(head == p)
    {
        head = next(*head);
        return;
    }
    auto prev = head;
    while(next(*prev) != p)
        prev = next(*prev);
    next(*prev) = next(*p);
}

auto
object_impl::
bucket(string_view key) const noexcept ->
    value_type*&
{
    auto const hash = digest(key);
    auto const i = hash % buckets();
    return bucket_begin()[i];
}

auto
object_impl::
bucket(std::size_t hash) const noexcept ->
    value_type*&
{
    return bucket_begin()[hash % buckets()];
}

auto
object_impl::
begin() const noexcept ->
    value_type*
{
    if(! tab_)
        return nullptr;
    return reinterpret_cast<
        value_type*>(tab_ + 1);
}

auto
object_impl::
end() const noexcept ->
    value_type*
{
    return begin() + size();
}

auto
object_impl::
bucket_begin() const noexcept ->
    value_type**
{
    return reinterpret_cast<
        value_type**>(
            begin() + capacity());
}

std::size_t
object_impl::
digest(string_view key) noexcept
{
    return digest(key,
        std::integral_constant<bool,
            sizeof(std::size_t) ==
            sizeof(std::uint64_t)>{});
}

//----------------------------------------------------------

std::uint32_t
object_impl::
digest(
    string_view key,
    std::false_type) noexcept
{
    std::uint32_t prime = 0x01000193UL;
    std::uint32_t hash  = 0x811C9DC5UL;
    for(auto p = key.begin(),
        end = key.end(); p != end; ++p)
        hash = (*p ^ hash) * prime;
    return hash;
}

std::uint64_t
object_impl::
digest(
    string_view key,
    std::true_type) noexcept
{
    std::uint64_t prime = 0x100000001B3ULL;
    std::uint64_t hash  = 0xcbf29ce484222325ULL;
    for(auto p = key.begin(),
        end = key.end(); p != end; ++p)
        hash = (*p ^ hash) * prime;
    return hash;
}

auto
object_impl::
next(value_type& e) noexcept ->
    value_type*&
{
    return next_access::get(e);
}

auto
object_impl::
next(value_type const& e) noexcept ->
    value_type const*
{
    return next_access::get(e);
}

//----------------------------------------------------------

unchecked_object::
~unchecked_object()
{
    if(data_)
        object::value_type::destroy(
            data_, size_);
}

void
unchecked_object::
relocate(object::value_type* dest) noexcept
{
    if(size_ > 0)
        std::memcpy(
            reinterpret_cast<void*>(dest),
            data_, size_ * sizeof(object::value_type));
    data_ = nullptr;
}

} // detail
} // json
} // boost

#endif
