//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_BASIC_PARSER_HPP
#define BOOST_JSON_BASIC_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/number.hpp>
#include <boost/json/detail/basic_parser.hpp>
#include <boost/json/detail/stack.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>

#include <string>
#include <vector>

namespace boost {
namespace beast {
namespace json {

/** A parser for serialized JSON
*/
class basic_parser
#ifndef GENERATING_DOCUMENTATION
    : private detail::parser_base
#endif
{
    enum class state : char;

    /// Depth to which the stack does not require dynamic allocation
    static std::size_t const stack_capacity = 64;

    detail::stack<
        state, stack_capacity> stack_;
    number::mantissa_type n_mant_;
    number::exponent_type n_exp_;
    bool n_neg_;
    bool n_exp_neg_;
    bool is_key_;

    BOOST_JSON_DECL
    static
    bool
    append_digit(
        number::mantissa_type* value,
        char digit);

    BOOST_JSON_DECL
    static
    bool
    append_digit(
        number::exponent_type* value,
        char digit, bool neg);

public:
    /// Returns `true` if the parser has completed without error
    BOOST_JSON_DECL
    bool
    is_done() const noexcept;

    /** Reset the state, to parse a new document.
    */
    BOOST_JSON_DECL
    void
    reset();

    template<
        class ConstBufferSequence
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            ! std::is_convertible<
                ConstBufferSequence,
                boost::asio::const_buffer>::value>::type
#endif
    >
    std::size_t
    write_some(
        ConstBufferSequence const& buffers,
        error_code& ec);

    BOOST_JSON_DECL
    std::size_t
    write_some(
        boost::asio::const_buffer buffer,
        error_code& ec);

    template<
        class ConstBufferSequence
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            ! std::is_convertible<
                ConstBufferSequence,
                boost::asio::const_buffer>::value>::type
#endif
    >
    std::size_t
    write(
        ConstBufferSequence const& buffers,
        error_code& ec);

    BOOST_JSON_DECL
    std::size_t
    write(
        boost::asio::const_buffer buffer,
        error_code& ec);

    BOOST_JSON_DECL
    void
    write_eof(error_code& ec);

protected:
    /// Constructor (default)
    BOOST_JSON_DECL
    basic_parser();

    virtual
    void
    on_document_begin(error_code& ec) = 0;

    virtual
    void
    on_object_begin(error_code& ec) = 0;

    virtual
    void
    on_object_end(error_code& ec) = 0;

    virtual
    void
    on_array_begin(error_code& ec) = 0;

    virtual
    void
    on_array_end(error_code& ec) = 0;

    virtual
    void
    on_key_data(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_key_end(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_string_data(
        string_view s,
        error_code& ec) = 0;

    virtual
    void
    on_string_end(
        string_view,
        error_code& ec) = 0;

    virtual
    void
    on_number(number n, error_code& ec) = 0;

    virtual
    void
    on_bool(bool b, error_code& ec) = 0;

    virtual
    void
    on_null(error_code& ec) = 0;
};

} // json
} // beast
} // boost

#include <boost/json/impl/basic_parser.hpp>
#if BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/basic_parser.ipp>
#endif

#endif
