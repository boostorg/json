//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_VISIT_HPP
#define BOOST_JSON_IMPL_VISIT_HPP

#include <boost/type_traits/copy_cv_ref.hpp>

namespace boost {
namespace json {
namespace detail {

template<class Visitor, class Value>
auto
visit(
    Visitor&& v,
    Value&& jv) -> decltype(
        static_cast<Visitor&&>(v)(
            std::declval< copy_cv_ref_t<std::nullptr_t, Value&&> >() ) )
{
    auto&& vis_ref = static_cast<Visitor&&>(v);
    auto&& arg_ref = static_cast<Value&&>(jv);
    switch(jv.kind())
    {
    default: // unreachable()?
    case kind::bool_:   return vis_ref( arg_ref.get_bool() );
    case kind::int64:   return vis_ref( arg_ref.get_int64() );
    case kind::uint64:  return vis_ref( arg_ref.get_uint64() );
    case kind::double_: return vis_ref( arg_ref.get_double() );
    case kind::string:  return vis_ref( arg_ref.get_string() );
    case kind::array:   return vis_ref( arg_ref.get_array() );
    case kind::object:  return vis_ref( arg_ref.get_object() );
    case kind::null: {
        std::nullptr_t np;
        using Ref = copy_cv_ref_t<std::nullptr_t, Value&&>;
        return vis_ref( static_cast<Ref>(np) );
    }
    }
}

} // namespace detail

template<class Visitor>
auto
visit(
    Visitor&& v,
    value& jv) -> decltype(
        static_cast<Visitor&&>(v)( std::declval<std::nullptr_t&>() ) )
{
    return detail::visit( static_cast<Visitor&&>(v), jv );
}

template<class Visitor>
auto
visit(
    Visitor&& v,
    value const& jv) -> decltype(
        static_cast<Visitor&&>(v)( std::declval<std::nullptr_t const&>() ) )
{
    return detail::visit( static_cast<Visitor&&>(v), jv );
}

} // namespace json
} // namespace boost

#endif
