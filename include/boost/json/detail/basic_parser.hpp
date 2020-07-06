//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_BASIC_PARSER_HPP
#define BOOST_JSON_DETAIL_BASIC_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/parse_options.hpp>
#include <boost/json/detail/stack.hpp>
#include <boost/json/detail/stream.hpp>
#include <string>
#include <vector>

/*  VFALCO NOTE

    This file is in the detail namespace because it
    is not allowed to be included directly by users,
    who should be including <boost/json/basic_parser.hpp>
    instead, which provides the member function template
    definitions.

    The source code is arranged this way to keep compile
    times down.
*/
namespace boost {
namespace json {

/** An incremental SAX parser for serialized JSON.

    This implements a SAX-style parser. The serialized
    JSON is presented to the parser by one or more calls
    to @ref write_some. The parsing events are realized
    through member function calls to a handler passed as
    an argument to the write function.

    <br>

    The parser may dynamically allocate intermediate
    storage as needed to accommodate the nesting level
    of the JSON being parsed. This storage is freed
    when the parser is destroyed, allowing the parser
    to cheaply re-use this memory when parsing
    subsequent JSONs, improving performance.

    @par Usage

    Users who wish to parse JSON into the DOM container
    @ref value will not use this class directly; instead
    they will create an instance of @ref parser and
    use that instead. Alternatively, they may call the
    function @ref parse. This class is designed for
    users who wish to perform custom actions instead of
    building a @ref value. For example, to produce a
    DOM from an external library.

    <br>

    To use this class it is necessary to create a derived
    class which calls @ref reset at the beginning of
    parsing a new JSON, and then calls @ref write_some one
    or more times with the serialized JSON.

    @note

    By default, only conforming JSON is accepted.
    However, extensions can be configured through the 
    use of @ref parse_options.

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
    std::size_t max_depth_ = 32;
    unsigned u1_;
    unsigned u2_;
    bool more_; // false for final buffer
    bool complete_ = false; // true on complete parse
    bool is_key_;
    parse_options opt_;

    inline static bool is_control(char c) noexcept;
    inline static char hex_digit(char c) noexcept;
    inline void reserve();
    inline void suspend(state st);
    inline void suspend(state st, number const& num);
    inline bool skip_white(const_stream& cs);

    template<result Result, class Handler>
    result constant_result(Handler&, const_stream&);

    template<bool StackEmpty, bool ReturnValue, bool TrailingCommas, class Handler>
    result parse_comment(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, class Handler>
    result parse_document(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, bool AllowComments, bool TrailingCommas, class Handler>
    result parse_value(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, bool AllowComments, bool TrailingCommas, class Handler>
    result resume_value(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, bool AllowComments, bool TrailingCommas, class Handler>
    result parse_object(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, bool AllowComments, bool TrailingCommas, class Handler>
    result parse_array(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, class Handler>
    result parse_null(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, class Handler>
    result parse_true(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, class Handler>
    result parse_false(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, class Handler>
    result parse_string(Handler& h, const_stream& cs);
    
    template<bool StackEmpty, char First, class Handler>
    result parse_number(Handler& h, const_stream& cs);

public:
     /** Default constructor.
        
        The parser will only recognize strict JSON.
    */
    basic_parser() noexcept = default;

    /** Constructor.
        
        Construct a parser with the provided options.

        @param opt The options for the parser.
    */
    inline
    basic_parser(const parse_options& opt) noexcept;

    /** Destructor.

        All dynamically allocated internal memory is freed.
    */
    virtual
    ~basic_parser()
    {
        // VFALCO defaulting this causes link
        // errors on some older toolchains.
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
    is_complete() const noexcept
    {
        return complete_;
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

protected:
    /** Reset the state, to parse a new document.
    */
    inline
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
        can be indicated by passing `more = false`.

        @par Handler

        The handler provided must be implemented as an
        object of class type which defines each of the
        required event member functions below. The event
        functions return a `bool` where `true` indicates
        success, and `false` indicates failure. If the
        member function returns `false`, it must set
        the error code to a suitable value. This error
        code will be returned by the write function to
        the caller.
        
        <br>
        
        The following declaration meets the parser's
        handler requirements:

        @code
        struct handler
        {
            // Called once when the JSON parsing begins.
            bool on_document_begin( error_code& ec );

            // Called when the JSON parsing is done.
            bool on_document_end( error_code& ec );

            // Called when the beginning of an object is encountered.
            bool on_object_begin( error_code& ec );

            // Called when the end of the current object is encountered.
            bool on_object_end( error_code& ec );

            // Called when the beginning of an array is encountered.
            bool on_array_begin( error_code& ec );

            // Called when the end of the current array is encountered.
            bool on_array_end( error_code& ec );

            // Called with characters corresponding to part of the current key.
            bool on_key_part( string_view s, error_code& ec );

            // Called with the last characters corresponding to the current key.
            bool on_key( string_view s, error_code& ec );

            // Called with characters corresponding to part of the current string.
            bool on_string_part( string_view s, error_code& ec );

            // Called with the last characters corresponding to the current string.
            bool on_string( string_view s, error_code& ec );

            // Called when a signed integer is parsed.
            bool on_int64( int64_t i, error_code& ec );

            // Called when an unsigend integer is parsed.
            bool on_uint64( uint64_t u, error_code& ec );

            // Called when a double is parsed.
            bool on_double( double d, error_code& ec );

            // Called when a boolean is parsed.
            bool on_bool( bool b, error_code& ec );

            // Called when a null is parsed.
            bool on_null( error_code& ec );

            // Called with characters corresponding to part of the current comment.
            bool on_comment_part( string_view s, error_code& ec );

            // Called with the last characters corresponding to the current comment.
            bool on_comment( string_view s, error_code& ec );
        };
        @endcode

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
        Handler& h,
        bool more,
        char const* data,
        std::size_t size,
        error_code& ec);
};

} // json
} // boost

#endif
