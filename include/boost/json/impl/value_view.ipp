//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_VALUE_VIEW_IPP
#define BOOST_JSON_IMPL_VALUE_VIEW_IPP

#include <boost/container_hash/hash.hpp>
#include <boost/json/value_view.hpp>
#include <boost/json/parser.hpp>
#include <cstring>
#include <istream>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

key_value_view_pair::key_value_view_pair(key_value_pair const& other)
    : key_value_view_pair(other.key(), other.value())
{
}

namespace
{

struct value_view_hasher
{
    std::size_t& seed;

    template< class T >
    void operator()( T&& t ) const noexcept
    {
        boost::hash_combine( seed, t );
    }
};

} // namespace

/*
value_view::value_view(const json::value & val)
{

    switch(val.kind())
    {
        case json::kind::null:    new (&sca_) scalar_view(); break;
        case json::kind::bool_:   new (&sca_) scalar_view(val.get_bool()); break;
        case json::kind::int64:   new (&sca_) scalar_view(val.get_int64()); break;
        case json::kind::uint64:  new (&sca_) scalar_view(val.get_uint64()); break;
        case json::kind::double_: new (&sca_) scalar_view(val.get_double()); break;
        case json::kind::string:
            new (&str_) detail::string_view_with_kind(val.get_string());
            break;
        case json::kind::array:
            new (&arr_) array_view(val.get_array());
            break;
        case json::kind::object:
            new (&arr_) object_view(val.get_object());
            break;
    }
}*/


value_view::
~value_view() noexcept
{
    switch(kind())
    {
    case json::kind::null:
    case json::kind::bool_:
    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
        sca_.~scalar_view();
        break;

    case json::kind::string:
        str_.~string_view_with_kind();
        break;

    case json::kind::array:
        arr_.~array_view();
        break;

    case json::kind::object:
        obj_.~object_view();
        break;
    }
}



//----------------------------------------------------------
//
// private
//
//----------------------------------------------------------


bool
value_view::
equal(value_view const& other) const noexcept
{
    switch(kind())
    {
    default: // unreachable()?
    case json::kind::null:
        return other.kind() == json::kind::null;

    case json::kind::bool_:
        return
            other.kind() == json::kind::bool_ &&
            get_bool() == other.get_bool();

    case json::kind::int64:
        switch(other.kind())
        {
        case json::kind::int64:
            return get_int64() == other.get_int64();
        case json::kind::uint64:
            if(get_int64() < 0)
                return false;
            return static_cast<std::uint64_t>(
                get_int64()) == other.get_uint64();
        default:
            return false;
        }

    case json::kind::uint64:
        switch(other.kind())
        {
        case json::kind::uint64:
            return get_uint64() == other.get_uint64();
        case json::kind::int64:
            if(other.get_int64() < 0)
                return false;
            return static_cast<std::uint64_t>(
                other.get_int64()) == get_uint64();
        default:
            return false;
        }

    case json::kind::double_:
        return
            other.kind() == json::kind::double_ &&
            get_double() == other.get_double();

    case json::kind::string:
        return
            other.kind() == json::kind::string &&
            get_string() == other.get_string();

    case json::kind::array:
        return
            other.kind() == json::kind::array &&
            get_array() == other.get_array();

    case json::kind::object:
        return
            other.kind() == json::kind::object &&
            get_object() == other.get_object();
    }
}

//----------------------------------------------------------

namespace detail
{

std::size_t
hash_value_impl( value_view const& jv ) noexcept
{
    std::size_t seed = 0;

    kind const k = jv.kind();
    boost::hash_combine( seed, k != kind::int64 ? k : kind::uint64 );

    visit( value_view_hasher{seed}, jv );
    return seed;
}

} // namespace detail
} // namespace json
} // namespace boost

//----------------------------------------------------------
//
// std::hash specialization
//
//----------------------------------------------------------

std::size_t
std::hash<::boost::json::value_view>::operator()(
    ::boost::json::value_view const& jv) const noexcept
{
    return ::boost::hash< ::boost::json::value_view >()( jv );
}

//----------------------------------------------------------

#endif
