//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_HANDLER_HPP
#define BOOST_JSON_DETAIL_HANDLER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/string.hpp>
#include <boost/json/value_stack.hpp>

namespace boost {
namespace json {
namespace detail {

struct handler
{
    static constexpr std::size_t
        max_object_size = object::max_size();

    static constexpr std::size_t
        max_array_size = array::max_size();

    static constexpr std::size_t
        max_key_size = string::max_size();

    static constexpr std::size_t
        max_string_size = string::max_size();

    value_stack st;

    template<class... Args>
    explicit
    handler(Args&&... args);

    inline std::true_type on_document_begin(error_code&) const;
    inline std::true_type on_document_end(error_code&) const;
    inline std::true_type on_object_begin(error_code&) const;
    inline std::true_type on_object_end(std::size_t n, error_code& ec);
    inline std::true_type on_array_begin(error_code&) const;
    inline std::true_type on_array_end(std::size_t n, error_code& ec);
    inline std::true_type on_key_part(string_view s, std::size_t n, error_code& ec);
    inline std::true_type on_key(string_view s, std::size_t n, error_code& ec);
    inline std::true_type on_string_part(string_view s, std::size_t n, error_code& ec);
    inline std::true_type on_string(string_view s, std::size_t n, error_code& ec);
    inline std::true_type on_number_part(string_view, error_code&) const;
    inline std::true_type on_int64(std::int64_t i, string_view, error_code& ec);
    inline std::true_type on_uint64(std::uint64_t u, string_view, error_code& ec);
    inline std::true_type on_double(double d, string_view, error_code& ec);
    inline std::true_type on_bool(bool b, error_code& ec);
    inline std::true_type on_null(error_code& ec);
    inline std::true_type on_comment_part(string_view, error_code&) const;
    inline std::true_type on_comment(string_view, error_code&) const;
};

} // detail
} // namespace json
} // namespace boost

#endif
