//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_PARSER_HPP
#define BOOST_JSON_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_stack.hpp>
#include <boost/json/detail/basic_parser.hpp>
#include <type_traits>
#include <cstddef>

BOOST_JSON_NS_BEGIN

//----------------------------------------------------------

/** A DOM parser for serialized JSON.

    This class is used to incrementally parse JSON
    from character buffers into a @ref value container.

    @par Usage

    Before parsing a new JSON, the function @ref reset
    must be called, optionally passing the storage
    pointer to be used by the @ref value container into
    which the parsed results are stored. After the
    parse is started, call the @ref write function
    to provide buffers of characters of the JSON.
    When there are no more buffers, call @ref finish.
    The parse is complete when the function @ref done
    returns `true`, or when a non-successful error
    code is returned.

    @par Incremental Parsing

    The parser allows the input to be presented in
    multiple character buffers. This is useful when
    not all of the serialized JSON is present at once
    and it is desirable to process the data as it becomes
    available, such as when reading from a network socket
    or other device. The incremental interface may also
    be used to bound the amount of work performed in each
    parsing cycle.

    @par Temporary Storage

    The parser may dynamically allocate temporary
    storage as needed to accommodate the nesting level
    of the JSON being parsed. Temporary storage is
    first obtained from an optional, caller-owned
    buffer specified upon construction. When that
    is exhausted, the next allocation uses the
    @ref memory_resource passed to the constructor; if
    no such argument is specified, the default memory
    resource is used instead. Temporary storage is
    freed only when the parser is destroyed, improving
    performance when the parser is reused to parse
    multiple JSONs.
\n
    It is important to note that the @ref memory_resource
    supplied upon construction is used for temporary
    storage only, and not for allocating the elements
    which make up the parsed value. That other memory
    resource is optionally supplied in each call
    to @ref reset.

    @par Duplicate Keys

    If there are object elements with duplicate keys;
    that is, if multiple elements in an object have
    keys that compare equal, only the last equivalent
    element will be inserted.

    @par Non-Standard JSON

    The @ref parse_options structure optionally
    provided upon construction is used to customize
    some  parameters of the parser, including which
    non-standard JSON extensions should be allowed.
    A default-constructed parse options allows only
    standard JSON.

    @par Thread Safety

    Distinct instances may be accessed concurrently.
    Non-const member functions of a shared instance
    may not be called concurrently with any other
    member functions of that instance.

    @see
        @ref parse,
        @ref parse_options.
*/
class parser
{
    struct handler
    {
        value_stack st;

        template<class... Args>
        explicit
        handler(Args&&... args)
            : st(std::forward<Args>(args)...)
        {
        }

        inline bool on_document_begin(error_code& ec);
        inline bool on_document_end(error_code& ec);
        inline bool on_object_begin(error_code& ec);
        inline bool on_object_end(std::size_t n, error_code& ec);
        inline bool on_array_begin(error_code& ec);
        inline bool on_array_end(std::size_t n, error_code& ec);
        inline bool on_key_part(string_view s, std::size_t n, error_code& ec);
        inline bool on_key(string_view s, std::size_t n, error_code& ec);
        inline bool on_string_part(string_view s, std::size_t n, error_code& ec);
        inline bool on_string(string_view s, std::size_t n, error_code& ec);
        inline bool on_number_part(string_view, error_code&);
        inline bool on_int64(std::int64_t i, string_view, error_code& ec);
        inline bool on_uint64(std::uint64_t u, string_view, error_code& ec);
        inline bool on_double(double d, string_view, error_code& ec);
        inline bool on_bool(bool b, error_code& ec);
        inline bool on_null(error_code& ec);
        inline bool on_comment_part(string_view, error_code&);
        inline bool on_comment(string_view, error_code&);
    };

    basic_parser<handler> p_;

public:
    /** Destructor.

        All dynamically allocated memory, including
        any incomplete parsing results, is freed.
    */
    ~parser() = default;
   
    /** Constructor.
        
        Construct a parser with these optionally
        specified parameters:

        @li The @ref memory_resource for the
        implementation to use when it needs to
        acquire temporary memory,

        @li The @ref parse_options to use, which
        can allow non-standard JSON extensions such
        as comments or trailing commas, and

        @li A caller-owned temporary buffer to use
        before allocating with the memory
        resource specified on construction.

        @par Example

        The following code constructs a parser which
        uses the default memory resource and a local
        buffer for temporary storage, and allows
        trailing commas to appear in the JSON:

        @code

        // this buffer will be used for temporary storage
        unsigned char temp[ 4096 ];

        // default constructed parse options allow strict JSON
        parse_options opt;

        // enable the trailing commas extension
        opt.allow_trailing_commas = true;

        // construct the parser
        parser p(
            storage_ptr(),  // use the default memory resource
            opt,
            temp, sizeof(temp) );

        // to begin parsing, reset must becalled
        p.reset();

        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        for the implementation to use to acquire
        temporary storage.

        @param opt The options for the parser. If this
        parameter is omitted, a default constructed
        parse options is used, which allows only strict
        JSON and an implementation defined maximum depth.

        @param temp_buffer A pointer to valid memory
        which the implementation will use first to
        acquire temporary storage, or `nullptr` for
        the implementation to go directoy to the
        memory resource. If this parameter is left out
        the behavior is the same as if it were null.

        @param temp_size The size of the memory pointed
        to by `temp_buffer`. This parameter is ignored
        if `temp_buffer` is null.
    */
    BOOST_JSON_DECL
    parser(
        storage_ptr sp,
        parse_options const& opt,
        unsigned char* temp_buffer,
        std::size_t temp_size) noexcept;

