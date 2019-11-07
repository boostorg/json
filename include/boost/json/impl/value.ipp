//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
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
        std::input_iterator_tag;

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

    BOOST_JSON_STATIC_ASSERT(
        std::is_constructible<
            object::value_type,
            init_iter::value_type>::value);
};

//----------------------------------------------------------

value::
~value()
{
    switch(kind_)
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
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::boolean:
    case json::kind::null:
        sca_.sp.~storage_ptr();
        break;
    }
}

value::
value(pilfered<value> p) noexcept
{
    std::memcpy(this, &p.get(), sizeof(*this));
    ::new(&p.get().sca_.sp) storage_ptr{};
    p.get().kind_ = json::kind::null;
}

value::
value(
    value const& other,
    storage_ptr sp)
{
    switch(other.kind_)
    {
    case json::kind::object:
        ::new(&obj_) object(
            other.obj_, std::move(sp));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            other.arr_, std::move(sp));
        break;

    case json::kind::string:
        ::new(&str_) string(
            other.str_, std::move(sp));
        break;

    case json::kind::int64:
        ::new(&sca_.i) std::int64_t(
            other.sca_.i);
        ::new(&sca_.sp)
            storage_ptr(std::move(sp));
        break;

    case json::kind::uint64:
        ::new(&sca_.u) std::uint64_t(
            other.sca_.u);
        ::new(&sca_.sp)
            storage_ptr(std::move(sp));
        break;

    case json::kind::double_:
        ::new(&sca_.d) double(
            other.sca_.d);
        ::new(&sca_.sp)
            storage_ptr(std::move(sp));
        break;

    case json::kind::boolean:
        ::new(&sca_.b) bool(
            other.sca_.b);
        ::new(&sca_.sp)
            storage_ptr(std::move(sp));
        break;

    case json::kind::null:
        ::new(&sca_.sp)
            storage_ptr(std::move(sp));
        break;
    }
    kind_ = other.kind_;
}

value::
value(value&& other) noexcept
{
    std::memcpy(this, &other, sizeof(*this));
    ::new(&other.sca_.sp) storage_ptr{};
    other.kind_ = json::kind::null;
}

value::
value(
    value&& other,
    storage_ptr sp)
{
    switch(other.kind_)
    {
    case json::kind::object:
        ::new(&obj_) object(
            std::move(other.obj_),
            std::move(sp));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            std::move(other.arr_),
            std::move(sp));
        break;

    case json::kind::string:
        ::new(&str_) string(
            std::move(other.str_),
            std::move(sp));
        break;

    case json::kind::int64:
        ::new(&sca_.i) std::int64_t(
            other.sca_.i);
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
        break;

    case json::kind::uint64:
        ::new(&sca_.u) std::uint64_t(
            other.sca_.u);
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
        break;

    case json::kind::double_:
        ::new(&sca_.d) double(
            other.sca_.d);
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
        break;

    case json::kind::boolean:
        ::new(&sca_.b) bool(
            other.sca_.b);
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
        break;

    case json::kind::null:
        ::new(&sca_.sp) storage_ptr(
            std::move(sp));
        break;
    }
    kind_ = other.kind_;
}

value&
value::
operator=(value&& other)
{
    undo u(this);
    ::new(this) value(
        std::move(other),
        u.old.get_storage());
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
        u.old.get_storage());
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
                std::length_error(
                    "count > max_size()"));
        ::new(&obj_) object(
            init_iter{init.begin()},
            init_iter{init.end()},
            static_cast<
                object::size_type>(init.size()),
            std::move(sp));
        kind_ = json::kind::object;
    }
    else
    {
        ::new(&arr_) array(
            init, std::move(sp));
        kind_ = json::kind::array;
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
    kind_ = json::kind::object;
    return obj_;
}

array&
value::
emplace_array() noexcept
{
    ::new(&arr_) array(destroy());
    kind_ = json::kind::array;
    return arr_;
}

string&
value::
emplace_string() noexcept
{
    ::new(&str_) string(destroy());
    kind_ = json::kind::string;
    return str_;
}

