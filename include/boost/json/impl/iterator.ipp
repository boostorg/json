//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_ITERATOR_IMPL_IPP
#define BOOST_JSON_ITERATOR_IMPL_IPP

#include <boost/json/iterator.hpp>

namespace boost {
namespace json {

const_iterator::
node::
~node()
{
    if(it)
    {
        if(v->is_object())
        {
            obj_it.~const_iterator();
        }
        else
        {
            // VFALCO Why not?
            //arr.it.~const_iterator();
        }
    }
}

const_iterator::
node::
node(
    value const& v_,
    bool it_) noexcept
    : v(&v_)
    , it(it_)
{
    if(it)
    {
        BOOST_ASSERT(
            v->is_structured());
        if(v->is_object())
            ::new(&obj_it)
                object::const_iterator(
                    v->as_object().begin());
        else
            ::new(&arr_it)
                array::const_iterator(
                    v->as_array().begin());
    }
}

const_iterator::
node::
node(node const& other) noexcept
    : v(other.v)
    , key(other.key)
{
    it = other.it;
    if(it)
        ::new(&obj_it)
            object::const_iterator(
                other.obj_it);
    else
        ::new(&arr_it)
            array::const_iterator(
                other.arr_it);
}

bool
const_iterator::
node::
last() const noexcept
{
    if(! it)
        return true;
    if(v->is_object())
        return obj_it ==
            v->as_object().end();
    return arr_it ==
        v->as_array().end();
}

//------------------------------------------------------------------------------

const_iterator::
const_iterator(
    value const& jv)
{
    stack_.emplace_front(
        jv, false);
}

auto
const_iterator::
operator*() const noexcept ->
    value_type
{
    auto const& n = stack_.front();
    if(! n.it)
    {
        BOOST_ASSERT(
            stack_.size() == 1);
        return {
            stack_.size() - 1,
            n.key,
            *n.v,
            true,
            false };
    }
    BOOST_ASSERT(n.v->is_structured());
    if(n.v->is_object())
    {
        if(n.obj_it !=
            n.v->as_object().end())
            return {
                stack_.size(),
                n.obj_it->first,
                n.obj_it->second,
                std::next(n.obj_it) ==
                    n.v->as_object().end(),
                false };
        return {
            stack_.size() - 1,
            "",
            *n.v,
            stack_.size() == 1
                || stack_[1].last(),
            true};
    }
    if(n.arr_it !=
        n.v->as_array().end())
        return {
            stack_.size(),
            "",
            *n.arr_it,
            std::next(n.arr_it) ==
                n.v->as_array().end(),
            false };
    return {
        stack_.size() - 1,
        "",
        *n.v,
        stack_.size() == 1
            || stack_[1].last(),
        true};
}

const_iterator&
const_iterator::
operator++() noexcept
{
    auto& n = stack_.front();
    if(! n.it)
    {
        if(n.v->is_structured())
        {
            stack_.pop_front();
            stack_.emplace_front(
                *n.v, true);
        }
        else
        {
            stack_.pop_front();
        }
    }
    else if(n.v->is_object())
    {
        if(n.obj_it ==
            n.v->as_object().end())
        {
            stack_.pop_front();
        }
        else
        {
            auto const& jv =
                n.obj_it->second;
            if(jv.is_structured())
                stack_.emplace_front(
                    jv, true);
            ++n.obj_it;
        }
    }
    else
    {
        if(n.arr_it ==
            n.v->as_array().end())
        {
            stack_.pop_front();
        }
        else
        {
            auto const& jv = *n.arr_it;
            if(jv.is_structured())
                stack_.emplace_front(
                    jv, true);
            ++n.arr_it;
        }
    }
    return *this;
}

} // json
} // boost

#endif
