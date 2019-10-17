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

    std::initializer_list<value>::iterator it_;

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
            it_->as_array()[0].as_string(),
            it_->as_array()[1] };
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

    case json::kind::number:
        nat_.num_.~number();
        BOOST_FALLTHROUGH;

    case json::kind::boolean:
    case json::kind::null:
        nat_.sp_.~storage_ptr();
        break;
    }
}

value::
value() noexcept
    : value(
        json::kind::null,
        default_storage())
{
}

value::
value(storage_ptr sp) noexcept
    : value(
        json::kind::null,
        std::move(sp))
{
}

value::
value(
    json::kind k,
    storage_ptr sp) noexcept
{
    switch(k)
    {
    case json::kind::object:
        ::new(&obj_) object(
            std::move(sp));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            std::move(sp));
        break;

    case json::kind::string:
        ::new(&str_) string(
            std::move(sp));
        break;

    case json::kind::number:
        ::new(&nat_.num_) number;
        ::new(&nat_.sp_)
            storage_ptr(std::move(sp));
        break;

    case json::kind::boolean:
        ::new(&nat_.bool_) bool{};
        ::new(&nat_.sp_)
            storage_ptr(std::move(sp));
        break;

    case json::kind::null:
        ::new(&nat_.sp_)
            storage_ptr(std::move(sp));
        break;
    }
    kind_ = k;
}

value::
value(value const& other)
    : value(
        other,
        other.get_storage())
{
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

    case json::kind::number:
        ::new(&nat_.num_) number(
            other.nat_.num_);
        ::new(&nat_.sp_) storage_ptr(
            std::move(sp));
        break;

    case json::kind::boolean:
        nat_.bool_ = other.nat_.bool_;
        ::new(&nat_.sp_) storage_ptr(
            std::move(sp));
        break;

    case json::kind::null:
        ::new(&nat_.sp_) storage_ptr(
            std::move(sp));
        break;
    }
    kind_ = other.kind_;
}

value::
value(pilfered<value> p) noexcept
{
    auto& other = p.get();
    switch(other.kind_)
    {
    case json::kind::object:
        relocate(&obj_, other.obj_);
        ::new(&other.nat_.sp_) storage_ptr;
        break;

    case json::kind::array:
        relocate(&arr_, other.arr_);
        ::new(&other.nat_.sp_) storage_ptr;
        break;

    case json::kind::string:
        relocate(&str_, other.str_);
        ::new(&other.nat_.sp_) storage_ptr;
        break;

    case json::kind::number:
        relocate(&nat_.num_, other.nat_.num_);
        ::new(&nat_.sp_) storage_ptr(
            std::move(other.nat_.sp_));
        break;

    case json::kind::boolean:
        nat_.bool_ = other.nat_.bool_;
        ::new(&nat_.sp_) storage_ptr(
            std::move(other.nat_.sp_));
        break;

    case json::kind::null:
        ::new(&nat_.sp_) storage_ptr(
            std::move(other.nat_.sp_));
        break;
    }
    kind_ = other.kind_;
    other.kind_ = json::kind::null;
}

