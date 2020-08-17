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
    instead, which provides the member function definitions.

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
template<class Handler>
class basic_parser
{
    enum class state : char
    {
        doc1,  doc2,  doc3,
        com1,  com2,  com3,
        com4,  com5,  com6, 
        com7,  com8,  com9, 
        com10, com11, com12,
        nul1,  nul2,  nul3,
        tru1,  tru2,  tru3,
        fal1,  fal2,  fal3,  fal4,
        str1,  str2,  str3,  str4,
        str5,  str6,  str7,
        sur1,  sur2,  sur3,
        sur4,  sur5,  sur6,
        utf1,  utf2,  utf3, 
        utf4,  utf5,  utf6,
        utf7,  utf8,  utf9, 
        utf10, utf11, utf12, 
        utf13, utf14, utf15, 
        utf16, utf17, utf18,
        obj1,  obj2,  obj3,  obj4,
        obj5,  obj6,  obj7,
        arr1,  arr2,  arr3,  arr4,
        num1,  num2,  num3,  num4,
        num5,  num6,  num7,  num8,
        exp1,  exp2,  exp3
    };

    struct number
    {
        uint64_t mant;
        int bias;
        int exp;
        bool frac;
        bool neg;
    };

    // optimization: must come first
    Handler h_;

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
    const char* end_;
    parse_options opt_;

    inline static bool is_control(char c) noexcept;
    inline static char hex_digit(char c) noexcept;
    
    inline void reserve();

    BOOST_NOINLINE
    inline
    std::nullptr_t
    propagate(state st);

    BOOST_NOINLINE
    inline
    std::nullptr_t
    fail(const char* p) noexcept;

    BOOST_NOINLINE
    inline
    std::nullptr_t
    fail(
        const char* p, 
        error ev) noexcept;

    BOOST_NOINLINE
    inline
    std::nullptr_t
    maybe_suspend(
        const char* p, 
        state st);

    BOOST_NOINLINE
    inline
    std::nullptr_t
    maybe_suspend(
        const char* p,
        state st,
        const number& num);

    BOOST_NOINLINE
    inline
    std::nullptr_t
    suspend(
        const char* p,
        state st);

    BOOST_NOINLINE
    inline
    std::nullptr_t
    suspend(
        const char* p,
        state st,
        const number& num);

    BOOST_NOINLINE
    inline
    const char*
    syntax_error(const char* p);

    template<
        bool StackEmpty, bool ReturnValue,
        bool Terminal, bool AllowTrailing, 
        bool AllowBadUTF8>
    const char* parse_comment(const char* p);
    
    template<bool StackEmpty>
    const char* validate_utf8(const char* p, const char* end);

    template<bool StackEmpty>
    const char* parse_document(const char* p);
    
    template<bool StackEmpty, bool AllowComments,
        bool AllowTrailing, bool AllowBadUTF8>
    const char* parse_value(const char* p);
    
    template<bool StackEmpty, bool AllowComments,
        bool AllowTrailing, bool AllowBadUTF8>
    const char* resume_value(const char* p);
    
    template<bool StackEmpty, bool AllowComments,
        bool AllowTrailing, bool AllowBadUTF8>
    const char* parse_object(const char* p);
    
    template<bool StackEmpty, bool AllowComments,
        bool AllowTrailing, bool AllowBadUTF8>
    const char* parse_array(const char* p);
    
    template<bool StackEmpty>
    const char* parse_null(const char* p);
    
    template<bool StackEmpty>
    const char* parse_true(const char* p);
    
    template<bool StackEmpty>
    const char* parse_false(const char* p);
    
    template<bool StackEmpty, bool AllowBadUTF8>
    const char* parse_string(const char* p);
    
    template<bool StackEmpty, char First>
    const char* parse_number(const char* p);

public:
    /** Destructor.

        All dynamically allocated internal memory is freed.
    */
    ~basic_parser() = default;

    /** Constructor.
        
        The parser will only recognize strict JSON.
    */
    basic_parser() = default;

    /** Constructor.
        
        Construct a parser with the provided options.

        @param opt The options for the parser.

        @param args Optional additional arguments
        forwarded to the handler's constructor.
    */
    template<class... Args>
    basic_parser(
        parse_options const& opt,
        Args&&... args);

    /** Return a reference to the handler.
    */
    Handler&
    handler() noexcept
    {
        return h_;
    }

    /** Return a reference to the handler.
    */
    Handler const&
    handler() const noexcept
    {
        return h_;
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

    /** Reset the state, to parse a new document.
    */
    inline//BOOST_JSON_DECL
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

            // Called with the characters corresponding to the part of the current number.
            bool on_number_part( string_view s, error_code& ec );

            // Called when a signed integer is parsed.
            bool on_int64( int64_t i, string_view s, error_code& ec );

            // Called when an unsigend integer is parsed.
            bool on_uint64( uint64_t u, string_view s, error_code& ec );

            // Called when a double is parsed.
            bool on_double( double d, string_view s, error_code& ec );

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
    std::size_t
    write_some(
        bool more,
        char const* data,
        std::size_t size,
        error_code& ec);
};

} // json
} // boost

#endif
