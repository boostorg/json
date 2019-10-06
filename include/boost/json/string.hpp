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
#include <boost/json/storage.hpp>
#include <boost/pilfer.hpp>
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>

namespace boost {
namespace json {

/** The native type of string values
*/
class string
{
    struct impl;

    impl* s_ = nullptr;
    storage_ptr sp_;

public:
    using traits_type = std::char_traits<char>;
    using value_type = char;
    using size_type = unsigned long;
    using difference_type = long;
    using pointer = char*;
    using reference = char&;
    using iterator = char*;
    using const_pointer = char const*;
    using const_reference = const char&;
    using const_iterator = char const*;
    using reverse_iterator =
        std::reverse_iterator<iterator>;
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    /// A special index
    static constexpr size_type npos =
        (std::numeric_limits<size_type>::max)();

    BOOST_JSON_DECL
    ~string();

    BOOST_JSON_DECL
    string() noexcept;

    BOOST_JSON_DECL
    explicit
    string(storage_ptr sp) noexcept;

    BOOST_JSON_DECL
    string(
        size_type count,
        char ch,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(
        string const& other,
        size_type pos,
        size_type count = npos,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(
        char const* s,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(
        char const* s,
        size_type count,
        storage_ptr sp =
            default_storage());

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                char>::value>::type
    #endif
    >
    string(
        InputIt first,
        InputIt last,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(string const& other);

    BOOST_JSON_DECL
    string(
        string const& other,
        storage_ptr sp);

    BOOST_JSON_DECL
    string(pilfered<string> other) noexcept;

    BOOST_JSON_DECL
    string(string&& other) noexcept;

    BOOST_JSON_DECL
    string(
        string&& other,
        storage_ptr sp);

    BOOST_JSON_DECL
    string(
        std::initializer_list<char> init,
        storage_ptr sp = default_storage());

    BOOST_JSON_DECL
    string(
        string_view sv,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(
        string_view sv,
        size_type pos,
        size_type n,
        storage_ptr sp =
            default_storage());

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string&
    operator=(string const& other);

    BOOST_JSON_DECL
    string&
    operator=(string&& other);

    BOOST_JSON_DECL
    string&
    operator=(char const* s);

    BOOST_JSON_DECL
    string&
    operator=(char ch);

    BOOST_JSON_DECL
    string&
    operator=(std::initializer_list<char> init);

    BOOST_JSON_DECL
    string&
    operator=(string_view sv);

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string&
    assign(
        size_type count,
        char ch);

    BOOST_JSON_DECL
    string&
    assign(
        string const& other);

    BOOST_JSON_DECL
    string&
    assign(
        string const& other,
        size_type pos,
        size_type count);

    BOOST_JSON_DECL
    string&
    assign(string&& other);

    BOOST_JSON_DECL
    string&
    assign(
        char const* s,
        size_type count);

    BOOST_JSON_DECL
    string&
    assign(
        char const* s);

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                char>::value>::type
    #endif
    >
    string&
    assign(
        InputIt first,
        InputIt last);

    BOOST_JSON_DECL
    string&
    assign(std::initializer_list<char> init);

    BOOST_JSON_DECL
    string&
    assign(string_view sv);

    BOOST_JSON_DECL
    string&
    assign(
        string_view sv,
        size_type pos,
        size_type count = npos);

    //--------------------------------------------------------------------------

    storage_ptr const&
    get_storage() const noexcept
    {
        return sp_;
    }

    //--------------------------------------------------------------------------
    //
    // Element Access
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    char&
    at(size_type pos);

    BOOST_JSON_DECL
    char const&
    at(size_type pos) const;

    BOOST_JSON_DECL
    char&
    operator[](size_type pos);

    BOOST_JSON_DECL
    const char&
    operator[](size_type pos) const;

    BOOST_JSON_DECL
    char&
    front();

    BOOST_JSON_DECL
    char const&
    front() const;

    BOOST_JSON_DECL
    char&
    back();

    BOOST_JSON_DECL
    char const&
    back() const;

    BOOST_JSON_DECL
    char*
    data() noexcept;

    BOOST_JSON_DECL
    char const*
    data() const noexcept;

    BOOST_JSON_DECL
    char const*
    c_str() const noexcept;

    BOOST_JSON_DECL
    operator string_view() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Iterators
    //
    //--------------------------------------------------------------------------

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    iterator
    begin() noexcept;

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    begin() const noexcept;

    /** Return an iterator to the first character

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    cbegin() const noexcept
    {
        return begin();
    }

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    iterator
    end() noexcept;

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    end() const noexcept;

    /** Return an iterator to the character following the last character

        The character acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_iterator
    cend() const noexcept
    {
        return end();
    }

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    reverse_iterator
    rbegin() noexcept;

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_reverse_iterator
    rbegin() const noexcept;

    /** Return a reverse iterator to the first character of the reversed container

        The pointed-to character corresponds to the last character
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_reverse_iterator
    crbegin() const noexcept
    {
        return rbegin();
    }

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    reverse_iterator
    rend() noexcept;

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_reverse_iterator
    rend() const noexcept;

    /** Return a reverse iterator to the character following the last character of the reversed container

        The pointed-to character corresponds to the character
        preceding the first character of the non-reversed container.
        This character acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    BOOST_JSON_DECL
    const_reverse_iterator
    crend() const noexcept
    {
        return rend();
    }

    //--------------------------------------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    bool
    empty() const noexcept;

    BOOST_JSON_DECL
    size_type
    size() const noexcept;

    size_type
    length() const noexcept
    {
        return size();
    }

    size_type
    max_size() const noexcept
    {
        return npos - 1;
    }

    BOOST_JSON_DECL
    void
    reserve(size_type new_capacity);

    BOOST_JSON_DECL
    size_type
    capacity() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Operations
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    void
    clear() noexcept;

    //--------------------------------------------------------------------------

    // insert

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string&
    erase(
        size_type index = 0,
        size_type count = npos);

    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos);

    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    void
    push_back(char ch);

    BOOST_JSON_DECL
    void
    pop_back();

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string&
    append(size_type count, char ch);

    BOOST_JSON_DECL
    string&
    append(string const& str );

    BOOST_JSON_DECL
    string&
    append(
        string const& str,
        size_type pos,
        size_type count = npos);

    BOOST_JSON_DECL
    string&
    append(
        char const* s,
        size_type count);

    BOOST_JSON_DECL
    string&
    append(char const* s);

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<
                    InputIt>::value_type,
                char>::value>::type
    #endif
    >
    string&
    append(InputIt first, InputIt last);

    BOOST_JSON_DECL
    string&
    append(std::initializer_list<char> init);

    BOOST_JSON_DECL
    string&
    append(string_view sv);

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string_view
    substr(
        size_type pos = 0,
        size_type count = npos) const;

private:
    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    BOOST_JSON_DECL
    void
    raw_resize(
        size_type size);

    BOOST_JSON_DECL
    char*
    raw_insert(
        size_type pos,
        size_type n);

    template<class InputIt>
    void
    maybe_raw_resize(
        InputIt, InputIt,
        std::input_iterator_tag)
    {
    }

    template<class InputIt>
    void
    maybe_raw_resize(
        InputIt first,
        InputIt last,
        std::forward_iterator_tag)
    {
        raw_resize(std::distance(
            first, last));
    }
};

inline
bool
operator==(string const& lhs, string const& rhs)
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(),
        rhs.begin(), rhs.end())
            == 0;
}

inline
bool
operator==(char const* lhs, string const& rhs)
{
    return std::lexicographical_compare(
        lhs, lhs + std::char_traits<char>::length(lhs),
        rhs.begin(), rhs.end())
            == 0;
}

inline
bool
operator==(string const& lhs, char const* rhs)
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(),
        rhs, rhs + std::char_traits<char>::length(rhs))
            == 0;
}

BOOST_JSON_DECL
std::ostream&
operator<<(std::ostream& os, string const& s);

} // json
} // boost

#include <boost/json/impl/string.hpp>
#if BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/string.ipp>
#endif

#endif
