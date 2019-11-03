//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STRING_HPP
#define BOOST_JSON_STRING_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/detail/assert.hpp>
#include <boost/json/detail/string.hpp>
#include <boost/pilfer.hpp>
#include <algorithm>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** The native type of string values.

    Instances of string store and manipulate sequences
    of `char` optionally stored using the UTF-8 encoding.
    The elements of a string are stored contiguously.
    A pointer to any character in a string may be passed
    to functions that expect a pointer to the first
    element of a null-terminated `char[]` array.

    String iterators are simple pointers.

    Meets the requirements of
        <em>Container</em>,
        <em>ContiguousContainer</em>,
        <em>ReversibleContainer</em>, and
        <em>SequenceContainer</em>.
*/
class string
{
public:
    /// The traits used to perform character operations
    using traits_type       = std::char_traits<char>;

    /// The type of a character
    using value_type        = char;

    /// The type used to represent unsigned integers
    using size_type         = string_view::size_type;

    /// The type used to represent signed integers
    using difference_type   = string_view::difference_type;

    /// A pointer to an element
    using pointer           = char*;

    /// A const pointer to an element
    using const_pointer     = char const*;

    /// A reference to an element
    using reference         = char&;

    /// A const reference to an element
    using const_reference   = const char&;

    /// A random access iterator to an element
    using iterator          = char*;

    /// A random access const iterator  to an element
    using const_iterator    = char const*;

    /// A reverse random access iterator to an element
    using reverse_iterator =
        std::reverse_iterator<iterator>;

    /// A reverse random access const iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    /// A special index
    static constexpr size_type npos =
        string_view::npos;

private:
    template<class T>
    using is_inputit = typename std::enable_if<
        std::is_convertible<typename
            std::iterator_traits<T>::value_type,
            char>::value>::type;

    using impl_size_type = unsigned long;

    static constexpr
        impl_size_type mask_ = 0x0f;

    struct impl
    {
        impl_size_type size;
        impl_size_type capacity;

    #ifndef GENERATING_DOCUMENTATION
        // XSL has problems with anonymous unions
        union
        {
            char* p;
            char buf[20]; // SBO
        };
    #endif

        impl() noexcept
            : size(0)
            , capacity(sizeof(buf) - 1)
        {
            buf[0] = 0;
        }

        template<class InputIt>
        impl(
            InputIt first,
            InputIt last,
            storage_ptr const& sp,
            std::random_access_iterator_tag);

        template<class InputIt>
        impl(
            InputIt first,
            InputIt last,
            storage_ptr const& sp,
            std::input_iterator_tag);

        BOOST_JSON_DECL
        impl(
            size_type new_size,
            storage_ptr const& sp);

        BOOST_JSON_DECL
        static
        impl_size_type
        growth(
            size_type new_size,
            impl_size_type capacity);

        BOOST_JSON_DECL
        void
        destroy(
            storage_ptr const& sp) noexcept;

        BOOST_JSON_DECL
        char*
        assign(
            size_type new_size,
            storage_ptr const& sp);

        BOOST_JSON_DECL
        char*
        append(
            size_type n,
            storage_ptr const& sp);

        BOOST_JSON_DECL
        char*
        insert(
            size_type pos,
            size_type n,
            storage_ptr const& sp);

        BOOST_JSON_DECL
        void
        unalloc(storage_ptr const& sp) noexcept;

        bool
        in_sbo() const noexcept
        {
            return capacity < sizeof(buf);
        }

        void
        term(size_type n) noexcept
        {
            size = static_cast<
                impl_size_type>(n);
            data()[size] = 0;
        }

        char*
        data() noexcept
        {
            if(in_sbo())
                return buf;
            return p;
        }

        char const*
        data() const noexcept
        {
            if(in_sbo())
                return buf;
            return p;
        }

        char*
        end() noexcept
        {
            return data() + size;
        }

        char const*
        end() const noexcept
        {
            return data() + size;
        }

        bool
        contains(char const* s) const noexcept
        {
            return s >= data() && s < end();
        }

    };

    storage_ptr sp_;
    impl impl_;

public:
    /** Destructor.

        Any dynamically allocated internal storage
        is freed.

        @par Complexity

        Constant.
    */
    ~string()
    {
        if(! impl_.in_sbo())
            sp_->deallocate(impl_.p,
                impl_.capacity + 1, 1);
    }

    //
    // Construction
    //

