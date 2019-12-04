//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_IPP
#define BOOST_JSON_IMPL_VALUE_IPP

#include <boost/json/value.hpp>
#include <boost/config.hpp>
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

struct value::init_iter
{
    using value_type = std::pair<
        string_view const, value const&>;
    using pointer = value_type const*;
    using reference = value_type const&;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    std::initializer_list<
        value>::iterator it_;

    init_iter&
    operator++() noexcept
    {
        ++it_;
        return *this;
    }

    init_iter
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++it_;
        return tmp;
    }

    value_type
    operator*() const noexcept
    {
        return {
            (*it_->if_array())[0].as_string(),
            (*it_->if_array())[1] };
    }

    bool
    operator==(
        init_iter const& other) const noexcept
    {
        return it_ == other.it_;
    }

    bool
    operator!=(
        init_iter const& other) const noexcept
    {
        return it_ != other.it_;
    }

    BOOST_STATIC_ASSERT(
        std::is_constructible<
            object::value_type,
            init_iter::value_type>::value);
};

//----------------------------------------------------------

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
    std::initializer_list<value> init,
    storage_ptr sp)
{
    if(maybe_object(init))
    {
        if(init.size() > object::max_size())
            BOOST_JSON_THROW(
                detail::object_too_large_exception());
        ::new(&obj_) object(
            init_iter{init.begin()},
            init_iter{init.end()},
            init.size(),
            detail::move(sp));
    }
    else
    {
        ::new(&arr_) array(
            init, detail::move(sp));
    }
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
    ::new(&obj_) object(destroy());
    return obj_;
}

array&
value::
emplace_array() noexcept
{
    ::new(&arr_) array(destroy());
    return arr_;
}

string&
value::
emplace_string() noexcept
{
    ::new(&str_) string(destroy());
    return str_;
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
// Observers
//
//----------------------------------------------------------

bool
value::
is_key_value_pair() const noexcept
{
    if(! is_array())
        return false;
    if(arr_.size() != 2)
        return false;
    if(! arr_[0].is_string())
        return false;
    return true;
}

bool
value::
maybe_object(
    std::initializer_list<value> init) noexcept
{
    for(auto const& v : init)
        if(! v.is_key_value_pair())
            return false;
    return true;
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
    if(sp->need_free())
        sp->deallocate(key_, len_ + 1, 1);
}

key_value_pair::
key_value_pair(
    key_value_pair const& other)
    : value_(other.value_)
    , len_(other.len_)
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
{
}

key_value_pair::
key_value_pair::
key_value_pair(
    key_value_pair const& other,
    storage_ptr sp)
    : value_(other.value_, detail::move(sp))
    , len_(other.len_)
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
{
}

//----------------------------------------------------------

} // json
} // boost

#endif