std::int64_t&
value::
emplace_int64() noexcept
{
    ::new(&sca_.sp) storage_ptr(destroy());
    kind_ = json::kind::int64;
    sca_.i = 0;
    return sca_.i;
}

std::uint64_t&
value::
emplace_uint64() noexcept
{
    ::new(&sca_.sp) storage_ptr(destroy());
    kind_ = json::kind::uint64;
    sca_.u = 0;
    return sca_.u;
}

double&
value::
emplace_double() noexcept
{
    ::new(&sca_.sp) storage_ptr(destroy());
    kind_ = json::kind::double_;
    sca_.d = 0;
    return sca_.d;
}

bool&
value::
emplace_bool() noexcept
{
    ::new(&sca_.sp) storage_ptr(destroy());
    kind_ = json::kind::boolean;
    sca_.b = false;
    return sca_.b;
}

void
value::
emplace_null() noexcept
{
    ::new(&sca_.sp) storage_ptr(destroy());
    kind_ = json::kind::null;
}

void
value::
swap(value& other)
{
    if(*get_storage() != *other.get_storage())
    {
        // copy
        value temp1(
            std::move(*this),
            other.get_storage());
        value temp2(
            std::move(other),
            this->get_storage());
        other.~value();
        ::new(&other) value(pilfer(temp1));
        this->~value();
        ::new(this) value(pilfer(temp2));
    }
    else
    {
        value tmp(pilfer(*this));
        this->~value();
        ::new(this) value(pilfer(other));
        other.~value();
        ::new(&other) value(pilfer(tmp));
    }
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
    if(kind_ != json::kind::array)
        return false;
    if(arr_.size() != 2)
        return false;
    if(arr_[0].kind_ != json::kind::string)
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

storage_ptr const&
value::
get_storage() const noexcept
{
    switch(kind_)
    {
    case json::kind::object:
        return obj_.get_storage();

    case json::kind::array:
        return arr_.get_storage();

    case json::kind::string:
        return str_.get_storage();

    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::boolean:
    case json::kind::null:
    default:
        break;
    }
    return sca_.sp;
}

//----------------------------------------------------------

storage_ptr
value::
destroy() noexcept
{
    switch(kind_)
    {
    case json::kind::object:
    {
        auto sp = obj_.get_storage();
        obj_.~object();
        return sp;
    }

    case json::kind::array:
    {
        auto sp = arr_.get_storage();
        arr_.~array();
        return sp;
    }

    case json::kind::string:
    {
        auto sp = str_.get_storage();
        str_.~string();
        return sp;
    }

    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::boolean:
    case json::kind::null:
    default:
        break;
    }
    return std::move(sca_.sp);
}

//----------------------------------------------------------

object::
value_type::
~value_type()
{
    auto const& sp = value_.get_storage();
    if(sp->need_free())
        sp->deallocate(key_, len_ + 1, 1);
}

object::
value_type::
value_type(value_type const& other)
    : value_(other.value_)
    , len_(other.len_)
    , key_(
        [&]
        {
            auto s = reinterpret_cast<
                char*>(value_.get_storage()->
                    allocate(other.len_ + 1));
            std::memcpy(s, other.key_, other.len_);
            s[other.len_] = 0;
            return s;
        }())
{
}

object::
value_type::
value_type(
    value_type const& other,
    storage_ptr sp)
    : value_(other.value_, std::move(sp))
    , len_(other.len_)
    , key_(
        [&]
        {
            auto s = reinterpret_cast<
                char*>(value_.get_storage()->
                    allocate(other.len_ + 1));
            std::memcpy(s, other.key_, other.len_);
            s[other.len_] = 0;
            return s;
        }())
{
}

void
object::
value_type::
destroy(
    value_type* p,
    size_type n) noexcept
{
    if(n == 0)
        return;
    if(! p->value().get_storage()->need_free())
        return;
    while(n--)
        (*p++).~value_type();
}

//----------------------------------------------------------

} // json
} // boost

#endif