    /** Default constructor.

        The string will have a zero size and a non-zero,
        unspecified capacity, using the default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    string() = default;

    /** Pilfer constructor.

        Constructs the string with the contents of `other`
        using pilfer semantics.
        Ownership of the @ref storage is transferred.

        @note

        After construction, the pilfered-from string may
        only be destroyed.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The string to pilfer

        @see
        
        Pilfering constructors are described in
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">Valueless Variants Considered Harmful</a>, by Peter Dimov.
    */
    string(pilfered<string> other) noexcept
        : sp_(std::move(other.get().sp_))
        , impl_(other.get().impl_)
    {
        ::new(&other.get().impl_) impl();
    }

    /** Constructor.

        The string will have a zero size and a non-zero,
        unspecified capacity, using the specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    explicit
    string(storage_ptr sp)
        : sp_(std::move(sp))
    {
    }

    /** Constructor.

        Construct the contents with `count` copies of
        character `ch`.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The size of the resulting string.

        @param ch THe value to initialize characters
        of the string with.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `count > max_size()`.
    */
    string(
        size_type count,
        char ch,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(count, ch);
    }

    /** Constructor.

        Construct the contents with a copy of the
        substring `[pos, pos+count)` of `other`. If
        the requested substring lasts past the
        end of the string, or if `count == npos`,
        the resulting substring is `[pos, other.size())`.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.

        @param pos The starting character position to
        copy from.

        @param count The number of characters to copy.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::out_of_range `pos >= other.size()`.
    */
    string(
        string const& other,
        size_type pos,
        size_type count = npos,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(other, pos, count);
    }

    /** Constructor.

        Construct the contents with those of the null
        terminated string pointed to by `s`. The length
        of the string is determined by the first null
        character.

        @par Complexity

        Linear in `strlen(s)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s A pointer to a character string used to
        copy from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `strlen(s) > max_size()`.
    */
    string(
        char const* s,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(s);
    }

    /** Constructor.

        Construct the contents with copies of the characters
        in the range `[s, s+count)`. This range can contain
        null characters.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The number of characters to copy.

        @param s A pointer to a character string used to
        copy from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `count > max_size()`.
    */
    string(
        char const* s,
        size_type count,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(s, count);
    }

    /** Constructor.

        Construct the contents with copies of characters
        in the range `[first, last)`.

        @par Constraints

        `InputIt` satisfies __InputIterator__.

        @par Complexity

        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param first An input iterator pointing to the
        first character to insert, or pointing to the
        end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `std::distance(first, last) > max_size()`.
    */
    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    string(
        InputIt first,
        InputIt last,
        storage_ptr sp = {});

    /** Copy constructor.

        Construct the contents with a copy of `other`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.
    */
    string(string const& other)
        : sp_(other.sp_)
    {
        assign(other);
    }

    /** Constructor.

        Construct the contents with a copy of `other`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.
    */
    string(
        string const& other,
        storage_ptr sp)
        : sp_(std::move(sp))
    {
        assign(other);
    }

    /** Move constructor.

        Constructs the string with the contents of `other`
        using move semantics. Ownership of the underlying
        memory is transferred.
        The container acquires shared ownership of the
        @ref storage used by `other`.

        @note

        After construction, the moved-from string behaves as
        if newly constructed with its current storage pointer.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The string to move
    */
    string(string&& other) noexcept
        : sp_(other.sp_)
        , impl_(other.impl_)
    {
        ::new(&other.impl_) impl();
    }

    /** Constructor.

        Construct the contents with those of `other`
        using move semantics.

        @li If `*other.get_storage() == *sp`,
        ownership of the underlying memory is transferred
        in constant time, with no possibility of exceptions.
        After construction, the moved-from string behaves
        as if newly constructed with its current @ref storage.

        @li if `*other.get_storage() != *sp`,
        a copy of the characters in `other` is made. In this
        case, the moved-from container is not changed.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to assign from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.
    */
    string(
        string&& other,
        storage_ptr sp)
        : sp_(std::move(sp))
    {
        assign(std::move(other));
    }

    /** Constructor.

        Construct the contents with those of the
        initializer list `init`.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to copy from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `init.size() > max_size()`.
    */
    string(
        std::initializer_list<char> init,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(init);
    }

    /** Constructor.

        Construct the contents with those of a
        string view. This view can contain
        null characters.

        @par Complexity

        Linear in `s.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to copy from.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::length_error `s.size() > max_size()`.
    */
    string(
        string_view s,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(s);
    }

    /** Constructor.

        Constructs the contents with the characters
        from the substring `[pos, pos+count)` of `s`.
        If `count == npos`, if `count` is not specified,
        or if the requested substring lasts past the end
        of the string, the resulting substring is `[pos, s.size())`.
        The substring can contain null characters.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to copy from.

        @param pos The starting character position to
        copy from.

        @param count The number of characters to copy.

        @param sp An optional pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
        If this parameter is omitted, the default storage
        is used.

        @throws std::out_of_range `pos >= s.size()`

        @throws std::length_error `count > max_size()`.
    */
    string(
        string_view s,
        size_type pos,
        size_type count,
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
        assign(s.substr(pos, count));
    }

