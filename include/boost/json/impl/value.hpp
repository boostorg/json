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

#include <boost/json/except.hpp>
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

inline
std::uint32_t
key_value_pair::
key_size(std::size_t n)
{
    if(n > BOOST_JSON_MAX_STRING_SIZE)
        string_too_large::raise();
    return static_cast<
        std::uint32_t>(n);
}

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
                key_too_large::raise();
            auto s = reinterpret_cast<
                char*>(value_.storage()->
                    allocate(key.size() + 1));
            std::memcpy(s, key.data(), key.size());
            s[key.size()] = 0;
            return s;
        }())
    , len_(key_size(key.size()))
{
}

//----------------------------------------------------------

} // json
} // boost

#endif
