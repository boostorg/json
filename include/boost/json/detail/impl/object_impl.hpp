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
    constexpr static std::size_t list[] = 
    {
        0ULL,

        3ULL,                     7ULL,
        11ULL,                    17ULL,
        29ULL,                    53ULL,
        97ULL,                    193ULL,
        389ULL,                   769ULL,
        1543ULL,                  3079ULL,
        6151ULL,                  12289ULL,
        24593ULL,                 49157ULL,
        98317ULL,                 196613ULL,
        393241ULL,                786433ULL,
        1572869ULL,               3145739ULL,
        6291469ULL,               12582917ULL,
        25165843ULL,              50331653ULL,
        100663319ULL,             201326611ULL,
        402653189ULL,             805306457ULL,
        1610612741ULL,            3221225473ULL,
#if BOOST_JSON_ARCH == 32
        4294967295ULL
#else
        6442450939ULL,            12884901893ULL,
        25769803751ULL,           51539607551ULL,
        103079215111ULL,          206158430209ULL,
        412316860441ULL,          824633720831ULL,
        1649267441651ULL,         3298534883309ULL,
        6597069766657ULL,         13194139533299ULL,
        26388279066623ULL,        52776558133303ULL,
        105553116266489ULL,       211106232532969ULL,
        422212465066001ULL,       844424930131963ULL,
        1688849860263953ULL,      3377699720527861ULL,
        6755399441055731ULL,      13510798882111483ULL,
        27021597764222939ULL,     54043195528445957ULL,
        108086391056891903ULL,    216172782113783843ULL,
        432345564227567621ULL,    864691128455135207ULL,
        1729382256910270481ULL,   3458764513820540933ULL,
        6917529027641081903ULL,   13835058055282163729ULL,
        18446744073709551557ULL,  18446744073709551615ULL
#endif
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
        default:
        case 0: return 0;
        case 1: return hash % bucket_sizes()[1];
        case 2: return hash % bucket_sizes()[2];
        case 3: return hash % bucket_sizes()[3];
        case 4: return hash % bucket_sizes()[4];
        case 5: return hash % bucket_sizes()[5];
        case 6: return hash % bucket_sizes()[6];
        case 7: return hash % bucket_sizes()[7];
        case 8: return hash % bucket_sizes()[8];
        case 9: return hash % bucket_sizes()[9];
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
        case 33: return hash % bucket_sizes()[33];
#if BOOST_JSON_ARCH == 64
        case 34: return hash % bucket_sizes()[34];
        case 35: return hash % bucket_sizes()[35];
        case 36: return hash % bucket_sizes()[36];
        case 37: return hash % bucket_sizes()[37];
        case 38: return hash % bucket_sizes()[38];
        case 39: return hash % bucket_sizes()[39];
        case 40: return hash % bucket_sizes()[40];
        case 41: return hash % bucket_sizes()[41];
        case 42: return hash % bucket_sizes()[42];
        case 43: return hash % bucket_sizes()[43];
        case 44: return hash % bucket_sizes()[44];
        case 45: return hash % bucket_sizes()[45];
        case 46: return hash % bucket_sizes()[46];
        case 47: return hash % bucket_sizes()[47];
        case 48: return hash % bucket_sizes()[48];
        case 49: return hash % bucket_sizes()[49];
        case 50: return hash % bucket_sizes()[50];
        case 51: return hash % bucket_sizes()[51];
        case 52: return hash % bucket_sizes()[52];
        case 53: return hash % bucket_sizes()[53];
        case 54: return hash % bucket_sizes()[54];
        case 55: return hash % bucket_sizes()[55];
        case 56: return hash % bucket_sizes()[56];
        case 57: return hash % bucket_sizes()[57];
        case 58: return hash % bucket_sizes()[58];
        case 59: return hash % bucket_sizes()[59];
        case 60: return hash % bucket_sizes()[60];
        case 61: return hash % bucket_sizes()[61];
        case 62: return hash % bucket_sizes()[62];
        case 63: return hash % bucket_sizes()[63];
        case 64: return hash % bucket_sizes()[64];
        case 65: return hash % bucket_sizes()[65];
        case 66: return hash % bucket_sizes()[66];
#endif
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
