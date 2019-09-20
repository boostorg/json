//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VARINT_HPP
#define BOOST_JSON_DETAIL_VARINT_HPP

#include <cstdlib>
#include <cstdint>
#include <utility>

namespace boost {
namespace beast {
namespace json {
namespace detail {

BOOST_JSON_DECL
int
varint_size(std::uint64_t value);

BOOST_JSON_DECL
std::pair<std::uint64_t, int>
varint_read(void const* src);

BOOST_JSON_DECL
int
varint_write(
    void* dest,
    std::uint64_t value);

} // detail
} // json
} // beast
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/varint.ipp>
#endif

#endif
