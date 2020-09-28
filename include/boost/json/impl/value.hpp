//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_VALUE_HPP
#define BOOST_JSON_IMPL_VALUE_HPP

#include <boost/json/detail/except.hpp>
#include <cstring>
#include <limits>
#include <type_traits>

BOOST_JSON_NS_BEGIN

value::
scalar::
scalar(
    storage_ptr sp_) noexcept
    : sp(std::move(sp_))
    , k(json::kind::null)
{
}

value::
scalar::
scalar(
    bool b_,
    storage_ptr sp_) noexcept
    : sp(std::move(sp_))
    , k(json::kind::bool_)
    , b(b_)
{
}

value::
scalar::
scalar(
    std::int64_t i_,
    storage_ptr sp_) noexcept
    : sp(std::move(sp_))
    , k(json::kind::int64)
    , i(i_)
{
}

value::
scalar::
scalar(
    std::uint64_t u_,
    storage_ptr sp_) noexcept
    : sp(std::move(sp_))
    , k(json::kind::uint64)
    , u(u_)
{
}

value::
scalar::
scalar(
    double d_,
    storage_ptr sp_) noexcept
    : sp(std::move(sp_))
    , k(json::kind::double_)
    , d(d_)
{
}

//----------------------------------------------------------

value::
value(detail::unchecked_object&& uo)
    : obj_(std::move(uo))
{
}

value::
value(detail::unchecked_array&& ua)
    : arr_(std::move(ua))
{
}

value::
value(
    char** key,
    std::size_t len,
    storage_ptr sp)
    : str_(key, len, std::move(sp))
{
}

//----------------------------------------------------------
//
// Assignment
//
//----------------------------------------------------------

value&
value::
operator=(std::nullptr_t) noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.k = json::kind::null;
    }
    else
    {
        ::new(&sca_) scalar(
            destroy());
    }
    return *this;
}

template<class Bool, class>
value&
value::
operator=(Bool b) noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.b = b;
        sca_.k = json::kind::bool_;
    }
    else
    {
        ::new(&sca_) scalar(
            b, destroy());
    }
    return *this;
}

value&
value::
operator=(short i) noexcept
{
    return operator=(
        static_cast<long long>(i));
}

value&
value::
operator=(int i) noexcept
{
    return operator=(
        static_cast<long long>(i));
}


value&
value::
operator=(long i) noexcept
{
    return operator=(
        static_cast<long long>(i));
}

value&
value::
operator=(long long i) noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.i = i;
        sca_.k = json::kind::int64;
    }
    else
    {
        ::new(&sca_) scalar(static_cast<
            std::int64_t>(i), destroy());
    }
    return *this;
}

value&
value::
operator=(unsigned short u) noexcept
{
    return operator=(static_cast<
        unsigned long long>(u));
}

value&
value::
operator=(unsigned int u) noexcept
{
    return operator=(static_cast<
        unsigned long long>(u));
}

value&
value::
operator=(unsigned long u) noexcept
{
    return operator=(static_cast<
        unsigned long long>(u));
}

value&
value::
operator=(unsigned long long u) noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.u = u;
        sca_.k = json::kind::uint64;
    }
    else
    {
        ::new(&sca_) scalar(static_cast<
            std::uint64_t>(u), destroy());
    }
    return *this;
}

value&
value::
operator=(double d) noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.d = d;
        sca_.k = json::kind::double_;
    }
    else
    {
        ::new(&sca_) scalar(
            d, destroy());
    }
    return *this;
}

value&
value::
operator=(long double d) noexcept
{
    return operator=(
        static_cast<double>(d));
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

void
value::
emplace_null() noexcept
{
    if(kind() < json::kind::string)
        sca_.k = json::kind::null;
    else
        ::new(&sca_) scalar(destroy());
}

bool&
value::
emplace_bool() noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.k = json::kind::bool_;
        sca_.b = false;
    }
    else
    {
        ::new(&sca_) scalar(
            false, destroy());
    }
    return sca_.b;
}

std::int64_t&
value::
emplace_int64() noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.k = json::kind::int64;
        sca_.i = 0;
    }
    else
    {
        ::new(&sca_) scalar(
            std::int64_t(0),
            destroy());
    }
    return sca_.i;
}

std::uint64_t&
value::
emplace_uint64() noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.k = json::kind::uint64;
        sca_.u = 0;
    }
    else
    {
        ::new(&sca_) scalar(
            std::uint64_t(0),
            destroy());
    }
    return sca_.u;
}

double&
value::
emplace_double() noexcept
{
    if(kind() < json::kind::string)
    {
        sca_.k = json::kind::double_;
        sca_.d = 0;
    }
    else
    {
        ::new(&sca_) scalar(
            0.0,
            destroy());
    }
    return sca_.d;
}

//----------------------------------------------------------
//
// private
//
//----------------------------------------------------------

void
value::
relocate(
    value* dest,
    value const& src) noexcept
{
    std::memcpy(
        reinterpret_cast<
            void*>(dest),
        &src,
        sizeof(src));
}

//----------------------------------------------------------
//
// key_value_pair
//
//----------------------------------------------------------

template<class... Args>
key_value_pair::
key_value_pair(
    string_view key,
    Args&&... args)
    : value_(std::forward<Args>(args)...)
    , key_(
        [&]
        {
            if(key.size() > string::max_size())
                detail::throw_length_error(
                    "key too large",
                    BOOST_CURRENT_LOCATION);
            auto s = reinterpret_cast<
                char*>(value_.storage()->
                    allocate(key.size() + 1));
            std::memcpy(s, key.data(), key.size());
            s[key.size()] = 0;
            return s;
        }())
    , len_(static_cast<
        std::uint32_t>(key.size()))
{
}

//----------------------------------------------------------

BOOST_JSON_NS_END

#endif
