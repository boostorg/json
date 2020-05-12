//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_REF_HPP
#define BOOST_JSON_IMPL_VALUE_REF_HPP

#include <boost/json/value_from.hpp>

namespace boost {
namespace json {

template<class T>
value
value_ref::
from_builtin(
    void const* p,
    storage_ptr sp) noexcept
{
    return value(
        *reinterpret_cast<
            T const*>(p),
        detail::move(sp));
}

template<class T>
value
value_ref::
from_const(
    void const* p,
    storage_ptr sp)
{
    return value_from(
        *reinterpret_cast<
            T const*>(p),
        detail::move(sp));
}

template<class T>
value
value_ref::
from_rvalue(
    void* p,
    storage_ptr sp)
{
    return value_from(
        detail::move(
            *reinterpret_cast<T*>(p)),
        detail::move(sp));
}

} // json
} // boost

#endif
