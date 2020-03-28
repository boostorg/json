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
#include <boost/json/kind.hpp>
#include <boost/json/detail/stack.hpp>
#include <boost/json/detail/stream.hpp>
#include <string>
#include <vector>

namespace boost {
namespace json {

/** An incremental SAX parser for serialized JSON.

    This implements a SAX-style parser. The serialized
    JSON is presented to the parser by calling to
    @ref write_some, @ref write, and @ref finish.
    The parsing events are realized through calls of
    protected virtual functions, whose implementations
    are in the derived class.

    <br>

    The parser may dynamically allocate intermediate
    storage as needed to accommodate the nesting level
    of the JSON being parsed. This storage is freed
    when the parser is destroyed, allowing the parser
    to cheaply re-use this memory when parsing
    subsequent JSONs, improving performance.

    @note

    The parser is strict: no extensions are supported.
    Only compliant JSON is recognized.

    @see @ref parse, @ref parser
*/
class basic_parser
{
    enum class state : char;
    using const_stream = detail::const_stream;

    enum result
    {
        ok = 0,
        fail,
        partial
    };

    struct number
    {
        uint64_t mant;
        int bias;
        int exp;
        bool frac;
        bool neg;
    };

    number num_;
    error_code ec_;
    detail::stack st_;
    std::size_t depth_ = 0;
    unsigned u1_;
    unsigned u2_;
    bool done_; // true on complete parse
    bool more_; // false for final buffer
    bool is_key_;

    inline static bool is_control(char c) noexcept;
    inline static char hex_digit(char c) noexcept;
    inline void reserve();
    inline void suspend(state st);
    inline void suspend(state st, std::size_t n);
    inline void suspend(state st, number const& num);
    inline bool skip_white(const_stream& cs);
    template<class Handler> inline result parse_element(Handler h, const_stream& cs);
    template<class Handler> inline result parse_value(Handler h, const_stream& cs);
    template<class Handler> inline result parse_null(Handler h, const_stream& cs);
    template<class Handler> inline result parse_true(Handler h, const_stream& cs);
    template<class Handler> inline result parse_false(Handler h, const_stream& cs);
    template<class Handler> inline result parse_string(Handler h, const_stream& cs);
    template<class Handler> inline result parse_object(Handler h, const_stream& cs);
    template<class Handler> inline result parse_array(Handler h, const_stream& cs);
    template<class Handler> inline result parse_number(Handler h, const_stream& cs);

public:
    //------------------------------------------------------

    template<class T>
    struct handler_ref
    {
        T& p;
        bool on_document_begin( error_code& ec ) { return p.on_document_begin(ec); }
        bool on_document_end( error_code& ec ) { return p.on_document_end(ec); }
        bool on_object_begin( error_code& ec ) { return p.on_object_begin(ec); }
        bool on_object_end( std::size_t n, error_code& ec ) { return p.on_object_end(n, ec); }
        bool on_array_begin( error_code& ec ) { return p.on_array_begin(ec); }
        bool on_array_end( std::size_t n, error_code& ec ) { return p.on_array_end(n, ec); }
        bool on_key_part( string_view s, error_code& ec ) { return p.on_key_part(s, ec); }
        bool on_key( string_view s, error_code& ec ) { return p.on_key(s, ec); }
        bool on_string_part( string_view s, error_code& ec ) { return p.on_string_part(s, ec); }
        bool on_string( string_view s, error_code& ec ) { return p.on_string(s, ec); }
        bool on_int64( std::int64_t i, error_code& ec ) { return p.on_int64(i, ec); }
        bool on_uint64( std::uint64_t u, error_code& ec ) { return p.on_uint64(u, ec); }
        bool on_double( double d, error_code& ec ) { return p.on_double(d, ec); }
        bool on_bool( bool b, error_code& ec ) { return p.on_bool(b, ec); }
        bool on_null( error_code& ec ) { return p.on_null(ec); }
    };

    template<class T>
    static
    handler_ref<T>
    handler(T& t)
    {
        return handler_ref<T>{t};
    }

