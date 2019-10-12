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

namespace detail {

template<class T>
using is_viewy = typename std::enable_if<
    std::is_convertible<
        T const&, string_view>::value &&
    ! std::is_convertible<
        T const&, char const&>::value
            >::type;

} // detail

/** The native type of string values
*/
class BOOST_SYMBOL_VISIBLE string
{
public:
    using traits_type       = std::char_traits<char>;
    using value_type        = char;
    using size_type         = string_view::size_type;
    using difference_type   = string_view::difference_type;
    using pointer           = char*;
    using reference         = char&;
    using iterator          = char*;
    using const_pointer     = char const*;
    using const_reference   = const char&;
    using const_iterator    = char const*;

    using reverse_iterator =
        std::reverse_iterator<iterator>;

    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    /// A special index
    static constexpr size_type npos =
        string_view::npos;

    template<class T>
    using is_inputit = typename std::enable_if<
        std::is_convertible<typename
            std::iterator_traits<T>::value_type,
            char>::value>::type;

private:
    using impl_size_type = unsigned long;

    static constexpr
        size_type max_size_ = 0x7ffffffe; // 2GB

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

        BOOST_JSON_DECL
        static
        impl_size_type
        growth(
            size_type new_size,
            impl_size_type capacity);

        BOOST_JSON_DECL
        void
        destroy(
            storage_ptr const& sp);

        BOOST_JSON_DECL
        void
        construct() noexcept;

        BOOST_JSON_DECL
        char*
        construct(
            size_type new_size,
            storage_ptr const& sp);

        template<class InputIt>
        void
        construct(
            InputIt first,
            InputIt last,
            storage_ptr const& sp,
            std::forward_iterator_tag);

        template<class InputIt>
        void
        construct(
            InputIt first,
            InputIt last,
            storage_ptr const& sp,
            std::input_iterator_tag);

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
    };

    impl s_;
    storage_ptr sp_;

public:
    BOOST_JSON_DECL
    ~string();

    //
    // Construction
    //

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
        ,class = is_inputit<InputIt>
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
        string_view s,
        storage_ptr sp =
            default_storage());

    BOOST_JSON_DECL
    string(
        string_view s,
        size_type pos,
        size_type n,
        storage_ptr sp =
            default_storage());

    //--------------------------------------------------------------------------
    //
    // Assignment
    //
    //--------------------------------------------------------------------------

    string&
    operator=(string const& other)
    {
        return assign(other);
    }

    string&
    operator=(string&& other)
    {
        return assign(std::move(other));
    }

    string&
    operator=(char const* s)
    {
        return assign(s);
    }

    BOOST_JSON_DECL
    string&
    operator=(char ch);

    string&
    operator=(std::initializer_list<char> init)
    {
        return assign(init);
    }

    string&
    operator=(string_view s)
    {
        return assign(s);
    }

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

    string&
    assign(
        string const& other,
        size_type pos,
        size_type count)
    {
        return assign(
            other.substr(pos, count));
    }

    BOOST_JSON_DECL
    string&
    assign(string&& other);

    BOOST_JSON_DECL
    string&
    assign(
        char const* s,
        size_type count);

    string&
    assign(
        char const* s)
    {
        return assign(s,
            traits_type::length(s));
    }

    template<class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    string&
    assign(
        InputIt first,
        InputIt last);

    string&
    assign(std::initializer_list<char> init)
    {
        return assign(init.begin(), init.size());
    }

    string&
    assign(string_view s)
    {
        return assign(s.data(), s.size());
    }

    string&
    assign(
        string_view s,
        size_type pos,
        size_type count = npos)
    {
        return assign(s.substr(pos, count));
    }

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

    char&
    operator[](size_type pos)
    {
        return s_.data()[pos];
    }

    const char&
    operator[](size_type pos) const
    {
        return s_.data()[pos];
    }

    char&
    front()
    {
        return s_.data()[0];
    }

    char const&
    front() const
    {
        return s_.data()[0];
    }

    char&
    back()
    {
        return s_.data()[s_.size - 1];
    }

    char const&
    back() const
    {
        return s_.data()[s_.size - 1];
    }

    char*
    data() noexcept
    {
        return s_.data();
    }

    char const*
    data() const noexcept
    {
        return s_.data();
    }

    char const*
    c_str() const noexcept
    {
        return s_.data();
    }

    operator string_view() const noexcept
    {
        return {data(), size()};
    }

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
    iterator
    begin() noexcept
    {
        return s_.data();
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
        return s_.data();
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
        return s_.data();
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
        return s_.end();
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
        return s_.end();
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
        return s_.end();
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
        return reverse_iterator(s_.end());
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
        return const_reverse_iterator(s_.end());
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
        return const_reverse_iterator(s_.end());
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

    //--------------------------------------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------------------------------------

    bool
    empty() const noexcept
    {
        return s_.size == 0;
    }

    size_type
    size() const noexcept
    {
        return s_.size;
    }

    size_type
    length() const noexcept
    {
        return s_.size;
    }

    size_type
    max_size() const noexcept
    {
        return max_size_;
    }

    BOOST_JSON_DECL
    void
    reserve(size_type new_capacity);

    size_type
    capacity() const noexcept
    {
        return s_.capacity;
    }

    BOOST_JSON_DECL
    void
    shrink_to_fit();

    //--------------------------------------------------------------------------
    //
    // Operations
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    void
    clear() noexcept;

    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------

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

    BOOST_JSON_DECL
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

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    string&
    operator+=(string const& s)
    {
        return append(s);
    }

    BOOST_JSON_DECL
    string&
    operator+=(char ch)
    {
        push_back(ch);
        return *this;
    }

    BOOST_JSON_DECL
    string&
    operator+=(char const* s)
    {
        return append(s);
    }

    BOOST_JSON_DECL
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

    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------

    string_view
    substr(
        size_type pos = 0,
        size_type count = npos) const
    {
        return string_view(*this).substr(pos, count);
    }

    //--------------------------------------------------------------------------

    size_type
    copy(
        char* dest,
        size_type count,
        size_type pos = 0) const
    {
        return string_view(*this).copy(dest, count, pos);
    }

    //--------------------------------------------------------------------------

    void
    resize(size_type count)
    {
        resize(count, 0);
    }

    BOOST_JSON_DECL
    void
    resize(size_type count, char ch);

    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    void
    swap(string& other);

    //--------------------------------------------------------------------------
    //
    // Search
    //
    //--------------------------------------------------------------------------

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
        return string_view(*this).find(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

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
        return string_view(*this).rfind(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

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
        return string_view(*this).find_first_of(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

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
        return string_view(*this).find_first_not_of(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

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
        return string_view(*this).find_last_of(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

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
        return string_view(*this).find_last_not_of(string_view(t), pos);
    }

    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------

private:
    class undo;

    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    template<class InputIt>
    void
    assign(InputIt first, InputIt last,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    assign(InputIt first, InputIt last,
        std::input_iterator_tag);

    template<class InputIt>
    void
    append(InputIt first, InputIt last,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    append(InputIt first, InputIt last,
        std::input_iterator_tag);
};

//------------------------------------------------------------------------------

inline
void
swap(string& lhs, string& rhs)
{
    lhs.swap(rhs);
}

//------------------------------------------------------------------------------

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

// operator<<

BOOST_JSON_DECL
std::ostream&
operator<<(std::ostream& os, string const& s);

} // json
} // boost

#include <boost/json/impl/string.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/string.ipp>
#endif

#endif
