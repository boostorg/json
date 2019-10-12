//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_SRC_HPP
#define BOOST_JSON_SRC_HPP

/*

This file is meant to be included once, in a translation unit of
the program, with the macro BOOST_BEAST_SPLIT_COMPILATION defined.

*/

#include <boost/json/detail/config.hpp>

#if defined(BOOST_JSON_HEADER_ONLY)
# error Do not compile library source with the header only macro defined
#endif

#include <boost/json/impl/array.ipp>
#include <boost/json/impl/basic_parser.ipp>
#include <boost/json/impl/error.ipp>
#include <boost/json/impl/iterator.ipp>
#include <boost/json/impl/number.ipp>
#include <boost/json/impl/object.ipp>
#include <boost/json/impl/parse_file.ipp>
#include <boost/json/impl/parser.ipp>
#include <boost/json/impl/serializer.ipp>
#include <boost/json/impl/storage.ipp>
#include <boost/json/impl/string.ipp>
#include <boost/json/impl/value.ipp>

#endif
