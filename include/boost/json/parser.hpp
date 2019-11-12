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
#include <boost/json/detail/except.hpp>
#include <boost/json/detail/raw_stack.hpp>
#include <boost/json/detail/string.hpp>
#include <new>
#include <string>
#include <type_traits>
#include <stddef.h>

namespace boost {
namespace json {

class parser final
    : public basic_parser
{
    enum class state : char;

    storage_ptr sp_;
    detail::raw_stack rs_;
    std::size_t count_ = 0;
    std::size_t key_size_ = 0;
    std::size_t str_size_ = 0;
    state st_;

    inline
    void
    destroy() noexcept;

public:
    /** Destructor.
    */
    BOOST_JSON_DECL
    virtual
    ~parser();

    /** Default constructor.
    */
    BOOST_JSON_DECL
    parser();

    /** Prepare the parser for new serialized JSON.

        @param sp The storage to use for all values.
    */
    BOOST_JSON_DECL
    void
    start(storage_ptr sp = {}) noexcept;

    BOOST_JSON_DECL
    void
    clear() noexcept;

    /**

        @par Preconditions

        `is_done() == true`.
    */
    BOOST_JSON_DECL
    value
    release() noexcept;

private:
    template<class T>
    void
    push(T const& t);

    inline
    void
    push_chars(string_view s);

    template<class... Args>
    void
    emplace(Args&&... args);

    template<class T>
    void
    pop(T& t);

    inline
    unchecked_object
    pop_object() noexcept;

    inline
    unchecked_array
    pop_array() noexcept;

    inline
    string_view
    pop_chars(
        std::size_t size) noexcept;

    BOOST_JSON_DECL
    void
    on_document_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_document_end(
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
    on_key_part(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_key(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string_part(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string(
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

BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp);

inline
value
parse(
    string_view s,
    error_code& ec)
{
    return parse(s,
        storage_ptr{}, ec);
}

inline
value
parse(string_view s)
{
    return parse(
        s, storage_ptr{});
}

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/parser.ipp>
#endif

#endif
