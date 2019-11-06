//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_HPP
#define BOOST_JSON_IMPL_VALUE_HPP

#include <boost/json/error.hpp>
#include <boost/json/detail/make_void.hpp>
#include <limits>
#include <type_traits>

namespace boost {
namespace json {

//----------------------------------------------------------

struct value::undo
{
    union
    {
        value old;
    };
    value* cur;

    explicit
    undo(value* cur_) noexcept
        : cur(cur_)
    {
        relocate(&old, *cur);
    }

    void
    commit() noexcept
    {
        old.~value();
        cur = nullptr;
    }

    ~undo()
    {
        if(cur)
            relocate(cur, old);
    }
};

//----------------------------------------------------------

namespace detail {

template<class T, class = void>
struct is_range : std::false_type
{
};

template<class T>
struct is_range<T, void_t<
    typename T::value_type,
    decltype(
        std::declval<T const&>().begin(),
        std::declval<T const&>().end()
    )>> : std::true_type
{
};

} // detail

//----------------------------------------------------------
//
// assign to value
//

// range
template<class T
    ,class = typename std::enable_if<
        detail::is_range<T>::value
        && ! std::is_same<typename T::value_type, char>::value
        && has_to_json<typename T::value_type>::value
            >::type
>
void
to_json(T const& t, value& v)
{
    array arr(v.get_storage());
    for(auto const& e : t)
        arr.emplace_back(e);
    v = std::move(arr);
}

//----------------------------------------------------------
//
// assign value to
//

// integer

template<typename T
    ,class = typename std::enable_if<
        std::is_integral<T>::value>::type
>
void
from_json(T& t, value const& v)
{
    if(v.is_int64())
    {
        auto const rhs = v.as_int64();
        if( rhs > (std::numeric_limits<T>::max)() ||
            rhs < (std::numeric_limits<T>::min)())
            BOOST_JSON_THROW(system_error(
                error::integer_overflow));
        t = static_cast<T>(rhs);
    }
    else if(v.is_uint64())
    {
        auto const rhs = v.as_uint64();
        if(rhs > (std::numeric_limits<T>::max)())
            BOOST_JSON_THROW(system_error(
                error::integer_overflow));
        t = static_cast<T>(rhs);
    }
    else
    {
        BOOST_JSON_THROW(
            system_error(error::not_number));
    }
}

//----------------------------------------------------------

template<class Bool, class>
value::
value(Bool b, storage_ptr sp) noexcept
    : kind_(json::kind::boolean)
{
    ::new(&sca_.b) bool(b);
    ::new(&sca_.sp) storage_ptr(
        std::move(sp));
}

//----------------------------------------------------------

inline
void
swap(value& lhs, value& rhs)
{
    lhs.swap(rhs);
}

//----------------------------------------------------------

template<class... Args>
key_value_pair::
key_value_pair(
    string_view key_,
    Args&&... args)
    : value(std::forward<Args>(args)...)
    , key(
        [&]
        {
            auto s = reinterpret_cast<
                char*>(value.get_storage()->
                    allocate(key_.size() + 1));
            std::memcpy(s, key_.data(), key_.size());
            s[key_.size()] = 0;
            return string_view(s, key_.size() + 1);
        }())
{
}

//----------------------------------------------------------

} // json
} // boost

#endif
