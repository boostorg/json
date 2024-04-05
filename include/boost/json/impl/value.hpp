//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_VALUE_HPP
#define BOOST_JSON_IMPL_VALUE_HPP

namespace boost {
namespace json {

value const&
value::at_pointer(string_view ptr) const&
{
    return try_at_pointer(ptr).value();
}

value&
value::at_pointer(string_view ptr) &
{
    return try_at_pointer(ptr).value();
}

value&&
value::at_pointer(string_view ptr) &&
{
    return std::move( try_at_pointer(ptr).value() );
}

} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_VALUE_HPP
