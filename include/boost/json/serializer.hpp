//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_SERIALIZER_HPP
#define BOOST_JSON_SERIALIZER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/stack.hpp>
#include <boost/json/detail/stream.hpp>

BOOST_JSON_NS_BEGIN

/** A serializer for JSON.

    This class traverses a @ref value and emits
    a JSON text by filling a series of one or more
    caller-provided buffers. To use it, declare
    a serializer and construct it with a reference
    to the value to serialize. Alternatively, call
    @ref reset to set the value to serialize.
    Then call @ref read over and over while
    @ref done returns `false`.

    @par Example

    This demonstrates how the serializer may
    be used to print a JSON value to an output
    stream.

    @code

    void print( std::ostream& os, value const& jv)
    {
        serializer sr( jv );
        while( ! sr.done() )
        {
            char buf[4000];
            os << sr.read( buf );
        }
    }

    @endcode

    @par Thread Safety

    The same instance may not be accessed concurrently.
*/
class serializer
{
    enum class state : char;
    // VFALCO Too many streams
    using stream = detail::stream;
    using const_stream = detail::const_stream;
    using local_stream = detail::local_stream;
    using local_const_stream =
        detail::local_const_stream;

    value const* jv_ = nullptr;
    detail::stack st_;
    const_stream cs0_;
    char buf_[detail::max_number_chars + 1];
    bool done_ = false;

    inline bool suspend(state st);
    inline bool suspend(state st,
        array::const_iterator it, value const* jv);
    inline bool suspend(state st,
        object::const_iterator it, value const* jv);
    template<bool StackEmpty> bool write_null(stream& ss);
    template<bool StackEmpty> bool write_true(stream& ss);
    template<bool StackEmpty> bool write_false(stream& ss);
    template<bool StackEmpty> bool write_string(stream& ss);
    template<bool StackEmpty> bool write_number(stream& ss);
    template<bool StackEmpty> bool write_array(stream& ss);
    template<bool StackEmpty> bool write_object(stream& ss);
    template<bool StackEmpty> bool write_value(stream& ss);
    inline string_view read_some(char* dest, std::size_t size);

public:
    /** Default constructor

        This constructs a serializer with no value.
        The value may be set later by calling @ref reset.
        If serialization is attempted with no value,
        the output is as if a null value is serialized.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    serializer() noexcept;

    /** Constructor

        This constructs the serializer and prepares
        it to serialize the @ref value `jv` as if
        @ref reset was called.

        @par Exception Safety
        No-throw guarantee.

        @param jv The value to serialize. Ownership
        is not transferred. The caller is responsible
        for ensuring that the lifetime of the value
        extends until @ref done returns `true`,
        @ref reset is called with a new value, or
        the serializer is destroyed.
    */
    explicit
    serializer(value const& jv) noexcept
    {
        reset(jv);
    }

    /** Returns `true` if the serialization is complete

        This function returns `true` when all of the
        characters in the serialized representation of
        the value have been read.
    */
    bool
    done() const noexcept
    {
        return done_;
    }

    /** Reset the serializer for a new JSON value

        This function prepares the serializer to emit
        a new serialized JSON based on the specified
        value. Any internally allocated memory is
        preserved and re-used for the new output.

        @param jv The value to serialize. Ownership
        is not transferred. The caller is responsible
        for ensuring that the lifetime of the value
        extends until @ref done returns `true`,
        @ref reset is called with a new value, or
        the serializer is destroyed.
    */
    BOOST_JSON_DECL
    void
    reset(value const& jv) noexcept;

    /** Read the next buffer of serialized JSON

        This function attempts to fill the caller
        provided buffer starting at `dest` with
        up to `size` characters of the serialized
        JSON that represents the value. If the
        buffer is not large enough, multiple calls
        may be required.
\n
        If serialization completes during this call;
        that is, that all of the characters belonging
        to the serialized value have been written to
        caller-provided buffers, the function
        @ref done will return `true`.

        @par Preconditions
        `this->done() == true`

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A @ref string_view containing the
        characters written, which may be less than
        `size`.

        @param dest A pointer to valid memory of at
        least `size` bytes.

        @param size The maximum number of characters
        to write to the memory pointed to by `dest`.
    */
    BOOST_JSON_DECL
    string_view
    read(char* dest, std::size_t size);

    /** Read the next buffer of serialized JSON

        This function allows reading into a
        character array, with a deduced maximum size.

        @par Preconditions
        `this->done() == true`

        @par Effects
        @code
        return this->read( dest, N );
        @endcode

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A @ref string_view containing the
        characters written, which may be less than
        `size`.

        @param dest The character array to write to.
    */
    template<std::size_t N>
    string_view
    read(char(&dest)[N])
    {
        return read(dest, N);
    }

#ifndef BOOST_JSON_DOCS
    // Safety net for accidental buffer overflows
    template<std::size_t N>
    string_view
    read(char(&dest)[N], std::size_t n)
    {
        // If this goes off, check your parameters
        // closely, chances are you passed an array
        // thinking it was a pointer.
        BOOST_ASSERT(n <= N);
        return read(dest, n);
    }
#endif
};

BOOST_JSON_NS_END

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