    /** Destructor.

        All dynamically allocated internal memory is freed.
    */
    virtual
    ~basic_parser()
    {
        // VFALCO defaulting this causes link
        // errors on some older toolchains.
    }

    /// Constructor (default)
    BOOST_JSON_DECL
    basic_parser();

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
        return done_;
    }

    /** Returns the current depth of the JSON being parsed.

        The parsing depth is the total current nesting
        level of arrays and objects.
    */
    std::size_t
    depth() const noexcept
    {
        return depth_;
    }

    /** Reset the state, to parse a new document.
    */
    BOOST_JSON_DECL
    void
    reset() noexcept;

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, calling the handler to emit each SAX
        parsing event. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed, or

        @li Some of the characters in the buffer have been
        parsed and the JSON is complete, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        can be indicated by calling @ref finish().

        @par Complexity

        Linear in `size`.

        @param h The handler to invoke for each element
        of the parsed JSON.

        @param more `true` if there are possibly more
        buffers in the current JSON, otherwise `false`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @param ec Set to the error, if any occurred.

        @return The number of characters successfully
        parsed, which may be smaller than `size`.
    */
    template<class Handler>
    std::size_t
    write_some(
        Handler h,
        bool more,
        char const* data,
        std::size_t size,
        error_code& ec);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, emitting SAX parsing events by calling
        the derived class. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed, or

        @li Some of the characters in the buffer have been
        parsed and the JSON is complete, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        can be indicated by calling @ref finish().

        @par Complexity

        Linear in `size`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @param ec Set to the error, if any occurred.

        @return The number of characters successfully
        parsed, which may be smaller than `size`.
    */
    BOOST_JSON_DECL
    std::size_t
    write_some(
        char const* data,
        std::size_t size,
        error_code& ec);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, emitting SAX parsing events by calling
        the derived class. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        can be indicated by calling @ref finish().

        @par Complexity

        Linear in `size`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    void
    write(
        char const* data,
        std::size_t size,
        error_code& ec);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, emitting SAX parsing events by calling
        the derived class. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        can be indicated by calling @ref finish().

        @par Complexity

        Linear in `size`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @throw system_error Thrown on failure.
    */
    BOOST_JSON_DECL
    void
    write(
        char const* data,
        std::size_t size);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, emitting SAX parsing events by calling
        the derived class. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed and form a complete JSON, or

        @li A parsing error occurs.

        The caller uses this function to inform the
        parser that there is no more serialized JSON
        available. If the entire buffer is not consumed
        or if a complete JSON is not available after
        consuming the entire buffer, the error is
        set to indicate failure.

        @par Complexity

        Linear in `size`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    void
    finish(
        char const* data,
        std::size_t size,
        error_code& ec);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer, emitting SAX parsing events by calling
        the derived class. The parse proceeds from the
        current state, which is at the beginning of a
        new JSON or in the middle of the current JSON
        if any characters were already parsed.

        <br>

        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have been
        parsed and form a complete JSON, or

        @li A parsing error occurs.

        The caller uses this function to inform the
        parser that there is no more serialized JSON
        available. If the entire buffer is not consumed
        or if a complete JSON is not available after
        consuming the entire buffer, the error is
        set to indicate failure.

        @par Complexity

        Linear in `size`.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @throw system_error Thrown on failure.
    */
    BOOST_JSON_DECL
    void
    finish(
        char const* data,
        std::size_t size);

