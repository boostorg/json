//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_STRING_IMPL_IPP
#define BOOST_JSON_DETAIL_IMPL_STRING_IMPL_IPP

#include <boost/json/detail/string_impl.hpp>
#include <boost/json/detail/except.hpp>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

string_impl::
string_impl() noexcept
{
    s_.k = short_string_;
    s_.buf[sbo_chars_] =
        static_cast<char>(
            sbo_chars_);
    s_.buf[0] = 0;
}

string_impl::
string_impl(
    std::size_t size,
    storage_ptr const& sp)
{
    if(size <= sbo_chars_)
    {
        s_.k = short_string_;
        s_.buf[sbo_chars_] =
            static_cast<char>(
                sbo_chars_ - size);
        s_.buf[size] = 0;
    }
    else
    {
        s_.k = kind::string;
        auto const n = growth(
            size, sbo_chars_ + 1);
        p_.t = ::new(sp->allocate(
            sizeof(table) +
                n + 1,
            alignof(table))) table{
                static_cast<
                    std::uint32_t>(size),
                static_cast<
                    std::uint32_t>(n)};
        term(size);
    }
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
    // growth factor 2
    if( capacity >
        max_size() - capacity)
        return static_cast<
            std::uint32_t>(max_size()); // overflow
    return static_cast<std::uint32_t>(
        (std::max)(capacity * 2, new_size));
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
shrink_to_fit(
    storage_ptr const& sp) noexcept
{
    if(s_.k == short_string_)
        return;
    auto const t = p_.t;
    if(t->size <= sbo_chars_)
    {
        s_.k = short_string_;
        std::memcpy(
            s_.buf, data(), t->size);
        s_.buf[sbo_chars_] =
            static_cast<char>(
                sbo_chars_ - t->size);
        s_.buf[t->size] = 0;
        sp->deallocate(t,
            sizeof(table) +
                t->capacity + 1,
            alignof(table));
        return;
    }
    if(t->size >= t->capacity)
        return;
#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        string_impl tmp(t->size, sp);
        std::memcpy(
            tmp.data(),
            data(),
            size());
        destroy(sp);
        *this = tmp;
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch(std::exception const&)
    {
        // eat the exception
    }
#endif
}

} // detail
} // json
} // boost

#endif
