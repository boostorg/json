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

namespace boost {
namespace json {

//----------------------------------------------------------

/** A factory for building a value.

    A value builder implements an algorithm for
    efficiently constructing a @ref value from an
    external source (provided by the caller).
    It uses a dynamically allocated internal storage
    to hold portions of the document, allowing complete
    objects and arrays to be constructed using a single
    allocation when their contents are eventually known.
    This internal storage is reused when creating multiple
    values with the same builder. \n

    To use the builder construct it with an optionally
    specified memory resource to use for the internal
    storage. Then call @ref reset once before building
    each complete DOM, optionally specifying the
    memory resource to use for the resulting @ref value.
    Once the reset function is called, the value may
    be built iteratively by calling the appropriate
    insertion functions as desired. After construction
    is finished, the caller can take ownership of the
    resulting value by calling @ref release.

    @par Example

    The following code constructs a @ref value which
    when serialized produces a JSON object with three
    elements.

    @code
    value_builder vb;
    vb.reset();
    vb.begin_object();
    vb.insert_key("a");
    vb.insert_int64(1);
    vb.insert_key("b");
    vb.insert_null();
    vb.insert_key("c");
    vb.insert_string("hello");
    vb.end_object();
    assert( to_string(vb.release()) == "{\"a\":1,\"b\":null,\"c\":\"hello\"}" );
    @endcode
*/
class value_builder
{
    enum class state : char;

    struct level
    {
        std::uint32_t count;
        char align;
        state st;
    };

    storage_ptr sp_;
    detail::raw_stack rs_;
    std::uint32_t key_size_ = 0;
    std::uint32_t str_size_ = 0;
    level lev_;

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
    */
    BOOST_JSON_DECL
    explicit 
    value_builder(storage_ptr sp = {}) noexcept;

    /** Reserve internal storage space.

        This function reserves space for `n` bytes
        in the builders's internal temporary storage.
        The request is only a hint to the
        implementation.

        @par Exception Safety

        Strong guarantee.

        @param n The number of bytes to reserve.
    */
    BOOST_JSON_DECL
    void
    reserve(std::size_t n);

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

    /** Insert an array.

        This function opens a new, empty array
        which will be inserted into the result as
        the next element of the currently open array
        or object, or as the top-level element if
        no other elements exist.
    \n
        After calling this function, elements
        are inserted into the array by calling
        the other insertion functions (including
        @ref begin_array and @ref begin_object).
    \n
        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    BOOST_JSON_DECL
    void
    begin_array();

    /** Insert an array.

        This function closes the current array,
        which must have been opened by a previously
        balanced call to @ref begin_array.
        The array is then inserted into the currently
        open array or object, or the top level if no
        enclosing array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    BOOST_JSON_DECL
    void
    end_array();

    /** Insert an object.

        This function opens a new, empty object
        which will be inserted into the result as
        the next element of the currently open array
        or object, or as the top-level element if
        no other elements exist.
    \n

        After calling this function, elements are
        inserted into the object by first inserting
        the key using @ref insert_key and
        @ref insert_key_part, and then calling
        the other insertion functions (including
        @ref begin_object and @ref begin_array) to
        add the value corresponding to the key.
    \n
        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    BOOST_JSON_DECL
    void
    begin_object();

    /** Insert an object.

        This function closes the current object,
        which must have been opened by a previously
        balanced call to @ref begin_object.
        The object is then inserted into the currently
        open array or object, or the top level if no
        enclosing array or object is open.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.
    */
    BOOST_JSON_DECL
    void
    end_object();

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

private:
    inline
    void
    destroy() noexcept;

    template<class T>
    void
    push(T const& t);

    inline
    void
    push_chars(string_view s);

    template<class... Args>
    void
    emplace_object(
        Args&&... args);

    template<class... Args>
    void
    emplace_array(
        Args&&... args);

    template<class... Args>
    void
    emplace(
        Args&&... args);

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
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/value_builder.ipp>
#endif

#endif