    /** Constructor (delegating)

        @par Effects
        @code
        parser( {}, {} )
        @endcode
    */
    parser() noexcept
        : parser({}, {})
    {
    }

    /** Constructor (delegating)

        @par Effects
        @code
        parser( std::move(sp), opt )
        @endcode
    */
    BOOST_JSON_DECL
    parser(
        storage_ptr sp,
        parse_options const& opt) noexcept;

    /** Constructor (delegating)

        @par Effects
        @code
        parser( std::move(sp), {}, nullptr, 0 )
        @endcode
    */
    explicit
    parser(storage_ptr sp) noexcept
        : parser(std::move(sp), {})
    {
    }

    /** Constructor (delegating)

        @par Effects
        @code
        parser( std::move(sp), opt, &buffer[0], N )
        @endcode
    */
    template<std::size_t N>
    parser(
        storage_ptr sp,
        parse_options const& opt,
        unsigned char(&buffer)[N]) noexcept
        : parser(std::move(sp), opt,
            &buffer[0], N)
    {
    }

#if defined(__cpp_lib_byte) || defined(BOOST_JSON_DOCS)
    /** Constructor (delegating)

        @par Effects
        @code
        parser( std::move(sp), opt,
                reinterpret_cast<unsigned char*>( temp_buffer ), N )
        @endcode
    */
    parser(
        storage_ptr sp,
        parse_options const& opt,
        std::byte* temp_buffer,
        std::size_t temp_size) noexcept
        : parser(sp, opt, reinterpret_cast<
            unsigned char*>(temp_buffer),
                temp_size)
    {
    }

    /** Constructor (delegating)

        @par Effects
        @code
        parser( std::move(sp), opt, &buffer[0], N )
        @endcode
    */
    template<std::size_t N>
    parser(
        storage_ptr sp,
        parse_options const& opt,
        std::byte(&buffer)[N]) noexcept
        : parser(std::move(sp), opt,
            &buffer[0], N)
    {
    }
#endif

#ifndef BOOST_JSON_DOCS
    // Safety net for accidental buffer overflows
    template<std::size_t N>
    parser(
        storage_ptr sp,
        parse_options const& opt,
        unsigned char(&buffer)[N],
        std::size_t n) noexcept
        : parser(std::move(sp), opt,
            &buffer[0], n)
    {
        // If this goes off, check your parameters
        // closely, chances are you passed an array
        // thinking it was a pointer.
        BOOST_ASSERT(n <= N);
    }

#ifdef __cpp_lib_byte
    // Safety net for accidental buffer overflows
    template<std::size_t N>
    parser(
        storage_ptr sp,
        parse_options const& opt,
        std::byte(&buffer)[N], std::size_t n) noexcept
        : parser(std::move(sp), opt,
            &buffer[0], n)
    {
        // If this goes off, check your parameters
        // closely, chances are you passed an array
        // thinking it was a pointer.
        BOOST_ASSERT(n <= N);
    }
#endif
#endif

    /** Returns the current depth of the JSON being parsed.

        The parsing depth is the total current nesting
        level of arrays and objects.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    std::size_t
    depth() const noexcept
    {
        return p_.depth();
    }

    /** Start parsing JSON incrementally.

        This function must be called once before parsing
        each new JSON; that is, before any calls to
        @ref write or @ref finish. Any previous partial
        results are destroyed.

        @par Complexity

        Constant or linear in the size of any previous
        partial parsing results.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use. The parser will acquire shared ownership.
    */
    BOOST_JSON_DECL
    void
    reset(storage_ptr sp = {}) noexcept;

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
        @par Exception Safety

        No-throw guarantee.
    */
    bool
    done() const noexcept
    {
        return p_.done();
    }

    /** Parse JSON incrementally.

        This function parses the JSON in the specified
        buffer. The parse proceeds from the current
        state, which is at the beginning of a new JSON
        or in the middle of the current JSON if any
        characters were already parsed.
    \n
        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have
        been parsed, or

        @li A complete JSON is parsed, including any
        optional trailing whitespace in the buffer, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        is be indicated by calling @ref finish().

        @par Complexity

        Linear in `size`.

        @return The number of characters consumed from
        the buffer, which may be less than the size
        provided.

        @param data A pointer to a buffer of `size`
        characters to parse.

        @param size The number of characters pointed to
        by `data`.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    std::size_t
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
    \n
        The characters in the buffer are processed
        starting from the beginning, until one of the
        following conditions is met:

        @li All of the characters in the buffer have
        been parsed, or

        @li A complete JSON is parsed, including any
        optional trailing whitespace in the buffer, or

        @li A parsing error occurs.

        The supplied buffer does not need to contain the
        entire JSON. Subsequent calls can provide more
        serialized data, allowing JSON to be processed
        incrementally. The end of the serialized JSON
        is be indicated by calling @ref finish().

        @par Complexity
        Linear in `s.size()`.

        @return The number of characters consumed from
        the input, which may be less than the size
        provided.

        @param s The characters to parse.

        @param ec Set to the error, if any occurred.
    */
    std::size_t
    write(
        string_view s,
        error_code& ec)
    {
        return write(
            s.data(), s.size(), ec);
    }

    /** Parse JSON incrementally.

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

    /** Return the parsed JSON as a @ref value.

        This returns the parsed value.
        is necessary to call @ref reset after calling
        this function in order to parse another JSON.
        Undefined behavior results if the parser is
        not done, or if an error occurred during
        parsing.

        @par Preconditions
        @code
        this->done() == true
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee. If an exception occurs,
        the valid operations are @ref reset or
        destruction.

        @return The parsed value. Ownership of this
        value is transferred to the caller.       
    */
    BOOST_JSON_DECL
    value
    release();
};

BOOST_JSON_NS_END

#endif
