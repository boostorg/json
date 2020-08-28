//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_VALUE_BUILDER_HPP
#define BOOST_JSON_VALUE_BUILDER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/raw_stack.hpp>
#include <stddef.h>

#include <string> // VFALCO REMOVE

namespace boost {
namespace json {

//----------------------------------------------------------

/** A factory for building a value.

    A value builder provides an algorithm allowing
    iterative construction of a @ref value. The
    implementation uses temporary internal storage
    to buffer elements so that arrays, objects, and
    strings in the document are constructed using a
    single memory allocation. This improves performance
    and makes efficient use of the @ref memory_resource
    used to create the resulting @ref value.

    Temporary storage used by the implementation
    initially comes from an optional memory buffer
    owned by the caller. If that storage is exhausted,
    then memory is obtained dynamically from the
    @ref memory_resource provided on construction.

    @par Usage

    Construct the builder with an optional initial
    temporary buffer, and a @ref storage_ptr to use for
    more storage when the initial buffer is exhausted.
    Then to build a @ref value, first call @ref reset
    and optionally specify the @ref memory_resource
    which will be used for the value. Then add elements
    to the value and its children by calling the
    corresponding insertion functions. When all the
    elements are added, call @ref release to return
    ownership of the @ref value to the caller.

    @par Performance

    The initial buffer and any dynamically allocated
    temporary buffers are retained until the builder
    is destroyed. This improves performance when using
    a single builder instance to produce multiple
    values.

    @par Example

    The following code constructs a @ref value which
    when serialized produces a JSON object with three
    elements. It uses a local buffer for the temporary
    storage, and a separate local buffer for the storage
    of the resulting value. No memory is dynamically
    allocated; this shows how to construct a value
    without using the heap.

    @code

    // This example builds a json::value without any dynamic memory allocations:

    // Construct the builder using a local buffer
    char temp[4096];
    value_builder vb( storage_ptr(), temp, sizeof(temp) );

    // Create a monotonic resource with a local initial buffer
    char buf[4096];
    monotonic_resource mr( buf, sizeof(buf) );

    // The builder will create a value using `mr`
    vb.reset(&mr);

    // Iteratively create the elements
    vb.begin_object();
    vb.insert_key("a");
    vb.insert_int64(1);
    vb.insert_key("b");
    vb.insert_null();
    vb.insert_key("c");
    vb.insert_string("hello");
    vb.end_object();

    // Take ownership of the value
    value jv = vb.release();

    assert( to_string(jv) == "{\"a\":1,\"b\":null,\"c\":\"hello\"}" );

    // At this point we could re-use the builder by calling reset

    @endcode
*/
class value_builder
{
    class stack
    {
        enum
        {
            min_size_ = 16
        };

        storage_ptr sp_;
        void* temp_;
        value* begin_;
        value* top_;
        value* end_;
        // string starts at top_+1
        std::size_t chars_ = 0;
        bool run_dtors_ = false;

    public:
        inline ~stack();
        inline stack(
            storage_ptr sp,
            void* temp, std::size_t size) noexcept;
        inline void run_dtors(bool b) noexcept;
        inline std::size_t size() const noexcept;
        inline bool has_chars();

        inline void prepare();
        inline void clear() noexcept;
        inline void maybe_grow();
        inline void grow_one();
        inline void grow(std::size_t nchars);

        inline void append(string_view s);
        template<class... Args>
        value& push(Args&&... args);
        template<class Unchecked>
        void exchange(Unchecked&& u);
        inline string_view release_string() noexcept;
        inline value* release(std::size_t n) noexcept;
    };

    stack st_;
    storage_ptr sp_;

public:
    /** Destructor.

        All dynamically allocated memory and
        partial or complete elements is freed.
    */
    BOOST_JSON_DECL
    ~value_builder();

