//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_BEAST_SRC_EXTRA_HPP
#define BOOST_BEAST_SRC_EXTRA_HPP

/*

This file is meant to be included once, in a translation unit of
the program, with the macro BOOST_BEAST_SPLIT_COMPILATION defined.

*/

#define BOOST_BEAST_SOURCE

#include <boost/beast/core/detail/config.hpp>

#if defined(BOOST_BEAST_HEADER_ONLY)
# error Do not compile Beast library source with BOOST_BEAST_HEADER_ONLY defined
#endif

#include <boost/json/detail/varint.ipp>
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
#include <boost/json/impl/value.ipp>

#endif
