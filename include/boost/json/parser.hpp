//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_PARSER_HPP
#define BOOST_JSON_PARSER_HPP

#include <boost/json/config.hpp>
#include <boost/json/basic_parser.hpp>
#include <boost/json/storage.hpp>
#include <boost/json/value.hpp>
#include <boost/json/string.hpp>
#include <boost/json/detail/raw_stack.hpp>
#include <new>
#include <string>
#include <type_traits>
#include <stddef.h>

namespace boost {
namespace json {

//----------------------------------------------------------

/** A DOM parser for serialized JSON.

    This parser is used to incrementally parse
    JSON from character buffers into a @ref value
    container.

    @par Usage

    Before parsing a new JSON, the function @ref start
    must be called, optionally passing the storage
    pointer to be used by the @ref value container into
    which the parsed results are stored. After the
    parse is started, the functions @ref write_some,
    @ref write, and @ref finish may be called to
    provide successive buffers of characters of the
    JSON. The caller can check that the parse is
    complete by calling @ref is_done, or that a
    non-successful error code is returned.

    @par Incremental Parsing

    The @ref write_some function allows partial data
    to be written. This is useful when not all of the
    serialized JSON is present at once and it is
    desired to process the data as it becomes available,
    such as when reading from a network socket or other
    device. The incremental interface may also be used
    to bound the amount of work performed in each
    parsing cycle.

    <br>

    The parser may dynamically allocate intermediate
    storage as needed to accommodate the nesting level
    of the JSON being parsed. This storage is freed
    when the parser is destroyed, allowing the parser
    to cheaply re-use this memory when parsing
    subsequent JSONs, improving performance.
*/
class parser
{
    friend class basic_parser;
    enum class state : char;
    struct level
    {
        std::uint32_t count;
        char align;
        state st;
    };

    basic_parser p_;
    storage_ptr sp_;
    detail::raw_stack rs_;
    std::size_t max_depth_ = 32;
    std::uint32_t key_size_ = 0;
    std::uint32_t str_size_ = 0;
    level lev_;

    inline
    void
    destroy() noexcept;

public:
    /** Destructor.

        All dynamically allocated memory, including
        any partial parsing results, is freed.
    */
    BOOST_JSON_DECL
    virtual
    ~parser();

    /** Default constructor.

        Before any JSON can be parsed, the function
        @ref start must be called.
    */
    BOOST_JSON_DECL
    parser();

    /** Reserve internal storage space.

        This function reserves space for `n` bytes
        in the parser's internal temporary storage.
        The request is only a hint to the
        implementation. 

        @par Exception Safety

        No-throw guarantee.

        @param n The number of bytes to reserve. A
        good choices is `C * sizeof(value)` where
        `C` is the total number of @ref value elements
        in a typical parsed JSON.
    */
    BOOST_JSON_DECL
    void
    reserve(std::size_t n) noexcept;

    /** Start parsing JSON incrementally.

        This function must be called once manually before
        parsing a new JSON incrementally; that is, when
        using @ref write_some, @ref write, or @ref finish.

        @param sp A pointer to the @ref storage
        to use. The parser will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    void
    start(storage_ptr sp = {}) noexcept;

    /** Returns the current depth of the JSON being parsed.

        The parsing depth is the total current nesting
        level of arrays and objects.
    */
    std::size_t
    depth() const noexcept
    {
        return p_.depth();
    }

    /** Returns the maximum allowed depth of input JSON.

        The maximum allowed depth may be configured.
    */
    std::size_t
    max_depth() const noexcept
    {
        return max_depth_;
    }

    /** Set the maximum allowed depth of input JSON.

        When the maximum depth is exceeded, parser
        operations will return @ref error::too_deep.

        @param levels The maximum depth.
    */
    void
    max_depth(unsigned long levels) noexcept
    {
        max_depth_ = levels;
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
        return p_.is_done();
    }

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.
        
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

        @throw system_error Thrown on failure.

