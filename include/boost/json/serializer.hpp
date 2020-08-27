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

namespace boost {
namespace json {

/** A serializer for JSON.

    This class traverses a @ref value and emits
    a serialized JSON object. It can work
    incrementally, by filling successive
    caller-provided buffers.
*/
class serializer
{
    enum class state : char;
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
    inline std::size_t write_some(
        char* dest, std::size_t size);

public:
    /** Default constructor.

        This constructs a serializer without
        a current @ref value to serialize. Before
        any serialization can take place, @ref reset
        must be called.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    serializer() noexcept;

    /** Constructor.

        This constructs the serializer and
        calls @ref reset with `jv`.

        @par Exception Safety

        No-throw guarantee.

        @param jv The value to serialize.
    */
    explicit
    serializer(value const& jv) noexcept
    {
        reset(jv);
    }

    /** Returns `true` if the serialization is complete.

        This function returns `true` when all of the
        characters in the serialized representation of
        the value have been read.
    */
    bool
    is_done() const noexcept
    {
        return done_;
    }

    /** Reset the serializer for a new JSON value.

        This function prepares the serializer to emit
        a new serialized JSON based on the specified
        value. Any internally allocated memory is
        preserved and re-used for the new output.

        @param jv The value to serialize.
    */
    BOOST_JSON_DECL
    void
    reset(value const& jv) noexcept;

    /** Read the next buffer of serialized JSON.

        This function attempts to fill the caller
        provided buffer starting at `dest` with
        up to `size` characters of the serialized
        JSON that represents the value. If the
        buffer is not large enough, multiple calls
        may be required.

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
                char buf[1024];
                auto const n = sr.read( buf, sizeof(buf) );
                os << string_view( buf, n );
            }
        }

        @endcode

        @par Exception Safety

        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @throw std::logic_error if no value is set.

        @return The number of characters written
        to `dest`.
    */
    BOOST_JSON_DECL
    std::size_t
    read(char* dest, std::size_t size);
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
