//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_ITERATOR_IMPL_IPP
#define BOOST_JSON_DETAIL_ITERATOR_IMPL_IPP

#include <boost/json/detail/iterator.hpp>

namespace boost {
namespace json {
namespace detail {

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
            // VFALCO Array iterators are just a pointer
            //arr_it.~const_iterator();
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
        BOOST_JSON_ASSERT(
            v->is_structured());
        if(v->is_object())
            ::new(&obj_it)
                object::const_iterator(
                    v->if_object()->begin());
        else
            ::new(&arr_it)
                array::const_iterator(
                    v->if_array()->begin());
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
            v->if_object()->end();
    return arr_it ==
        v->if_array()->end();
}

//----------------------------------------------------------

const_iterator::
const_iterator(
    value const& jv) noexcept
{
    stack_.emplace_front(
        jv, false);
    setup();
}

bool
const_iterator::
at_end() const noexcept
{
    return stack_.empty();
}

const_iterator&
const_iterator::
operator++() noexcept
{
    {
        auto& n = stack_.front();
        if(! n.it)
        {
            if(n.v->is_structured())
            {
                stack_.pop();
                stack_.emplace_front(
                    *n.v, true);
            }
            else
            {
                stack_.pop();
            }
        }
        else if(n.v->is_object())
        {
            if(n.obj_it ==
                n.v->if_object()->end())
            {
                stack_.pop();
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
                n.v->if_array()->end())
            {
                stack_.pop();
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
    }
    if(! stack_.empty())
        setup();
    return *this;
}

void
const_iterator::
setup() noexcept
{
    auto const& n = stack_.front();
    if(! n.it)
    {
        BOOST_JSON_ASSERT(
            stack_.size() == 1);
        BOOST_JSON_ASSERT(n.key.empty());
        v_.depth =  stack_.size() - 1,
        v_.key = "";
        v_.value = n.v;
        v_.has_key = false;
        v_.last = true;
        v_.end = false;
        return;
    }
    BOOST_JSON_ASSERT(n.v->is_structured());
    if(n.v->is_object())
    {
        if(n.obj_it !=
            n.v->if_object()->end())
        {
            v_.depth = stack_.size();
            v_.key = n.obj_it->first;
            v_.value = &n.obj_it->second;
            v_.has_key = true;
            v_.last = std::next(n.obj_it) ==
                n.v->if_object()->end();
            v_.end = false;
            return;
        }
        v_.depth = stack_.size() - 1;
        v_.key = "";
        v_.value = n.v;
        v_.has_key = true;
        v_.last = stack_.size() == 1
            || stack_[1].last();
        v_.end = true;
        return;
    }
    if(n.arr_it !=
        n.v->if_array()->end())
    {
        v_.depth = stack_.size();
        v_.key = "";
        v_.value = n.arr_it;
        v_.has_key = false;
        v_.last = std::next(n.arr_it) ==
            n.v->if_array()->end();
        v_.end = false;
        return;
    }
    v_.depth = stack_.size() - 1;
    v_.key = "";
    v_.value = n.v;
    v_.has_key = false;
    v_.last = stack_.size() == 1
        || stack_[1].last();
    v_.end = true;
}

} // detail
} // json
} // boost

#endif
