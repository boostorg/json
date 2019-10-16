//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_IPP
#define BOOST_JSON_IMPL_SERIALIZER_IPP

#include <boost/json/serializer.hpp>

namespace boost {
namespace json {

serializer::
impl::
impl(value const& jv)
    : jv_(jv)
    , it_(jv)
{
}

bool
serializer::
impl::
is_done() const noexcept
{
    return state_ == state::done;
}

std::size_t
serializer::
impl::
next(char* dest, std::size_t size)
{
    auto const p0 = dest;
    auto p = p0;
    auto n = size;
    while(n > 0)
    {
        switch(state_)
        {
        case state::next:
            ++it_;
            if(it_ == detail::end)
            {
                state_ = state::done;
                break;
            }
            BOOST_FALLTHROUGH;

        case state::init:
            str_ = it_->key;
            if(! str_.empty())
            {
                *p++ = '\"';
                --n;
                state_ = state::key;
            }
            else
            {
                state_ = state::value;
            }
            break;

        case state::key:
        case state::string:
            // Write escaped/UTF-8 string
            if(n >= str_.size())
            {
                std::memcpy(p,
                    str_.data(), str_.size());
                p += str_.size();
                n -= str_.size();
                if(state_ == state::string)
                {
                    if(last_)
                        str_ = {"\"", 1};
                    else
                        str_ = {"\",", 2};
                    state_ = state::literal;
                }
                else
                {
                    str_ = {"\":", 2};
                    state_ = state::key_literal;
                }
                break;
            }
            else
            {
                std::memcpy(p,
                    str_.data(), n);
                p += n;
                str_ = {
                    str_.data() + n,
                    str_.size() - n};
                n = 0;
            }
            break;

        case state::literal:
        case state::key_literal:
            // Copy content of str_ without escapes
            if(n >= str_.size())
            {
                std::memcpy(p,
                    str_.data(), str_.size());
                p += str_.size();
                n -= str_.size();
                if(state_ == state::literal)
                    state_ = state::next;
                else
                    state_ = state::value;
            }
            else
            {
                std::memcpy(p,
                    str_.data(), n);
                p += n;
                str_ = {
                    str_.data() + n,
                    str_.size() - n};
                n = 0;
            }
            break;

        case state::value:
        {
            auto const& e = *it_;
            if(! e.end)
            {
                switch(e.value.kind())
                {
                case kind::object:
                    *p++ = '{';
                    --n;
                    state_ = state::next;
                    break;
                case kind::array:
                    *p++ = '[';
                    --n;
                    state_ = state::next;
                    break;
                case kind::string:
                    *p++ = '\"';
                    str_ = e.value.as_string();
                    last_ = e.last;
                    state_ = state::string;
                    break;
                case kind::number:
                    str_ = e.value.as_number().print(
                        buf_, sizeof(buf_));
                    if(! e.last)
                    {
                        buf_[str_.size()] = ',';
                        str_ = {buf_, str_.size() + 1};
                    }
                    state_ = state::literal;
                    break;
                case kind::boolean:
                    if(e.value.as_bool())
                    {
                        if(! e.last)
                            str_ = {"true,", 5};
                        else
                            str_ = {"true", 4};
                    }
                    else
                    {
                        if(! e.last)
                            str_ = {"false,", 6};
                        else
                            str_ = {"false", 5};
                    }
                    state_ = state::literal;
                    break;
                case kind::null:
                    if(! e.last)
                        str_ = {"null,", 5};
                    else
                        str_ = {"null", 4};
                    state_ = state::literal;
                    break;
                }
            }
            else if(e->value.is_object())
            {
                if(! e.last)
                    str_ = {"},", 2};
                else
                    str_ = {"}", 1};
                state_ = state::literal;
            }
            else if(e->value.is_array())
            {
                if(! e.last)
                    str_ = {"],", 2};
                else
                    str_ = {"]", 1};
                state_ = state::literal;
            }
            break;
        }

        case state::done:
            goto finish;
        }
    }
finish:
    return p - p0;
}

//------------------------------------------------------------------------------

serializer::
~serializer()
{
    get_base().~base();
}

serializer::
serializer(value const& jv)
{
    ::new(&get_base()) impl(jv);
}

} // json
} // boost

#endif
