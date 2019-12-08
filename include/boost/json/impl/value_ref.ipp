//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_REF_IPP
#define BOOST_JSON_IMPL_VALUE_REF_IPP

#include <boost/json/value_ref.hpp>
#include <boost/json/array.hpp>
#include <boost/json/value.hpp>

namespace boost {
namespace json {

value_ref::
operator
value() const
{
    return make_value({});
}

bool
value_ref::
is_key_value_pair() const noexcept
{
    if(what_ != what::ini)
        return false;
    if(ini_.size() != 2)
        return false;
    auto const& e = *ini_.begin();
    if( e.what_ != what::str &&
        e.what_ != what::svw)
        return false;
    return true;
}

bool
value_ref::
maybe_object(
    std::initializer_list<
        value_ref> init) noexcept
{
    for(auto const& e : init)
        if(! e.is_key_value_pair())
            return false;
    return true;
}

string_view
value_ref::
get_string() const noexcept
{
    if(what_ == what::svw)
        return { sv_.data, sv_.size };
    BOOST_ASSERT(what_ == what::str);
    return *str_;
}

value
value_ref::
make_value(
    storage_ptr sp) const
{
    switch(what_)
    {
    case what::val:
        return value(
            std::move(*val_),
            std::move(sp));

    case what::cval:
        return value(*cval_,
            std::move(sp));

    case what::obj:
        return object(
            std::move(*obj_),
            std::move(sp));

    case what::cobj:
        return object(*cobj_,
            std::move(sp));

    case what::arr:
        return array(
            std::move(*arr_),
            std::move(sp));

    case what::carr:
        return array(*carr_,
            std::move(sp));

    case what::str:
        return string(
            std::move(*str_),
            std::move(sp));

    case what::svw:
        return string(
            string_view(
                sv_.data, sv_.size),
            std::move(sp));

    case what::i64:
        return value(i64_,
            std::move(sp));

    case what::u64:
        return value(u64_,
            std::move(sp));

    case what::dub:
        return value(dub_,
            std::move(sp));

    case what::boo:
        return value(boo_,
            std::move(sp));

    case what::nul:
        return value(nullptr,
            std::move(sp));

    case what::ini:
        return make_value(ini_,
            std::move(sp));

    case what::fun:
        break;
    }
    value jv(std::move(sp));
    return jv;
}

value
value_ref::
make_value(
    std::initializer_list<
        value_ref> init,
    storage_ptr sp)
{
    if(maybe_object(init))
        return make_object(
            init, std::move(sp));
    return make_array(
        init, std::move(sp));
}

object
value_ref::
make_object(
    std::initializer_list<value_ref> init,
    storage_ptr sp)
{
    object obj(std::move(sp));
    obj.reserve(init.size());
    for(auto const& e : init)
        obj.emplace(
            e.ini_.begin()[0].get_string(),
            e.ini_.begin()[1].make_value(
                obj.storage()));
    return obj;
}

array
value_ref::
make_array(
    std::initializer_list<
        value_ref> init,
    storage_ptr sp)
{
    array arr(std::move(sp));
    arr.reserve(init.size());
    for(auto const& e : init)
        arr.emplace_back(
            e.make_value(
                arr.storage()));
    return arr;
}

void
value_ref::
write_array(
    value* dest,
    std::initializer_list<
        value_ref> init,
    storage_ptr const& sp)
{
    struct undo
    {
        value* const base;
        value* pos;
        ~undo()
        {
            if(pos)
                while(pos > base)
                    (--pos)->~value();
        }
    };
    undo u{dest, dest};
    for(auto const& e : init)
    {
        ::new(u.pos) value(
            e.make_value(sp));
        ++u.pos;
    }
    u.pos = nullptr;
}

} // json
} // boost

#endif
