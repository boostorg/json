//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_SERIALIZER_HPP
#define BOOST_JSON_SERIALIZER_HPP

#include <boost/json/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/static_stack.hpp>
#include <iosfwd>

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

#ifndef GENERATING_DOCUMENTATION
    // The xsl has problems with anonymous unions
    struct nobj
    {
        object const* po;
        object::const_iterator it;
    };

    struct narr
    {
        array const* pa;
        array::const_iterator it;
    };

    struct node
    {
        union
        {
            value const* pjv;
            nobj obj;
            narr arr;
        };
        state st;

        inline
        explicit
        node(state st_) noexcept;

        inline
        explicit
        node(value const& jv) noexcept;

        inline
        explicit
        node(object const& o) noexcept;

        inline
        explicit
        node(array const& a) noexcept;
    };
#endif

    detail::static_stack<node, 16> stack_;

    string_view str_;
    unsigned char nbuf_;
    char buf_[detail::max_number_chars + 1];

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
    BOOST_JSON_DECL
    bool
    is_done() const noexcept;

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
        Calls to @ref storage::allocate may throw.

        @throw std::logic_error if no value is set.

        @return The number of characters written
        to `dest`.
    */
    BOOST_JSON_DECL
    std::size_t
    read(char* dest, std::size_t size);
};

//----------------------------------------------------------

/** Return a string representing a serialized @ref value.

    This function serializes the specified value
    and returns it as a @ref string.

    @par Exception Safety

    Strong guarantee.
    Calls to @ref storage::allocate may throw.

    @param jv The value to serialize.
*/
BOOST_JSON_DECL
string
to_string(
    value const& jv);

/** Serialize a @ref value to an output stream.

    This function serializes the specified value
    into the output stream.

    @par Exception Safety

    Strong guarantee.
    Calls to @ref storage::allocate may throw.

    @param os The output stream to serialize to.

    @param jv The value to serialize.

    @return `os`.
*/
BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    value const& jv);

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