    //------------------------------------------------------
    //
    // Assignment
    //
    //------------------------------------------------------

    /** Copy assignment.

        Replace the contents with a copy of `other`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.
    */
    string&
    operator=(string const& other)
    {
        return assign(other);
    }

    /** Move assignment.

        Replace the contents with those of `other`
        using move semantics.

        @li If `*other.get_storage() == *this->get_storage()`,
        ownership of the underlying memory is transferred
        in constant time, with no possibility of exceptions.
        After construction, the moved-from string behaves
        as if newly constructed with its current @ref storage.

        @li if `*other.get_storage() != *this->get_storage()`,
        a copy of the characters in `other` is made. In this
        case, the moved-from container is not changed.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
    */
    string&
    operator=(string&& other)
    {
        return assign(std::move(other));
    }

    /** Assign a value to the string.

        Replaces the contents with those of the null
        terminated string pointed to by `s`. The length
        of the string is determined by the first null
        character.

        @par Complexity

        Linear in `strlen(s)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s A pointer to a character string used to
        copy from.

        @throws std::length_error `strlen(s) > max_size()`.
    */
    string&
    operator=(char const* s)
    {
        return assign(s);
    }

    /** Assign a value to the string.

        Replaces the contents with those of the
        initializer list `init`.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to copy from.

        @throws std::length_error `init.size() > max_size()`.
    */
    string&
    operator=(std::initializer_list<char> init)
    {
        return assign(init);
    }

    /** Assign a value to the string.

        Replaces the contents with those of a
        string view. This view can contain
        null characters.

        @par Complexity

        Linear in `s.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to copy from.

        @throws std::length_error `s.size() > max_size()`.
    */
    string&
    operator=(string_view s)
    {
        return assign(s);
    }

    //------------------------------------------------------

    /** Assign characters to a string.

        Replace the contents with `count` copies of
        character `ch`.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The size of the resulting string.

        @param ch THe value to initialize characters
        of the string with.

        @throws std::length_error `count > max_size()`.
    */
    BOOST_JSON_DECL
    string&
    assign(
        size_type count,
        char ch);

    /** Assign characters to a string.

        Replace the contents with a copy of `other`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.
    */
    BOOST_JSON_DECL
    string&
    assign(
        string const& other);

    /** Assign characters to a string.

        Replace the contents with a copy of the
        substring `[pos, pos+count)` of `other`. If
        the requested substring lasts past the
        end of the string, or if `count == npos`,
        the resulting substring is `[pos, other.size())`.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to use as a source
        to copy from.

        @param pos The starting character position to
        copy from.

        @param count The number of characters to copy.

        @throws std::out_of_range `pos >= other.size()`.
    */
    string&
    assign(
        string const& other,
        size_type pos,
        size_type count)
    {
        return assign(
            other.substr(pos, count));
    }

    /** Assign characters to a string.

        Replace the contents with those of `other`
        using move semantics.

        @li If `*other.get_storage() == *this->get_storage()`,
        ownership of the underlying memory is transferred
        in constant time, with no possibility of exceptions.
        After construction, the moved-from string behaves
        as if newly constructed with its current @ref storage.

        @li if `*other.get_storage() != *this->get_storage()`,
        a copy of the characters in `other` is made. In this
        case, the moved-from container is not changed.

        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The string to assign from.
    */
    BOOST_JSON_DECL
    string&
    assign(string&& other);

    /** Assign characters to a string.

        Replaces the contents with copies of the characters
        in the range `[s, s+count)`. This range can contain
        null characters.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param count The number of characters to copy.

        @param s A pointer to a character string used to
        copy from.

        @throws std::length_error `count > max_size()`.
    */
    BOOST_JSON_DECL
    string&
    assign(
        char const* s,
        size_type count);

    /** Assign characters to a string.

        Replaces the contents with those of the null
        terminated string pointed to by `s`. The length
        of the string is determined by the first null
        character.

        @par Complexity

        Linear in `strlen(s)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s A pointer to a character string used to
        copy from.

        @throws std::length_error `strlen(s) > max_size()`.
    */
    string&
    assign(
        char const* s)
    {
        return assign(s,
            traits_type::length(s));
    }

    /** Assign characters to a string.

        Replaces the contents with copies of characters
        in the range `[first, last)`.

        @par Constraints

        `InputIt` satisfies __InputIterator__.

        @par Complexity

        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param first An input iterator pointing to the
        first character to insert, or pointing to the
        end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @throws std::length_error `std::distance(first, last) > max_size()`.
    */
    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    string&
    assign(
        InputIt first,
        InputIt last);

