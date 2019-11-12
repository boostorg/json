//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STRING_IMPL_IPP
#define BOOST_JSON_DETAIL_STRING_IMPL_IPP

#include <boost/json/detail/string_impl.hpp>
#include <boost/json/detail/except.hpp>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

string_impl::
string_impl(
    std::size_t size,
    storage_ptr const& sp)
{
    if(size < sizeof(buf))
    {
        // SBO
        capacity_ = sizeof(buf) - 1;
    }
    else
    {
        capacity_ = growth(size,
            sizeof(buf) - 1);
        p = static_cast<char*>(
            sp->allocate(capacity_ + 1, 1));
    }
    size_ = static_cast<
        std::uint32_t>(size);
}

std::uint32_t
string_impl::
growth(
    std::size_t new_size,
    std::size_t capacity)
{
    if(new_size > max_size())
        BOOST_JSON_THROW(
            string_too_large_exception());
    new_size |= mask_;
    if( new_size > max_size())
        return static_cast<
            std::uint32_t>(max_size());
    // growth factor 2
    if( capacity >
        max_size() - capacity)
        return static_cast<
            std::uint32_t>(max_size()); // overflow
    return static_cast<std::uint32_t>(
        (std::max)(capacity * 2, new_size));
}

void
string_impl::
destroy(
    storage_ptr const& sp) noexcept
{
    if(! in_sbo())
        sp->deallocate(
            p, capacity() + 1, 1);
}

char*
string_impl::
assign(
    std::size_t new_size,
    storage_ptr const& sp)
{
    if(new_size > capacity())
    {
        string_impl tmp(growth(
            new_size,
            capacity()), sp);
        destroy(sp);
        *this = tmp;
    }
    term(new_size);
    return data();
}

char*
string_impl::
append(
    std::size_t n,
    storage_ptr const& sp)
{
    if(n > max_size() - size())
        BOOST_JSON_THROW(
            string_too_large_exception());
    if(n <= capacity() - size())
    {
        term(size() + n);
        return end() - n;
    }
    string_impl tmp(growth(
        size() + n, capacity()), sp);
    std::memcpy(
        tmp.data(), data(), size());
    tmp.term(size() + n);
    destroy(sp);
    *this = tmp;
    return end() - n;
}

char*
string_impl::
insert(
    std::size_t pos,
    std::size_t n,
    storage_ptr const& sp)
{
    if(pos > size())
        BOOST_JSON_THROW(
            string_pos_too_large());
    if(n <= capacity() - size())
    {
        auto const dest =
            data() + pos;
        std::memmove(
            dest + n,
            dest,
            size() + 1 - pos);
        size(size() + n);
        return dest;
    }
    if(n > max_size() - size())
        BOOST_JSON_THROW(
            string_too_large_exception());
    string_impl tmp(growth(
        size() + n, capacity()), sp);
    tmp.size(size() + n);
    std::memcpy(
        tmp.data(),
        data(),
        pos);
    std::memcpy(
        tmp.data() + pos + n,
        data() + pos,
        size() + 1 - pos);
    destroy(sp);
    *this = tmp;
    return data() + pos;
}

void
string_impl::
unalloc(storage_ptr const& sp) noexcept
{
    BOOST_JSON_ASSERT(size() < sizeof(buf));
    BOOST_JSON_ASSERT(! in_sbo());
    auto const p_ = p;
    std::memcpy(
        buf, data(), size() + 1);
    sp->deallocate(
        p_, capacity() + 1, 1);
    capacity_ = sizeof(buf) - 1;
}

} // detail
} // json
} // boost

#endif
