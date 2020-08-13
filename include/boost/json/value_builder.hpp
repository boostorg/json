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

/** A factory for building a value DOM.

    A value builder implements an algorithm for
    efficiently constructing a @ref value from an
    external source (provided by the caller).

    The builder uses a dynamically allocated internal
    storage to hold portions of the document, allowing
    complete objects and arrays to be constructed using
    a single allocation when their contents are
    eventually known. This internal storage is reused
    when creating multiple values with the same builder.

    To use the builder construct it with an optionally
    specified memory resource to use for the internal
    storage. Then call @ref reset once before building
    each complete DOM, optionally specifying the
    memory resource to use for the resulting @ref value.

    The functions @ref on_document_begin and
    @ref on_document_end must be called exactly once
    at the beginning and at the end of construction.
    The remaining event handling functions are called
    according to their descriptions to build the document.
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

        All dynamically allocated memory, including
        any partially built results, is freed.
    */
    BOOST_JSON_DECL
    ~value_builder();

    /** Constructor.

        Constructs a empty builder using the default
        memory resource, or the optionally specified
        @ref storage_ptr, to allocate intermediate storage.

        @note
        Before any @ref value can be built,
        the function @ref start must be called. 

        <br>

        The `sp` parameter is only used to
        allocate intermediate storage; it will not be used
        for the @ref value returned by @ref release.

        @param sp The @ref storage_ptr to use for
        intermediate storage allocations.
    */
    BOOST_JSON_DECL
    explicit 
    value_builder(storage_ptr sp = {}) noexcept;

    /** Reserve internal storage space.

        This function reserves space for `n` bytes
        in the parser's internal temporary storage.
        The request is only a hint to the
        implementation. 

        @par Exception Safety

        Strong guarantee.

        @param n The number of bytes to reserve. A
        good choices is `C * sizeof(value)` where
        `C` is the total number of @ref value elements
        in a typical parsed JSON.
    */
    BOOST_JSON_DECL
    void
    reserve(std::size_t n);

    /** Prepare the builder for a new value.

        This function must be called before building
        a new @ref value. Any previously existing full
        or partial values are destroyed, but internal
        dynamically allocated memory is preserved which
        may be reused to build new values.

        @param sp A pointer to the @ref memory_resource
        to use for the resulting value. The builder will
        acquire shared ownership of the memory resource.
    */
    BOOST_JSON_DECL
    void
    reset(storage_ptr sp = {}) noexcept;

    /** Return the parsed JSON as a @ref value.

        If @ref is_complete() returns `true`, then the
        parsed value is returned. Otherwise an
        exception is thrown.

        @throw std::logic_error `! is_complete()`

        @return The parsed value. Ownership of this
        value is transferred to the caller.       
    */
    BOOST_JSON_DECL
    value
    release();

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

    //--------------------------------------------

    /** Begin building a new value.

        This function must be called exactly once
        after calling @ref reset, before any other
        event functions are invoked.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_document_begin(
        error_code& ec);

    /** Finish building a new value.

        This function must be called exactly once
        before calling @ref release, and after all
        event functions have been called.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_document_end(
        error_code& ec);

    /** Begin building an object.

        This instructs the builder to begin building
        a new JSON object, either as the top-level
        element of the resulting value, or as the
        next element of the current object or array
        being built.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_object_begin(
        error_code& ec);

    /** Finish building an object.

        This event function instructs the builder that
        the object currently being built, which was created
        by the last call to @ref on_object_begin, is finished.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_object_end(
        error_code& ec);

    /** Begin building an array.

        This instructs the builder to begin building
        a new JSON array, either as the top-level
        element of the resulting value, or as the
        next element of the current object or array
        being built.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_array_begin(
        error_code& ec);

    /** Finish building an array.

        This function instructs the builder that the
        array currently being built, which was created
        by the last call to @ref on_array_begin, is finished.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_array_end(
        error_code& ec);

    /** Continue creating a key.

        This function appends the specified characters
        to the key being built as the next element of
        a currently open object. If a key is not currently
        being built, the behavior is undefined.

        @return `true` on success.

        @param s The characters to append. This may be empty.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_key_part(
        string_view s,
        error_code& ec);

    /** Finish creating a key.

        This function appends the specified characters
        to the key being built as the next element of
        a currently open object, and finishes construction
        of the key. If a key is not currently being built,
        the behavior is undefined.

        @return `true` on success.

        @param s The characters to append. This may be empty.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_key(
        string_view s,
        error_code& ec);

    /** Begin or continue creating a string.

        This function appends the specified characters
        to the string being built. If a string is not
        currently being built, then a new empty string
        is started.

        @return `true` on success.

        @param s The characters to append. This may be empty.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_string_part(
        string_view s,
        error_code& ec);

    /** Create a string or finish creating a string.

        This function appends the specified characters
        to the string being built. If a string is not
        currently being built, then a new string is created
        with the specified characters.

        @return `true` on success.

        @param s The characters to append. This may be empty.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_string(
        string_view s,
        error_code& ec);

    /** Begin building a number from a string.

        This instructs the builder to begin building
        a new JSON number, either as the top-level
        element of the resulting value, or as the
        next element of the current object or array
        being built.

        @note This function has no effect and always
        returns `true`.

        @return `true` on success.

        @param s The characters to append. This may be empty.

        @param ec Set to the error, if any occurred.
    */
    bool
    on_number_part(
        string_view s,
        error_code& ec)
    {
        (void)s;
        (void)ec;
        return true;
    }

    /** Build a number.

        This function builds a number from the specified
        value and adds it as the top-level element of the
        resulting value, or as the next element of the
        current object or array being built.

        @return `true` on success.

        @param i The integer to build.

        @param s The characters to append. This value is ignored.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_int64(
        int64_t i,
        string_view s,
        error_code& ec);

    /** Build a number.

        This function builds a number from the specified
        value and adds it as the top-level element of the
        resulting value, or as the next element of the
        current object or array being built.

        @return `true` on success.

        @param i The unsigned integer to build.

        @param s The characters to append. This value is ignored.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_uint64(
        uint64_t u,
        string_view s,
        error_code& ec);

    /** Build a number.

        This function builds a number from the specified
        value and adds it as the top-level element of the
        resulting value, or as the next element of the
        current object or array being built.

        @return `true` on success.

        @param i The floating point number to build.

        @param s The characters to append. This value is ignored.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_double(
        double d,
        string_view s,
        error_code& ec);

    /** Build a boolean.

        This function builds a boolean from the specified
        value and adds it as the top-level element of the
        resulting value, or as the next element of the
        current object or array being built.

        @return `true` on success.

        @param b The boolean to build.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_bool(
        bool b,
        error_code& ec);

    /** Build a null.

        This function builds a null from the specified
        value and adds it as the top-level element of the
        resulting value, or as the next element of the
        current object or array being built.

        @return `true` on success.

        @param ec Set to the error, if any occurred.
    */
    BOOST_JSON_DECL
    bool
    on_null(error_code& ec);

    /** Specify part of comment.

        This function has no effect and always returns `true`.

        @param s The characters to append. This value is ignored.

        @param ec Set to the error, if any occurred.
    */
    bool
    on_comment_part(
        string_view s,
        error_code& ec) 
    { 
        (void)s;
        (void)ec;
        return true; 
    }
    
    /** Specify a comment.

        This function has no effect and always returns `true`.

        @param s The characters to append. This value is ignored.

        @param ec Set to the error, if any occurred.
    */
    bool
    on_comment(
        string_view s, 
        error_code& ec)
    { 
        (void)s;
        (void)ec;
        return true; 
    }

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
    bool
    emplace(
        error_code& ec,
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
