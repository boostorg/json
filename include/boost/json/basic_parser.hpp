//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_BASIC_PARSER_HPP
#define BOOST_JSON_BASIC_PARSER_HPP

#include <boost/json/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/detail/number.hpp>
#include <boost/json/detail/static_stack.hpp>
#include <string>
#include <vector>
#include <stdint.h>

namespace boost {
namespace json {

/** A parser for serialized JSON
*/
class basic_parser
{
    enum class state : char;

    detail::static_stack<state, 8> st_;
    detail::number_parser iep_;
    std::size_t depth_ = 0;
    std::size_t max_depth_ = 32;
    char const* lit_;
    error ev_;
    long u0_;
    unsigned short u_;
    bool is_key_;

    inline
    bool
    is_control(char c) noexcept;

    inline
    char
    hex_digit(char c) noexcept;

public:
    virtual
    ~basic_parser()
    {
        // VFALCO defaulting this causes link
        // link errors on some older toolchains.
    }

    /** Return true if a complete JSON has been parsed.

        This function returns `true` when all of these
        conditions are met:

        @li A complete serialized JSON has been
            presented to the parser, and

        @li No error has occurred since the parser
            was constructed, or since the last call
            to @ref reset,

        @par Complexity

        Constant.
    */
    bool
    is_done() const noexcept
    {
        return static_cast<
            char>(*st_) == 0;
    }

    /** Returns the current depth of the JSON being parsed.
    */
    std::size_t
    depth() const noexcept
    {
        return depth_;
    }

    /** Returns the maximum allowed depth of input JSON.
    */
    std::size_t
    max_depth() const noexcept
    {
        return max_depth_;
    }

    /** Set the maximum allowed depth of input JSON.
    */
    void
    max_depth(unsigned long levels) noexcept
    {
        max_depth_ = levels;
    }

    BOOST_JSON_DECL
    std::size_t
    write_some(
        char const* data,
        std::size_t size,
        error_code& ec);

    BOOST_JSON_DECL
    std::size_t
    write(
        char const* data,
        std::size_t size,
        error_code& ec);

    BOOST_JSON_DECL
    void
    write_eof(error_code& ec);

protected:
    using saved_state = char;

    /// Constructor (default)
    BOOST_JSON_DECL
    basic_parser();

    /** Reset the state, to parse a new document.
    */
    BOOST_JSON_DECL
    void
    reset() noexcept;

    saved_state
    save_state() noexcept
    {
        auto ss = *st_;
        st_.pop();
        return static_cast<
            saved_state>(ss);
    }

    void
    restore_state(
        saved_state ss) noexcept
    {
        st_.push(static_cast<
            state>(ss));
    }

    virtual
    void
    on_document_begin(
        error_code& ec) = 0;

    virtual
    void
    on_document_end(
        error_code& ec) = 0;

    virtual
    void
    on_object_begin(
        error_code& ec) = 0;

    virtual
    void
    on_object_end(
        error_code& ec) = 0;

    virtual
    void
    on_array_begin(
        error_code& ec) = 0;

    virtual
    void
    on_array_end(
        error_code& ec) = 0;

    virtual
    void
    on_key_part(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_key(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_string_part(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_string(
        string_view,
        error_code& ec) = 0;

    virtual
    void
    on_int64(
        int64_t i,
        error_code& ec) = 0;

    virtual
    void
    on_uint64(
        uint64_t u,
        error_code& ec) = 0;

    virtual
    void
    on_double(
        double d,
        error_code& ec) = 0;

    virtual
    void
    on_bool(bool b, error_code& ec) = 0;

    virtual
    void
    on_null(error_code& ec) = 0;
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/basic_parser.ipp>
#endif

#endif