    /** Assign characters to a string.

        Replaces the contents with those of the
        initializer list `init`.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to copy from.

        @throws std::length_error `init.size() > max_size()`.
    */
    string&
    assign(std::initializer_list<char> init)
    {
        return assign(init.begin(), init.size());
    }

    /** Assign characters to a string.

        Replaces the contents with those of a
        string view. This view can contain
        null characters.

        @par Complexity

        Linear in `s.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to copy from.

        @throws std::length_error `s.size() > max_size()`.
    */
    string&
    assign(string_view s)
    {
        return assign(s.data(), s.size());
    }

    /** Assign characters to a string.

        Replaces the contents with the characters
        from the substring `[pos, pos+count)` of `s`.
        If `count == npos`, if `count` is not specified,
        or if the requested substring lasts past the end
        of the string, the resulting substring is `[pos, s.size())`.
        The substring can contain null characters.

        @par Complexity

        Linear in `count`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param s The string view to copy from.

        @param pos The starting character position to
        copy from.

        @param count The number of characters to copy.

        @throws std::out_of_range `pos >= s.size()`

        @throws std::length_error `count > max_size()`.
    */
    string&
    assign(
        string_view s,
        size_type pos,
        size_type count = npos)
    {
        return assign(s.substr(pos, count));
    }

    //------------------------------------------------------

    /** Return a pointer to the storage associated with the container

        Shared ownership of the @ref storage is propagated by
        the container to all of its children recursively.

        @par Complexity

        Constant.
    */
    storage_ptr const&
    get_storage() const noexcept
    {
        return sp_;
    }

    //------------------------------------------------------
    //
    // Element Access
    //
    //------------------------------------------------------

    /** Access a character, with bounds checking

        Returns a reference to the character specified at
        location `pos`, with bounds checking. If pos is not
        within the range of the container, an exception of
        type `std::out_of_range` is thrown.

        @par Complexity

        Constant.

        @param pos A zero-based index

        @throws std::out_of_range `pos >= size()`
    */
    char&
    at(size_type pos)
    {
        if(pos >= size())
            BOOST_JSON_THROW(
                std::out_of_range(
                    "pos >= size()"));
        return impl_.data()[pos];
    }

    /** Access a character, with bounds checking

        Returns a reference to the character specified at
        location `pos`, with bounds checking. If pos is not
        within the range of the container, an exception of
        type `std::out_of_range` is thrown.

        @par Complexity

        Constant.

        @param pos A zero-based index

        @throws std::out_of_range `pos >= size()`
    */
    char const&
    at(size_type pos) const
    {
        if(pos >= size())
            BOOST_JSON_THROW(
                std::out_of_range(
                    "pos >= size()"));
        return impl_.data()[pos];
    }

    /** Access a character

        Returns a reference to the character specified at
        location `pos`. No bounds checking is performed.

        @par Precondition
        
        `pos >= size`

        @par Complexity

        Constant.

        @param pos A zero-based index
    */
    char&
    operator[](size_type pos)
    {
        return impl_.data()[pos];
    }

    /** Access a character

        Returns a reference to the element specified at
        location `pos`. No bounds checking is performed.

        @par Precondition

        `pos >= size`

        @par Complexity

        Constant.

        @param pos A zero-based index
    */
    const char&
    operator[](size_type pos) const
    {
        return impl_.data()[pos];
    }

    /** Access the first character

        Returns a reference to the first character.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    char&
    front()
    {
        return impl_.data()[0];
    }

    /** Access the first character

        Returns a reference to the first character.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    char const&
    front() const
    {
        return impl_.data()[0];
    }

    /** Access the last character

        Returns a reference to the last character.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    char&
    back()
    {
        return impl_.data()[impl_.size - 1];
    }

    /** Access the last character

        Returns a reference to the last character.

        @par Precondition

        `not empty()`

        @par Complexity

        Constant.
    */
    char const&
    back() const
    {
        return impl_.data()[impl_.size - 1];
    }

    /** Access the underlying character array directly

        Returns a pointer to the underlying array
        serving as storage. The value returned is such that
        the range `[data(), data()+size())` is always a
        valid range, even if the container is empty.

        @par Complexity

        Constant.

        @return A pointer to the string. The string is
        always null terminated. The value returned from
        this function is never equal to `nullptr`.
    */
    char*
    data() noexcept
    {
        return impl_.data();
    }

