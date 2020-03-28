//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
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
    using index_t = std::uint32_t;

    static
    inline
    index_t&
    get(key_value_pair& e) noexcept
    {
        return e.next_;
    }

    static
    inline
    index_t
    get(key_value_pair const& e) noexcept
    {
        return e.next_;
    }
};

void
object_impl::
remove(
    index_t& head,
    value_type& p) noexcept
{
    auto const i = index_of(p);
    if(head == i)
    {
        head = next(p);
        return;
    }
    auto prev = head;
    auto* pn = &next(get(prev));
    for(;;)
    {
        if(*pn == i)
            break;
        prev = *pn;
    }
    *pn = next(p);
}

auto
object_impl::
bucket(string_view key) const noexcept ->
    index_t&
{
    auto const hash = digest(key);
    auto const i = hash % buckets();
    return bucket_begin()[i];
}

auto
object_impl::
bucket(std::size_t hash) const noexcept ->
    index_t&
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
get(index_t i) const noexcept ->
    value_type&
{
    BOOST_ASSERT(i != null_index);
    return (reinterpret_cast<
        value_type*>(tab_ + 1))[i];
}

auto
object_impl::
index_of(value_type const& p) const noexcept ->
    index_t
{
    return static_cast<index_t>(
        &p - reinterpret_cast<
            value_type const*>(tab_ + 1));
}

auto
object_impl::
bucket_begin() const noexcept ->
    index_t*
{
    return reinterpret_cast<
        index_t*>(
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
    index_t&
{
    return next_access::get(e);
}

auto
object_impl::
next(value_type const& e) noexcept ->
    index_t
{
    return next_access::get(e);
}

//----------------------------------------------------------

unchecked_object::
~unchecked_object()
{
    if(data_)
        destroy(data_, size_);
}

void
unchecked_object::
relocate(object::value_type* dest) noexcept
{
    if(size_ > 0)
        std::memcpy(
            static_cast<void*>(dest), data_,
            size_ * sizeof(object::value_type));
    data_ = nullptr;
}

} // detail
} // json
} // boost

#endif
