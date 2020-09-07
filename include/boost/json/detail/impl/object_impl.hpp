//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_HPP
#define BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_HPP

#include <boost/json/detail/digest.hpp>
#include <boost/json/detail/object_impl.hpp>

BOOST_JSON_NS_BEGIN
namespace detail {

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
    auto* pn = &next(get(head));
    while(*pn != i)
        pn = &next(get(*pn));
    *pn = next(p);
}

auto
object_impl::
bucket_sizes() noexcept ->
    bucket_size_array&
{
    // Taken from Boost.Intrusive and Boost.MultiIndex code,
    // thanks to Ion Gaztanaga and Joaquin M Lopez Munoz.
    static constexpr std::size_t list[33] =
    {
        0,

        3,                     7,
        11,                    17,
        29,                    53,
        97,                    193,
        389,                   769,
        1543,                  3079,
        6151,                  12289,
        24593,                 49157,
        98317,                 196613,
        393241,                786433,
        1572869,               3145739,
        6291469,               12582917,
        25165843,              50331653,
        100663319,             201326611,
        402653189,             805306457,
        1610612741,
        BOOST_JSON_MAX_STRUCTURED_SIZE // 3221225473
    };
    return list;
}

std::size_t
object_impl::
bucket_index(
    std::size_t hash, 
    std::size_t index) noexcept
{
    // Suggested by Joaquin M Lopez Munoz
    // https://github.com/CPPAlliance/json/issues/115
    switch(index)
    {
        default: unreachable();
        case  0: return 0;
        case  1: return hash % bucket_sizes()[1];
        case  2: return hash % bucket_sizes()[2];
        case  3: return hash % bucket_sizes()[3];
        case  4: return hash % bucket_sizes()[4];
        case  5: return hash % bucket_sizes()[5];
        case  6: return hash % bucket_sizes()[6];
        case  7: return hash % bucket_sizes()[7];
        case  8: return hash % bucket_sizes()[8];
        case  9: return hash % bucket_sizes()[9];
        case 10: return hash % bucket_sizes()[10];
        case 11: return hash % bucket_sizes()[11];
        case 12: return hash % bucket_sizes()[12];
        case 13: return hash % bucket_sizes()[13];
        case 14: return hash % bucket_sizes()[14];
        case 15: return hash % bucket_sizes()[15];
        case 16: return hash % bucket_sizes()[16];
        case 17: return hash % bucket_sizes()[17];
        case 18: return hash % bucket_sizes()[18];
        case 19: return hash % bucket_sizes()[19];
        case 20: return hash % bucket_sizes()[20];
        case 21: return hash % bucket_sizes()[21];
        case 22: return hash % bucket_sizes()[22];
        case 23: return hash % bucket_sizes()[23];
        case 24: return hash % bucket_sizes()[24];
        case 25: return hash % bucket_sizes()[25];
        case 26: return hash % bucket_sizes()[26];
        case 27: return hash % bucket_sizes()[27];
        case 28: return hash % bucket_sizes()[28];
        case 29: return hash % bucket_sizes()[29];
        case 30: return hash % bucket_sizes()[30];
        case 31: return hash % bucket_sizes()[31];
        case 32: return hash % bucket_sizes()[32];
    }
}

auto
object_impl::
bucket(string_view key) const noexcept ->
    index_t&
{
    auto const hash = digest(key);
    return bucket_begin()[bucket_index(
        hash, tab_->prime_index)];
}

auto
object_impl::
bucket(std::size_t hash) const noexcept ->
    index_t&
{
    return bucket_begin()[bucket_index(
        hash, tab_->prime_index)];
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
digest(string_view key) const noexcept
{
    return detail::digest(
        key.data(), key.size(), salt());
}

//----------------------------------------------------------

auto
object_impl::
next(value_type& e) noexcept ->
    index_t&
{
    return value_access::next(e);
}

auto
object_impl::
next(value_type const& e) noexcept ->
    index_t
{
    return value_access::next(e);
}

//----------------------------------------------------------

unchecked_object::
~unchecked_object()
{
    if( data_ &&
        ! sp_.is_not_counted_and_deallocate_is_trivial())
    {
        value* p = data_;
        while(size_--)
        {
            p[0].~value();
            p[1].~value();
            p += 2;
        }
    }
}

} // detail
BOOST_JSON_NS_END

#endif
