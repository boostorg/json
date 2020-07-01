//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_VALUE_IPP
#define BOOST_JSON_IMPL_VALUE_IPP

#include <boost/json/value.hpp>
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

value::
~value()
{
    switch(kind())
    {
    case json::kind::object:
        obj_.~object();
        break;

    case json::kind::array:
        arr_.~array();
        break;

    case json::kind::string:
        str_.~string();
        break;

    case json::kind::int64:
        i64_.~int64_k();
        break;

    case json::kind::uint64:
        u64_.~uint64_k();
        break;

    case json::kind::double_:
        dub_.~double_k();
        break;

    case json::kind::bool_:
        bln_.~bool_k();
        break;

    case json::kind::null:
        nul_.~null_k();
        break;
    }
}

value::
value(pilfered<value> p) noexcept
{
    relocate(this, p.get());
    ::new(&p.get().nul_) null_k;
}

value::
value(
    value const& other,
    storage_ptr sp)
{
    switch(other.kind())
    {
    case json::kind::object:
        ::new(&obj_) object(
            other.obj_,
            detail::move(sp));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            other.arr_,
            detail::move(sp));
        break;

    case json::kind::string:
        ::new(&str_) string(
            other.str_,
            detail::move(sp));
        break;

    case json::kind::int64:
        ::new(&i64_) int64_k(
            other.i64_.i,
            detail::move(sp));
        break;

    case json::kind::uint64:
        ::new(&u64_) uint64_k(
            other.u64_.u,
            detail::move(sp));
        break;

    case json::kind::double_:
        ::new(&dub_) double_k(
            other.dub_.d,
            detail::move(sp));
        break;

    case json::kind::bool_:
        ::new(&bln_) bool_k(
            other.bln_.b,
            detail::move(sp));
        break;

    case json::kind::null:
        ::new(&nul_) null_k(
            detail::move(sp));
        break;
    }
}

value::
value(value&& other) noexcept
{
    relocate(this, other);
    ::new(&other.nul_) null_k(sp_);
}

value::
value(
    value&& other,
    storage_ptr sp)
{
    switch(other.kind())
    {
    case json::kind::object:
        ::new(&obj_) object(
            detail::move(other.obj_),
            detail::move(sp));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            detail::move(other.arr_),
            detail::move(sp));
        break;

    case json::kind::string:
        ::new(&str_) string(
            detail::move(other.str_),
            detail::move(sp));
        break;

    case json::kind::int64:
        ::new(&i64_) int64_k(
            other.i64_.i, detail::move(sp));
        break;

    case json::kind::uint64:
        ::new(&u64_) uint64_k(
            other.u64_.u, detail::move(sp));
        break;

    case json::kind::double_:
        ::new(&dub_) double_k(
            other.dub_.d, detail::move(sp));
        break;

    case json::kind::bool_:
        ::new(&bln_) bool_k(
            other.bln_.b, detail::move(sp));
        break;

    case json::kind::null:
        ::new(&nul_) null_k(
            detail::move(sp));
        break;
    }
}

value&
value::
operator=(value&& other)
{
    undo u(this);
    ::new(this) value(
        detail::move(other),
        u.saved.storage());
    u.commit();
    return *this;
}

value&
value::
operator=(value const& other)
{
    if(this == &other)
        return *this;

    undo u(this);
    ::new(this) value(other,
        u.saved.storage());
    u.commit();
    return *this;
}

//----------------------------------------------------------
//
// Conversion
//
//----------------------------------------------------------

value::
value(
    std::initializer_list<value_ref> init,
    storage_ptr sp)
{
    if(value_ref::maybe_object(init))
        ::new(&obj_) object(
            value_ref::make_object(
                init, std::move(sp)));
    else
        ::new(&arr_) array(
            value_ref::make_array(
                init, std::move(sp)));
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

object&
value::
emplace_object() noexcept
{
    return *::new(&obj_) object(destroy());
}

array&
value::
emplace_array() noexcept
{
    return *::new(&arr_) array(destroy());
}

string&
value::
emplace_string() noexcept
{
    return *::new(&str_) string(destroy());
}

std::int64_t&
value::
emplace_int64() noexcept
{
    ::new(&i64_) int64_k(destroy());
    return i64_.i;
}

std::uint64_t&
value::
emplace_uint64() noexcept
{
    ::new(&u64_) uint64_k(destroy());
    return u64_.u;
}

double&
value::
emplace_double() noexcept
{
    ::new(&dub_) double_k(destroy());
    return dub_.d;
}

bool&
value::
emplace_bool() noexcept
{
    ::new(&bln_) bool_k(destroy());
    return bln_.b;
}

void
value::
emplace_null() noexcept
{
    ::new(&nul_) null_k(destroy());
}

void
value::
swap(value& other)
{
    if(*storage() != *other.storage())
    {
        // copy
        value temp1(
            detail::move(*this),
            other.storage());
        value temp2(
            detail::move(other),
            this->storage());
        other.~value();
        ::new(&other) value(pilfer(temp1));
        this->~value();
        ::new(this) value(pilfer(temp2));
        return;
    }

    union U
    {
        value tmp;
        U(){}
        ~U(){}
    };
    U u;
    relocate(&u.tmp, *this);
    relocate(this, other);
    relocate(&other, u.tmp);
}

//----------------------------------------------------------
//
// Accessors
//
//----------------------------------------------------------

storage_ptr
value::
destroy() noexcept
{
    switch(kind())
    {
    case json::kind::object:
    {
        auto sp = obj_.storage();
        obj_.~object();
        return sp;
    }
    case json::kind::array:
    {
        auto sp = arr_.storage();
        arr_.~array();
        return sp;
    }
    case json::kind::string:
    {
        auto sp = str_.storage();
        str_.~string();
        return sp;
    }
    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::bool_:
    case json::kind::null:
        break;
    }
    return std::move(sp_);
}

//----------------------------------------------------------

key_value_pair::
~key_value_pair()
{
    auto const& sp = value_.storage();
    if(sp.is_not_counted_and_deallocate_is_null())
        return;
    sp->deallocate(key_, len_ + 1, 1);
}

key_value_pair::
key_value_pair(
    key_value_pair const& other)
    : value_(other.value_)
    , key_(
        [&]
        {
            auto s = reinterpret_cast<
                char*>(value_.storage()->
                    allocate(other.len_ + 1));
            std::memcpy(s, other.key_, other.len_);
            s[other.len_] = 0;
            return s;
        }())
    , len_(other.len_)
{
}

key_value_pair::
key_value_pair::
key_value_pair(
    key_value_pair const& other,
    storage_ptr sp)
    : value_(other.value_, detail::move(sp))
    , key_(
        [&]
        {
            auto s = reinterpret_cast<
                char*>(value_.storage()->
                    allocate(other.len_ + 1));
            std::memcpy(s, other.key_, other.len_);
            s[other.len_] = 0;
            return s;
        }())
    , len_(other.len_)
{
}

//----------------------------------------------------------

} // json
} // boost

#endif
