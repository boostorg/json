//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_HPP
#define BOOST_JSON_IMPL_VALUE_HPP

#include <boost/json/error.hpp>
#include <boost/json/detail/except.hpp>
#include <cstring>
#include <limits>
#include <type_traits>

namespace boost {
namespace json {

//----------------------------------------------------------

struct value::undo
{
    union
    {
        value saved;
    };
    value* self;

    explicit
    undo(value* self_) noexcept
        : self(self_)
    {
        relocate(&saved, *self_);
    }

    void
    commit() noexcept
    {
        saved.~value();
        self = nullptr;
    }

    ~undo()
    {
        if(self)
            relocate(self, saved);
    }
};

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

template<class T, class>
value&
value::
operator=(T&& t)
{
    undo u(this);
    ::new(this) value(
        std::forward<T>(t),
        u.saved.storage());
    u.commit();
    return *this;
}

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

template<class... Args>
key_value_pair::
key_value_pair(
    string_view key,
    Args&&... args)
    : value_(std::forward<Args>(args)...)
    , len_(key.size())
    , key_(
        [&]
        {
            if(key.size() > string::max_size())
                BOOST_JSON_THROW(
                    detail::key_too_large_exception());
            auto s = reinterpret_cast<
                char*>(value_.storage()->
                    allocate(key.size() + 1));
            std::memcpy(s, key.data(), key.size());
            s[key.size()] = 0;
            return s;
        }())
{
}

//----------------------------------------------------------

} // json
} // boost

#endif