    /** Access the underlying character array directly

        Returns a pointer to the underlying array
        serving as storage. The value returned is such that
        the range `[data(), data()+size())` is always a
        valid range, even if the container is empty.

        @par Complexity

        Constant.

        @return A pointer to the string. The string is
        always null terminated. The value returned from
        this function is never equal to `nullptr`.
    */
    char const*
    data() const noexcept
    {
        return impl_.data();
    }

    /** Access the underlying character array directly

        Returns a pointer to the underlying array
        serving as storage. The value returned is such that
        the range `[data(), data()+size())` is always a
        valid range, even if the container is empty.

        @par Complexity

        Constant.

        @return A pointer to the string. The string is
        always null terminated. The value returned from
        this function is never equal to `nullptr`.
    */
    char const*
    c_str() const noexcept
    {
        return impl_.data();
    }

    /** Return a reference to the string as a string view.

        This function returns a string view to the
        underlying character string. While the string is
        always null terminated, the size of the view does
        not include the null character.

        @par Complexity

        Constant.
    */
    operator string_view() const noexcept
    {
        return {data(), size()};
    }

    //------------------------------------------------------
    //
    // Iterators
    //
    //------------------------------------------------------

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    iterator
    begin() noexcept
    {
        return impl_.data();
    }

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    const_iterator
    begin() const noexcept
    {
        return impl_.data();
    }

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    const_iterator
    cbegin() const noexcept
    {
        return impl_.data();
    }

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    iterator
    end() noexcept
    {
        return impl_.end();
    }

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    const_iterator
    end() const noexcept
    {
        return impl_.end();
    }

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    const_iterator
    cend() const noexcept
    {
        return impl_.end();
    }

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    reverse_iterator
    rbegin() noexcept
    {
        return reverse_iterator(impl_.end());
    }

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    const_reverse_iterator
    rbegin() const noexcept
    {
        return const_reverse_iterator(impl_.end());
    }

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    const_reverse_iterator
    crbegin() const noexcept
    {
        return const_reverse_iterator(impl_.end());
    }

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    reverse_iterator
    rend() noexcept
    {
        return reverse_iterator(begin());
    }

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    const_reverse_iterator
    rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    const_reverse_iterator
    crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    //------------------------------------------------------
    //
    // Capacity
    //
    //------------------------------------------------------

    /** Check if the string has no characters.

        Returns `true` if there are no characters in
        the string, i.e. @ref size() returns 0.

        @par Complexity

        Constant.
    */
    bool
    empty() const noexcept
    {
        return impl_.size == 0;
    }

    /** Return the number of characters in the string.

        The value returned does not include the
        null terminator, which is always present.

        @par Complexity

        Constant.
    */
    size_type
    size() const noexcept
    {
        return impl_.size;
    }

    /** Return the maximum number of characters the string can hold

        The maximum is an implementation-defined number.
        This value is a theoretical limit; at runtime,
        the actual maximum size may be less due to
        resource limits.

        @par Complexity

        Constant.
    */
    static
    constexpr
    size_type
    max_size() noexcept
    {
        return detail::max_string_length_;
    }

    /** Return the number of characters that can be held without a reallocation.

        This number represents the largest number of
        characters that may be currently held without
        reallocating the existing storage.
        This number may be larger than the value returned
        by @ref size().

        @par Complexity

        Constant.
    */
    size_type
    capacity() const noexcept
    {
        return impl_.capacity;
    }

    /** Increase the capacity to at least a certain amount

        This increases the capacity of the array to a value
        that is greater than or equal to `new_capacity`. If
        `new_capacity > capacity()`, new memory is allocated.
        Otherwise, the call has no effect. The number of
        elements and therefore the @ref size() of the container
        is not changed.

        @note

        If new memory is allocated, all iterators including any
        past-the-end iterators, and all references to the
        elements are invalidated. Otherwise, no iterators or
        references are invalidated.

        @par Complexity

        At most, linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @throw std::length_error `new_capacity > max_size()`

        @param new_capacity The new capacity of the array.
    */
    void
    reserve(size_type new_capacity)
    {
        if(new_capacity <= impl_.capacity)
            return;
        reserve_impl(new_capacity);
    }

