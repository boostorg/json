//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_VALUE_REF_HPP
#define BOOST_JSON_IMPL_VALUE_REF_HPP

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
        std::move(sp));
}

namespace detail {

// Native type implementation: use value constructor
template<class T>
value
from_const_impl(
    void const* p,
    storage_ptr sp,
    std::true_type /* is_native */)
{
    return value(
        *reinterpret_cast<
            T const*>(p),
        std::move(sp));
}

// UDT implementation: use value_from
// Note: value_from must be visible at the point of instantiation
template<class T>
value
from_const_impl(
    void const* p,
    storage_ptr sp,
    std::false_type /* is_native */)
{
    value jv(std::move(sp));
    value_from(
        *reinterpret_cast<
            T const*>(p),
        jv);
    return jv;
}

// Native type implementation: use value constructor
template<class T>
value
from_rvalue_impl(
    void* p,
    storage_ptr sp,
    std::true_type /* is_native */)
{
    return value(
        std::move(
            *reinterpret_cast<T*>(p)),
        std::move(sp));
}

// UDT implementation: use value_from
// Note: value_from must be visible at the point of instantiation
template<class T>
value
from_rvalue_impl(
    void* p,
    storage_ptr sp,
    std::false_type /* is_native */)
{
    value jv(std::move(sp));
    value_from(
        std::move(
            *reinterpret_cast<T*>(p)),
        jv);
    return jv;
}

} // namespace detail

template<class T>
value
value_ref::
from_const(
    void const* p,
    storage_ptr sp)
{
    return detail::from_const_impl<T>(
        p,
        std::move(sp),
        detail::is_native_value_type<T>());
}

template<class T>
value
value_ref::
from_rvalue(
    void* p,
    storage_ptr sp)
{
    return detail::from_rvalue_impl<T>(
        p,
        std::move(sp),
        detail::is_native_value_type<T>());
}

} // namespace json
} // namespace boost

#endif