    /** Constructor.

        Constructs a empty builder. Before any
        @ref value can be built, the function
        @ref reset must be called. 

        The `sp` parameter is only used to allocate
        intermediate storage; it will not be used
        for the @ref value returned by @ref release.

        @param sp A pointer to the @ref memory_resource
        to use for intermediate storage allocations. If
        this argument is omitted, the default memory
        resource is used.

        @param temp_buffer A pointer to a caller-owned
        buffer which will be used to store temporary
        data used while building the value. If this
        pointer is null, the builder will use the
        storage pointer to allocate temporary data.

        @param temp_size The number of valid bytes of
        storage pointed to by `temp_buffer`.
    */
    BOOST_JSON_DECL
    value_builder(
        storage_ptr sp = {},
        void* temp_buffer = nullptr,
        std::size_t temp_size = 0) noexcept;

    /** Prepare to build a new value.

        This function must be called before building
        a new @ref value. Any previously existing partial
        or complete elements are destroyed, but internal
        dynamically allocated memory is preserved which
        may be reused to build new values.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref memory_resource
        to use for the resulting value. The builder will
        acquire shared ownership of the memory resource.
    */
    BOOST_JSON_DECL
    void
    reset(storage_ptr sp = {}) noexcept;

    /** Return the completed value.

        This function transfers ownership of the
        constructed value to the caller. The behavior
        is undefined if there is no top level element,
        or if any call to @ref begin_object or
        @ref begin_array is not balanced by a
        corresponding call to @ref end_object or
        @ref end_array.
        
        @par Exception Safety

        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A __value__ holding the result.
        Ownership of this value is transferred
        to the caller.       
    */
    BOOST_JSON_DECL
    value
    release();

    /** Discard all parsed JSON results.

        This function destroys all intermediate parsing
        results, while preserving dynamically allocated
        internal memory which may be reused on a
        subsequent parse.

        After calling this function, it is necessary
        to call @ref reset before building a new value.

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    //--------------------------------------------

    BOOST_JSON_DECL
    void
    push_array(std::size_t n);

    BOOST_JSON_DECL
    void
    push_object(std::size_t n);

    /** Set the key for the next value.

        This function appends the specified characters
        to the current key, which must be part of an
        open object. If a key is not currently being
        built or an object is not open, the behavior
        is undefined.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The characters to append. This may be empty.
    */
    BOOST_JSON_DECL
    void
    insert_key_part(
        string_view s);

    /** Set the key for the next value.

        This function appends the specified characters
        to the current key, which must be part of an
        open object. If a key is not currently being
        built or an object is not open, the behavior
        is undefined. After the characters are inserted,
        the key is finished and a value must be inserted
        next.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The characters to append. This may be empty.
    */
    BOOST_JSON_DECL
    void
    insert_key(
        string_view s);

    /** Insert a string.

        This function appends the specified characters
        to the current string, which will be created if
        it did not already exist from an immediately
        prior call to @ref insert_string_part.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The characters to append. This may be empty.
    */
    BOOST_JSON_DECL
    void
    insert_string_part(
        string_view s);

    /** Insert a string.

        This function appends the specified characters
        to the current string, which will be created if
        it did not already exist from an immediately prior
        call to @ref insert_string_part.
        The string is then inserted into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param s The characters to append. This may be empty.
    */
    BOOST_JSON_DECL
    void
    insert_string(
        string_view s);

    /** Insert a number.

        This function inserts a number into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param i The number to insert.
    */
    BOOST_JSON_DECL
    void
    insert_int64(
        int64_t i);

    /** Insert a number.

        This function inserts a number into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param u The number to insert.
    */
    BOOST_JSON_DECL
    void
    insert_uint64(
        uint64_t u);

    /** Insert a number.

        This function inserts a number into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param d The number to insert.
    */
    BOOST_JSON_DECL
    void
    insert_double(
        double d);

    /** Insert a boolean.

        This function inserts a boolean into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param b The boolean to insert.
    */
    BOOST_JSON_DECL
    void
    insert_bool(
        bool b);

    /** Build a null.

        This function inserts a null into the currently
        open array or object, or the top level if no
        array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    BOOST_JSON_DECL
    void
    insert_null();
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/value_builder.ipp>
#endif

#endif