    /** Request the removal of unused capacity

        This performs a non-binding request to reduce
        @ref capacity() to @ref size(). The request may
        or may not be fulfilled. If reallocation occurs,
        all iterators including any past-the-end iterators,
        and all references to characters are invalidated.
        Otherwise, no iterators or references are
        invalidated.

        @par Complexity

        At most, linear in @ref size().

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    shrink_to_fit();

    //------------------------------------------------------
    //
    // Operations
    //
    //------------------------------------------------------

    /** Clear the contents

        Erases all characters from the string. After this
        call, @ref size() returns zero but @ref capacity()
        is unchanged. All references, pointers, or iterators
        referring to contained elements are invalidated. Any
        past-the-end iterators are also invalidated.

        @par Complexity

        Linear in @ref size().
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    //------------------------------------------------------

    BOOST_JSON_DECL
    string&
    insert(
        size_type pos,
        size_type count,
        char ch);

    string&
    insert(
        size_type pos,
        char const* s)
    {
        return insert(pos, s,
            traits_type::length(s));
    }

    BOOST_JSON_DECL
    string&
    insert(
        size_type pos,
        char const* s,
        size_type count);

    string&
    insert(
        size_type pos,
        string const& s)
    {
        return insert(pos, s.data(), s.size());
    }

    string&
    insert(
        size_type pos,
        string const& s,
        size_type pos_str,
        size_type count = npos)
    {
        return insert(pos, s.substr(pos_str, count));
    }

    iterator
    insert(
        const_iterator pos,
        char ch)
    {
        return insert(pos, 1, ch);
    }

    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator pos,
        size_type count,
        char ch);

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    iterator
    insert(
        const_iterator pos,
        InputIt first,
        InputIt last);

    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator pos,
        std::initializer_list<char> init);

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    insert(
        size_type pos,
        T const& t)
    {
        string_view s(t);
        return insert(pos, s.data(), s.size());
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    insert(
        size_type pos,
        T const& t,
        size_type pos_str,
        size_type count = npos)
    {
        return insert(pos,
            string_view(t).substr(pos_str, count));
    }

    //------------------------------------------------------

    BOOST_JSON_DECL
    string&
    erase(
        size_type pos = 0,
        size_type count = npos);

    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos);

    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    //------------------------------------------------------

    BOOST_JSON_DECL
    void
    push_back(char ch);

    BOOST_JSON_DECL
    void
    pop_back();

    //------------------------------------------------------

    BOOST_JSON_DECL
    string&
    append(
        size_type count,
        char ch);

    string&
    append(string const& s)
    {
        return append(s.data(), s.size());
    }

    string&
    append(
        string const& s,
        size_type pos,
        size_type count = npos)
    {
        return append(s.substr(pos, count));
    }

    string&
    append(char const* s)
    {
        return append(s, traits_type::length(s));
    }

    BOOST_JSON_DECL
    string&
    append(
        char const* s,
        size_type count);

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    string&
    append(InputIt first, InputIt last);

    string&
    append(std::initializer_list<char> init)
    {
        return append(init.begin(), init.size());
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    append(T const& t)
    {
        string_view s(t);
        return append(s.data(), s.size());
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    append(
        T const& t,
        size_type pos,
        size_type count = npos)
    {
        auto s = string_view(t).substr(pos, count);
        return append(s.data(), s.size());
    }

    //------------------------------------------------------

    string&
    operator+=(string const& s)
    {
        return append(s);
    }

    string&
    operator+=(char ch)
    {
        push_back(ch);
        return *this;
    }

    string&
    operator+=(char const* s)
    {
        return append(s);
    }

    string&
    operator+=(std::initializer_list<char> init)
    {
        return append(init);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    operator+=(T const& t)
    {
        return append(t);
    }

    //------------------------------------------------------

    int
    compare(string const& s) const noexcept
    {
        return string_view(*this).compare(
            string_view(s));
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        string const& s) const
    {
        return string_view(*this).compare(
            pos1, count1, string_view(s));
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        string const& s,
        size_type pos2,
        size_type count2 = npos) const
    {
        return string_view(*this).compare(
            pos1, count1, string_view(s),
            pos2, count2);
    }

    int
    compare(char const* s) const
    {
        return string_view(*this).compare(s);
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        char const* s) const
    {
        return string_view(*this).compare(
            pos1, count1, s);
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        char const* s,
        size_type count2) const
    {
        return string_view(*this).compare(
            pos1, count1, s, count2);
    }

    int
    compare(string_view s) const noexcept
    {
        return string_view(*this).compare(s);
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        string_view s) const
    {
        return string_view(*this).compare(
            pos1, count1, s);
    }

    int
    compare(
        size_type pos1,
        size_type count1,
        string_view s,
        size_type pos2,
        size_type count2 = npos) const
    {
        return string_view(*this).compare(
            pos1, count1, s, pos2, count2);
    }

    //------------------------------------------------------

    bool
    starts_with(string_view s) const noexcept
    {
        return string_view(*this).starts_with(s);
    }

    bool
    starts_with(char ch) const noexcept
    {
        return string_view(*this).starts_with(ch);
    }

    bool 
    starts_with(char const* s) const
    {
        return string_view(*this).starts_with(s);
    }

    bool
    ends_with(string_view s) const noexcept
    {
        return string_view(*this).ends_with(s);
    }

    bool
    ends_with(char ch) const noexcept
    {
        return string_view(*this).ends_with(ch);
    }

    bool 
    ends_with(char const* s) const
    {
        return string_view(*this).ends_with(s);
    }

    //------------------------------------------------------

    string&
    replace(
        size_type pos,
        size_type count,
        string const& s);

    string&
    replace(
        const_iterator first,
        const_iterator last,
        string const& s);

    string&
    replace(
        size_type pos,
        size_type count,
        string const& s,
        size_type pos2,
        size_type count2 = npos);

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    string&
    replace(
        const_iterator first,
        const_iterator last,
        InputIt first2,
        InputIt last2);

    string&
    replace(
        size_type pos,
        size_type count,
        char const* s,
        size_type count2);

    string&
    replace(
        const_iterator first,
        const_iterator last,
        char const* s,
        size_type count2);

    string&
    replace(
        size_type pos,
        size_type count,
        char const* s);

    string&
    replace(
        const_iterator first,
        const_iterator last,
        char const* s);

    string&
    replace(
        size_type pos,
        size_type count,
        size_type count2,
        char ch);

    string&
    replace(
        const_iterator first,
        const_iterator last,
        size_type count2,
        char ch);

    string&
    replace(
        const_iterator first,
        const_iterator last,
        std::initializer_list<char> init);

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    replace(
        size_type pos,
        size_type count,
        T const& t);

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    replace(
        const_iterator first,
        const_iterator last,
        T const& t);

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    string&
    replace(
        size_type pos,
        size_type count,
        T const& t,
        size_type pos2,
        size_type count2 = npos);

    //------------------------------------------------------

    string_view
    substr(
        size_type pos = 0,
        size_type count = npos) const
    {
        return string_view(*this).substr(pos, count);
    }

    //------------------------------------------------------

    size_type
    copy(
        char* dest,
        size_type count,
        size_type pos = 0) const
    {
        return string_view(*this).copy(dest, count, pos);
    }

    //------------------------------------------------------

    void
    resize(size_type count)
    {
        resize(count, 0);
    }

    BOOST_JSON_DECL
    void
    resize(size_type count, char ch);

    /** Increase size without changing capacity.

        This increases the size of the string by `n`
        characters, adjusting the position of the
        terminating null for the new size. The new
        characters remain uninitialized. This function
        may be used to append characters directly into
        the storage between `end()` and
        `data() + capacity()`.

        @par Precondition

        @code
        count <= capacity() - size()
        @endcode

        @par Exception Safety
        
        No-throw guarantee.

        @param n The amount to increase the size by.
    */
    void
    grow(size_type n) noexcept
    {
        BOOST_JSON_ASSERT(
            n <= impl_.capacity - impl_.size);
        impl_.term(impl_.size + n);
    }