value::
value(value&& other) noexcept
{
    switch(other.kind_)
    {
    case json::kind::object:
        ::new(&obj_) object(
            std::move(other.obj_));
        break;

    case json::kind::array:
        ::new(&arr_) array(
            std::move(other.arr_));
        break;

    case json::kind::string:
        ::new(&str_) string(
            std::move(other.str_));
        break;

    case json::kind::number:
        ::new(&nat_.num_) number(
            std::move(other.nat_.num_));
        ::new(&nat_.sp_) storage_ptr(
            other.nat_.sp_);
        break;

    case json::kind::boolean:
        nat_.bool_ = other.nat_.bool_;
        ::new(&nat_.sp_) storage_ptr(
            other.nat_.sp_);
        break;

    case json::kind::null:
        ::new(&nat_.sp_) storage_ptr(
            other.nat_.sp_);
        break;
    }
    kind_ = other.kind_;
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

    case json::kind::number:
        relocate(
            &nat_.num_, other.nat_.num_);
        ::new(&nat_.sp_) storage_ptr(
            std::move(sp));
        break;

    case json::kind::boolean:
        nat_.bool_ = other.nat_.bool_;
        ::new(&nat_.sp_) storage_ptr(
            std::move(sp));
        break;

    case json::kind::null:
        ::new(&nat_.sp_) storage_ptr(
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
value(object obj) noexcept
    : obj_(std::move(obj))
    , kind_(json::kind::object)
{
}

value::
value(
    object obj,
    storage_ptr sp)
    : obj_(
        std::move(obj),
        std::move(sp))
    , kind_(json::kind::object)
{
}

value::
value(array arr) noexcept
    : arr_(std::move(arr))
    , kind_(json::kind::array)
{
}

value::
value(
    array arr,
    storage_ptr sp)
    : arr_(
        std::move(arr),
        std::move(sp))
    , kind_(json::kind::array)
{
}

value::
value(string str) noexcept
    : str_(std::move(str))
    , kind_(json::kind::string)
{
}

value::
value(
    string str,
    storage_ptr sp)
    : str_(
        std::move(str),
        std::move(sp))
    , kind_(json::kind::string)
{
}

value::
value(number num) noexcept
    : kind_(json::kind::number)
{
    ::new(&nat_.num_) number(num);
    ::new(&nat_.sp_) storage_ptr(
        default_storage());
}

value::
value(
    number num,
    storage_ptr sp)
    : kind_(json::kind::number)
{
    ::new(&nat_.num_) number(num);
    ::new(&nat_.sp_) storage_ptr(
        std::move(sp));
}

value::
value(std::initializer_list<value> init)
    : value(
        init,
        default_storage())
{
}

value::
value(
    std::initializer_list<value> init,
    storage_ptr sp)
{
    if(maybe_object(init))
    {
        ::new(&obj_) object(
            init_iter{init.begin()},
            init_iter{init.end()},
            init.size(),
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

value&
value::
operator=(object obj)
{
    undo u(this);
    ::new(this) value(
        std::move(obj),
        u.old.get_storage());
    u.commit();
    return *this;
}

value&
value::
operator=(array arr)
{
    undo u(this);
    ::new(this) value(
        std::move(arr),
        u.old.get_storage());
    u.commit();
    return *this;
}

value&
value::
operator=(string str)
{
    undo u(this);
    ::new(this) value(
        std::move(str),
        u.old.get_storage());
    u.commit();
    return *this;
}

value&
value::
operator=(number num)
{
    undo u(this);
    ::new(this) value(
        std::move(num),
        u.old.get_storage());
    u.commit();
    return *this;
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

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

    default:
        break;
    }
    return nat_.sp_;
}

//----------------------------------------------------------

// friends

std::ostream&
operator<<(std::ostream& os, value const& jv)
{
    switch(jv.kind())
    {
    case json::kind::object:
        os << '{';
        for(auto it = jv.as_object().begin(),
            last = jv.as_object().end();
            it != last;)
        {
            os << '\"' << it->first << "\":";
            os << it->second;
            if(++it != last)
                os << ',';
        }
        os << '}';
        break;
        
    case json::kind::array:
        os << '[';
        for(auto it = jv.as_array().begin(),
            last = jv.as_array().end();
            it != last;)
        {
            os << *it;
            if(++it != last)
                os << ',';
        }
        os << ']';
        break;
        
    case json::kind::string:
        os << '\"' << jv.as_string() << '\"';
        break;
        
    case json::kind::number:
        os << jv.as_number();
        break;
        
    case json::kind::boolean:
        if(jv.as_bool())
            os << "true";
        else
            os << "false";
        break;

    case json::kind::null:
        os << "null";
        break;
    }
    return os;
}

} // json
} // boost

#endif
