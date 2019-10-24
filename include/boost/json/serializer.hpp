//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_SERIALIZER_HPP
#define BOOST_JSON_SERIALIZER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/number.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/iterator.hpp>
#include <boost/json/detail/string.hpp>
#include <iosfwd>

namespace boost {
namespace json {

class serializer
{
    enum class state : char;

    detail::const_iterator it_;
    string_view str_;
    unsigned char nbuf_;
    bool key_;
    char buf_[number::max_string_chars + 1];
    state state_;

public:
    BOOST_JSON_DECL
    explicit
    serializer(value const& jv);

    BOOST_JSON_DECL
    bool
    is_done() const noexcept;

    BOOST_JSON_DECL
    std::size_t
    next(char* dest, std::size_t size);
};

BOOST_JSON_DECL
std::string
to_string(value const& jv);

BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    value const& jv);

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
