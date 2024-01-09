//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_VIEW_HPP
#define BOOST_JSON_DETAIL_VALUE_VIEW_HPP

#include <boost/json/kind.hpp>
#include <boost/json/storage_ptr.hpp>
#include <cstdint>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

class value_view;

namespace detail {

struct scalar_view
{
    kind k;         // must come first
    union
    {
        bool b;
        std::int64_t i;
        std::uint64_t u;
        double d;
    };

    explicit
    scalar_view() noexcept :
        k(json::kind::null)
    {
    }

    explicit
    scalar_view(bool b_) noexcept
        : k(json::kind::bool_)
        , b(b_)
    {
    }

    explicit
    scalar_view(std::int64_t i_) noexcept
        : k(json::kind::int64)
        , i(i_)
    {
    }

    explicit
    scalar_view(std::uint64_t u_) noexcept
        : k(json::kind::uint64)
        , u(u_)
    {
    }

    explicit
    scalar_view(double d_) noexcept
        : k(json::kind::double_)
        , d(d_)
    {
    }
};

struct string_view_with_kind
{
  kind k;         // must come first
  string_view view;

  template<typename ... Args>
  string_view_with_kind(Args && ... args)
        : k(json::kind::string)
        , view(std::forward<Args>(args)...) {}
};

BOOST_JSON_DECL
std::size_t
hash_value_impl( value_view const& jv ) noexcept;

} // detail
} // namespace json
} // namespace boost

#endif // BOOST_JSON_DETAIL_VALUE_VIEW_HPP