    //------------------------------------------------------

    BOOST_JSON_DECL
    void
    swap(string& other);

    //------------------------------------------------------
    //
    // Search
    //
    //------------------------------------------------------

    size_type
    find(
        string const& s,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find(string_view(s), pos);
    }

    size_type
    find(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).find(s, pos, count);
    }

    size_type
    find(
        char const* s,
        size_type pos = 0) const
    {
        return string_view(*this).find(s, pos);
    }


    size_type
    find(
        char ch,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    find(
        T const &t,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find(t, pos);
    }

    //------------------------------------------------------

    size_type
    rfind(
        string const& s,
        size_type pos = npos) const noexcept
    {
        return string_view(*this).rfind(string_view(s), pos);
    }

    size_type
    rfind(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).rfind(s, pos, count);
    }

    size_type
    rfind(
        char const* s,
        size_type pos = npos) const
    {
        return string_view(*this).rfind(s, pos);
    }

    size_type
    rfind(
        char ch,
        size_type pos = npos) const noexcept
    {
        return string_view(*this).rfind(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    rfind(
        T const& t,
        size_type pos = npos) const noexcept
    {
        return string_view(*this).rfind(t, pos);
    }

    //------------------------------------------------------

    size_type
    find_first_of(
        string const& s,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_of(string_view(s), pos);
    }

    size_type
    find_first_of(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).find_first_of(s, pos, count);
    }

    size_type
    find_first_of(
        char const* s,
        size_type pos = 0) const
    {
        return string_view(*this).find_first_of(s, pos);
    }

    size_type
    find_first_of(
        char ch,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_of(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    find_first_of(
        T const& t,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_of(t, pos);
    }

    //------------------------------------------------------

    size_type
    find_first_not_of(
        string const& s,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_not_of(string_view(s), pos);
    }

    size_type
    find_first_not_of(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).find_first_not_of(s, pos, count);
    }

    size_type
    find_first_not_of(
        char const* s,
        size_type pos = 0) const
    {
        return string_view(*this).find_first_not_of(s, pos);
    }

    size_type
    find_first_not_of(
        char ch,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_not_of(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    find_first_not_of(
        T const& t,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_first_not_of(t, pos);
    }

    //------------------------------------------------------

    size_type
    find_last_of(
        string const& s,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_of(string_view(s), pos);
    }

    size_type
    find_last_of(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).find_last_of(s, pos, count);
    }

    size_type
    find_last_of(
        char const* s,
        size_type pos = 0) const
    {
        return string_view(*this).find_last_of(s, pos);
    }

    size_type
    find_last_of(
        char ch,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_of(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    find_last_of(
        T const& t,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_of(t, pos);
    }

    //------------------------------------------------------

    size_type
    find_last_not_of(
        string const& s,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_not_of(string_view(s), pos);
    }

    size_type
    find_last_not_of(
        char const* s,
        size_type pos,
        size_type count) const
    {
        return string_view(*this).find_last_not_of(s, pos, count);
    }

    size_type
    find_last_not_of(
        char const* s,
        size_type pos = 0) const
    {
        return string_view(*this).find_last_not_of(s, pos);
    }

    size_type
    find_last_not_of(
        char ch,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_not_of(ch, pos);
    }

    template<class T
    #ifndef GENERATING_DOCUMENTATION
        ,class = detail::is_viewy<T>
    #endif
    >
    size_type
    find_last_not_of(
        T const& t,
        size_type pos = 0) const noexcept
    {
        return string_view(*this).find_last_not_of(t, pos);
    }

    //------------------------------------------------------

    /** Perform stream output.

        Behaves as a formatted output function.
    */
    BOOST_JSON_DECL
    friend
    std::ostream&
    operator<<(std::ostream& os, string const& s);

private:
    class undo;

    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    template<class InputIt>
    void
    assign(InputIt first, InputIt last,
        std::random_access_iterator_tag);

    template<class InputIt>
    void
    assign(InputIt first, InputIt last,
        std::input_iterator_tag);

    template<class InputIt>
    void
    append(InputIt first, InputIt last,
        std::random_access_iterator_tag);

    template<class InputIt>
    void
    append(InputIt first, InputIt last,
        std::input_iterator_tag);

    BOOST_JSON_DECL
    void
    reserve_impl(size_type new_capacity);
};

//----------------------------------------------------------

inline
void
swap(string& lhs, string& rhs)
{
    lhs.swap(rhs);
}

//----------------------------------------------------------

// operator==

inline
bool
operator==(string const& lhs, string const& rhs)
{
    return string_view(lhs) == string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator==(string const& lhs, T const& rhs)
{
    return string_view(lhs) == string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator==(T const& lhs, string const& rhs)
{
    return string_view(lhs) == string_view(rhs);
}

// operator!=

inline
bool
operator!=(string const& lhs, string const& rhs)
{
    return string_view(lhs) != string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator!=(string const& lhs, T const& rhs)
{
    return string_view(lhs) != string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator!=(T const& lhs, string const& rhs)
{
    return string_view(lhs) != string_view(rhs);
}

// operator<

inline
bool
operator<(string const& lhs, string const& rhs)
{
    return string_view(lhs) < string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator<(string const& lhs, T const& rhs)
{
    return string_view(lhs) < string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator<(T const& lhs, string const& rhs)
{
    return string_view(lhs) < string_view(rhs);
}

// operator<=

inline
bool
operator<=(string const& lhs, string const& rhs)
{
    return string_view(lhs) <= string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator<=(string const& lhs, T const& rhs)
{
    return string_view(lhs) <= string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator<=(T const& lhs, string const& rhs)
{
    return string_view(lhs) <= string_view(rhs);
}

// operator>=

inline
bool
operator>=(string const& lhs, string const& rhs)
{
    return string_view(lhs) >= string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator>=(string const& lhs, T const& rhs)
{
    return string_view(lhs) >= string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator>=(T const& lhs, string const& rhs)
{
    return string_view(lhs) >= string_view(rhs);
}

// operator>=

inline
bool
operator>(string const& lhs, string const& rhs)
{
    return string_view(lhs) > string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator>(string const& lhs, T const& rhs)
{
    return string_view(lhs) > string_view(rhs);
}

template<class T
#ifndef GENERATING_DOCUMENTATION
    ,class = detail::is_viewy<T>
#endif
>
bool operator>(T const& lhs, string const& rhs)
{
    return string_view(lhs) > string_view(rhs);
}

} // json
} // boost

#include <boost/json/impl/string.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/string.ipp>
#endif

#endif
