//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_SRC_HPP
#define BOOST_JSON_SRC_HPP

/*

This file is meant to be included once,
in a translation unit of the program.

*/

#ifndef BOOST_JSON_SOURCE
#define BOOST_JSON_SOURCE
#endif

#include <boost/json/detail/config.hpp>

#include <boost/json/impl/array.ipp>
#include <boost/json/impl/error.ipp>
#include <boost/json/impl/monotonic_resource.ipp>
#include <boost/json/impl/object.ipp>
#include <boost/json/impl/parse.ipp>
#include <boost/json/impl/parser.ipp>
#include <boost/json/impl/serializer.ipp>
#include <boost/json/impl/static_resource.ipp>
#include <boost/json/impl/string.ipp>
#include <boost/json/impl/to_string.ipp>
#include <boost/json/impl/value.ipp>
#include <boost/json/impl/value_stack.ipp>
#include <boost/json/impl/value_ref.ipp>

#include <boost/json/detail/impl/array_impl.ipp>
#include <boost/json/detail/impl/counted_resource.ipp>
#include <boost/json/detail/impl/default_resource.ipp>
#include <boost/json/detail/impl/digest.ipp>
#include <boost/json/detail/impl/except.ipp>
#include <boost/json/detail/impl/format.ipp>
#include <boost/json/detail/impl/object_impl.ipp>
#include <boost/json/detail/impl/raw_stack.ipp>
#include <boost/json/detail/impl/stack.ipp>
#include <boost/json/detail/impl/string_impl.ipp>

#include <boost/json/detail/ryu/impl/d2s.ipp>

#endif
