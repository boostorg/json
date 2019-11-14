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
#include <boost/json/detail/except.hpp>
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

#if 0
// range
template<class T
    ,class = typename std::enable_if<
        detail::is_range<T>::value
        && ! std::is_same<T, typename object::value_type>::value
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
#endif

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

inline
void
swap(value& lhs, value& rhs)
{
    lhs.swap(rhs);
}

//----------------------------------------------------------

template<class... Args>
object_value_type::
object_value_type(
    string_view key,
    Args&&... args)
    : value_(std::forward<Args>(args)...)
    , len_(key.size())
    , key_(
        [&]
        {
            if(key.size() > BOOST_JSON_MAX_STRING_SIZE)
                BOOST_JSON_THROW(
                    detail::key_too_large_exception());
            auto s = reinterpret_cast<
                char*>(value_.get_storage()->
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
