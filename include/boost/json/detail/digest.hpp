//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_DIGEST_HPP
#define BOOST_JSON_DETAIL_DIGEST_HPP

BOOST_JSON_NS_BEGIN
namespace detail {

// Calculate salted digest of string
BOOST_JSON_DECL
std::size_t
digest(
    char const* s,
    std::size_t n,
    std::size_t salt) noexcept;

} // detail
BOOST_JSON_NS_END

#endif