        @return The number of characters successfully
        parsed, which may be smaller than `size`.
    */
    BOOST_JSON_DECL
    std::size_t
    write_some(
        char const* data,
        std::size_t size);

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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

        This function parses the JSON in the specified
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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

        This function parses the JSON in the specified
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.

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

    /** Discard all parsed JSON results.

        This function destroys all intermediate parsing
        results, while preserving dynamically allocated
        internal memory which may be reused on a
        subsequent parse.

        @note

        After this call, it is necessary to call
        @ref start to parse a new JSON incrementally.
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    /** Return the parsed JSON as a @ref value.

        If @ref is_done() returns `true`, then the
        parsed value is returned. Otherwise an
        exception is thrown.

        @throw std::logic_error `! is_done()`

        @return The parsed value. Ownership of this
        value is transferred to the caller.       
    */
    BOOST_JSON_DECL
    value
    release();

private:
    template<class T>
    void
    push(T const& t);

    inline
    void
    push_chars(string_view s);

    template<class... Args>
    void
    emplace_object(Args&&... args);

    template<class... Args>
    void
    emplace_array(Args&&... args);

    template<class... Args>
    void
    emplace(Args&&... args);

    template<class T>
    void
    pop(T& t);

    inline
    detail::unchecked_object
    pop_object() noexcept;

    inline
    detail::unchecked_array
    pop_array() noexcept;

    inline
    string_view
    pop_chars(
        std::size_t size) noexcept;

    inline
    bool
    on_document_begin(
        error_code& ec);

    inline
    bool
    on_document_end(
        error_code& ec);

    inline
    bool
    on_object_begin(
        error_code& ec);

    inline
    bool
    on_object_end(
        std::size_t n,
        error_code& ec);

    inline
    bool
    on_array_begin(
        error_code& ec);

    inline
    bool
    on_array_end(
        std::size_t n,
        error_code& ec);

    inline
    bool
    on_key_part(
        string_view s,
        error_code& ec);

    inline
    bool
    on_key(
        string_view s,
        error_code& ec);

    inline
    bool
    on_string_part(
        string_view s,
        error_code& ec);

    inline
    bool
    on_string(
        string_view s,
        error_code& ec);

    inline
    bool
    on_int64(
        int64_t i,
        error_code& ec);

    inline
    bool
    on_uint64(
        uint64_t u,
        error_code& ec);

    inline
    bool
    on_double(
        double d,
        error_code& ec);

    inline
    bool
    on_bool(
        bool b,
        error_code& ec);

    inline
    bool
    on_null(error_code&);
};

//----------------------------------------------------------

/** Parse a string of JSON.

    This function parses an entire single string in
    one step to produce a complete JSON object, returned
    as a @ref value. If the buffer does not contain a
    complete serialized JSON, an error occurs. In this
    case the returned value will be null, using the
    default storage.

    @par Complexity

    Linear in `s.size()`.

    @par Exception Safety

    Strong guarantee.
    Calls to @ref storage::allocate may throw.

    @param s The string to parse.

    @param ec Set to the error, if any occurred.

    @param sp The storage that the new value and all of
    its elements will use. If this parameter is omitted,
    the default storage is used.

    @return A value representing the parsed JSON,
    or a null if any error occurred.
*/
BOOST_JSON_DECL
value
parse(
    string_view s,
    error_code& ec,
    storage_ptr sp = {});

/** Parse a string of JSON.

    This function parses an entire single string in
    one step to produce a complete JSON object, returned
    as a @ref value. If the buffer does not contain a
    complete serialized JSON, an error occurs. In this
    case the returned value will be null, using the
    default storage.

    @par Complexity

    Linear in `s.size()`.

    @par Exception Safety

    Strong guarantee.
    Calls to @ref storage::allocate may throw.

    @param s The string to parse.

    @param sp The storage that the new value and all of
    its elements will use. If this parameter is omitted,
    the default storage is used.

    @throw system_error Thrown on failure.

    @return A value representing the parsed JSON,
    or a null if any error occurred.
*/
BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp = {});

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/parser.ipp>
#endif

#endif
