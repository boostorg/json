//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_HPP
#define BOOST_JSON_DETAIL_VALUE_HPP

#include <boost/json/kind.hpp>
#include <boost/json/storage_ptr.hpp>
#include <new>
#include <utility>

BOOST_JSON_NS_BEGIN
namespace detail {

struct value_access
{
    template<class Value, class... Args>
    static
    Value&
    construct_value(Value* p, Args&&... args)
    {
        return *reinterpret_cast<
            Value*>(::new(p) Value(
            std::forward<Args>(args)...));
    }

    template<class KeyValuePair, class... Args>
    static
    KeyValuePair&
    construct_key_value_pair(
        KeyValuePair* p, Args&&... args)
    {
        return *reinterpret_cast<
            KeyValuePair*>(::new(p)
                KeyValuePair(
                    std::forward<Args>(args)...));
    }

    template<class Value>
    static
    char const*
    release_key(
        Value& jv,
        std::size_t& len) noexcept
    {
        BOOST_ASSERT(jv.is_string());
        return jv.str_.impl_.release_key(len);
    }

    using index_t = std::uint32_t;

    template<class KeyValuePair>
    static
    index_t&
    next(KeyValuePair& e) noexcept
    {
        return e.next_;
    }

    template<class KeyValuePair>
    static
    index_t const&
    next(KeyValuePair const& e) noexcept
    {
        return e.next_;
    }
};

} // detail
BOOST_JSON_NS_END

#endif
