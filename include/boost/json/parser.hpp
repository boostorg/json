//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_PARSER_HPP
#define BOOST_JSON_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/basic_parser.hpp>
#include <boost/json/storage.hpp>
#include <boost/json/value.hpp>
#include <boost/json/string.hpp>
#include <boost/json/detail/static_stack.hpp>
#include <boost/json/detail/string.hpp>
#include <new>
#include <string>
#include <type_traits>

namespace boost {
namespace json {

class parser final
    : public basic_parser
{
    using size_type = unsigned;

    static std::size_t const
        default_max_depth = 32;

    union
    {
        char buf_[512];
        char* alloc_;
    };
    unsigned capacity_ = sizeof(buf_);
    value jv_;
    detail::static_stack<value*,
        default_max_depth> stack_;
    string s_;
    unsigned long max_depth_ =
        default_max_depth;
    bool obj_;

public:
    BOOST_JSON_DECL
    virtual
    ~parser();

    BOOST_JSON_DECL
    parser();

    BOOST_JSON_DECL
    parser(storage_ptr sp);

    /** Returns the maximum allowed depth of input JSON.
    */
    BOOST_JSON_DECL
    std::size_t
    max_depth() const noexcept;

    /** Set the maximum allowed depth of input JSON.
    */
    BOOST_JSON_DECL
    void
    max_depth(unsigned long levels) noexcept;

    BOOST_JSON_DECL
    value const&
    get() const noexcept;

    BOOST_JSON_DECL
    value
    release() noexcept;

private:
    BOOST_JSON_DECL
    void
    reset();

    BOOST_JSON_DECL
    void
    on_stack_info(
        stack& s) noexcept override;

    BOOST_JSON_DECL
    void
    on_stack_grow(
        stack& s,
        unsigned capacity,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_document_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_object_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_object_end(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_array_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_array_end(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_key_data(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_key_end(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string_data(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string_end(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_int64(
        int64_t i,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_uint64(
        uint64_t u,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_double(
        double d,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_bool(
        bool b,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_null(error_code&) override;
};

//----------------------------------------------------------

BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp,
    error_code& ec);

inline
value
parse(
    string_view s,
    error_code& ec)
{
    return parse(s,
        storage_ptr{}, ec);
}

BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp);

inline
value
parse(string_view s)
{
    return parse(s, storage_ptr{});
}

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/parser.ipp>
#endif

#endif