    /** Parse JSON incrementally.

        This function finishes parsing the current JSON,
        emitting SAX parsing events by calling the derived
        class. The parse is finalized according to the
        current state, which includes all previously
        parsed data since the last reset.

        <br>

        The caller uses this function to inform the
        parser that there is no more serialized JSON
        available. If a complete JSON is not available
        the error is set to indicate failure.

        @par Complexity

        Constant.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    void
    finish(error_code& ec);

    /** Parse JSON incrementally.

        This function finishes parsing the current JSON,
        emitting SAX parsing events by calling the derived
        class. The parse is finalized according to the
        current state, which includes all previously
        parsed data since the last reset.

        <br>

        The caller uses this function to inform the
        parser that there is no more serialized JSON
        available. If a complete JSON is not available
        the error is set to indicate failure.

        @par Complexity

        Constant.

        @throw system_error Thrown on failure.
    */
    BOOST_JSON_DECL
    void
    finish();

protected:
    /** Called once when the JSON parsing begins.

        This function is invoked at the beginning of
        parsing a new JSON document, when data is first
        presented. The call happens before any other
        events.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_document_begin(
        error_code& ec) { return false; }

    /** Called when the JSON parsing is done.

        This function is invoked once when a complete
        JSON document has been parsed without errors.
        The call happens last; no other calls are made
        before the parser is reset.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_document_end(
        error_code& ec) { return false; }

    /** Called when the beginning of an object is encountered.

        This function is called during parsing when a new
        object is started.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_object_begin(
        error_code& ec) { return false; }

    /** Called when the end of the current object is encountered.

        This function is called during parsing after all of
        the elements of an object have been parsed, and the
        closing brace for the object is reached.

        @param n The number of elements in the object.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_object_end(
        std::size_t n,
        error_code& ec) { return false; }

    /** Called when the beginning of an array is encountered.

        This function is called during parsing when a new
        array is started.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_array_begin(
        error_code& ec) { return false; }

    /** Called when the end of the current array is encountered.

        This function is called during parsing after all of
        the elements of an array have been parsed, and the
        closing bracket for the array is reached.

        @param n The number of elements in the array.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_array_end(
        std::size_t n,
        error_code& ec) { return false; }

    /** Called with characters corresponding to part of the current key.

        This function is called during parsing to provide
        the characters corresponding to the key being parsed
        as part of an object. More key data may be provided
        by zero or more calls to @ref on_key_part, followed
        by a final call to @ref on_key.

        @param s The string view holding the next buffer of
        key data, with escapes converted to their actual
        characters.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_key_part(
        string_view s,
        error_code& ec) { return false; }

    /** Called with the last characters corresponding to the current key.

        This function is called during parsing to provide
        the final character string corresponding to the key
        being parsed as part of an object. The key is
        considered complete with these characters.

        @param s The string view holding the final buffer of
        key data, with escapes converted to their actual
        characters.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_key(
        string_view s,
        error_code& ec) { return false; }

    /** Called with characters corresponding to part of the current string.

        This function is called during parsing to provide
        the characters corresponding to a string value being
        parsed. More string data may be provided by zero or
        more calls to @ref on_string_part, followed by a
        final call to @ref on_string.

        @param s The string view holding the next buffer of
        string data, with escapes converted to their actual
        characters.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_string_part(
        string_view s,
        error_code& ec) { return false; }

    /** Called with the last characters corresponding to the current string.

        This function is called during parsing to provide
        the final character string corresponding to the
        string being parsed. The string element is
        considered complete with these characters.

        @param s The string view holding the final buffer of
        string data, with escapes converted to their actual
        characters.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_string(
        string_view s,
        error_code& ec) { return false; }

    /** Called when a `std::int64_t` is parsed.

        This function is called when a suitable
        number is parsed.

        @param i The number encountered.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_int64(
        int64_t i,
        error_code& ec) { return false; }

    /** Called when a `std::uint64_t` is parsed.

        This function is called when a suitable
        number is parsed.

        @param u The number encountered.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_uint64(
        uint64_t u,
        error_code& ec) { return false; }

    /** Called when a `double` is parsed.

        This function is called when a suitable
        number is parsed.

        @param d The number encountered.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_double(
        double d,
        error_code& ec) { return false; }

    /** Called when a `bool` is parsed.

        This function is called when a
        boolean value is encountered.

        @param b The boolean value.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_bool(bool b, error_code& ec) { return false; }

    /** Called when a null is parsed.

        This function is called when a null is encountered.

        @param ec The error, if any, which will be
        returned by the current invocation of
        @ref write_some, @ref write, or @ref finish.

        @return `true` on success.
    */
    virtual
    bool
    on_null(error_code& ec) { return false; }
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/basic_parser.ipp>
#endif

#endif
