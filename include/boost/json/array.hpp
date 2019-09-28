//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_ARRAY_HPP
#define BOOST_JSON_ARRAY_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/allocator.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/storage.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>

namespace boost {
namespace json {

class value;

/** The native type of array values
*/
class array
{
    class table;
    friend class value;

    table* tab_ = nullptr;
    storage_ptr sp_;

    struct cleanup_assign;
    struct cleanup_insert;

public:
    using value_type = value;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value&;
    using const_reference = value const&;
    using pointer = value*;
    using const_pointer = value const*;
    using iterator = value*;
    using const_iterator = value const*;
    using reverse_iterator =
        std::reverse_iterator<iterator>;
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    //--------------------------------------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    ~array();

    BOOST_JSON_DECL
    array();

    BOOST_JSON_DECL
    explicit
    array(storage_ptr store);

    BOOST_JSON_DECL
    explicit
    array(
        size_type count);

    BOOST_JSON_DECL
    array(
        size_type count,
        storage_ptr store);

    BOOST_JSON_DECL
    array(
        size_type count,
        value_type const& v);

    BOOST_JSON_DECL
    array(
        size_type count,
        value_type const& v,
        storage_ptr store);

    template<class InputIt>
    array(
        InputIt first, InputIt last);

    template<class InputIt>
    array(
        InputIt first, InputIt last,
        storage_ptr store);

    BOOST_JSON_DECL
    array(array const& other);

    BOOST_JSON_DECL
    array(
        array const& other,
        storage_ptr store);

    BOOST_JSON_DECL
    array(array&& other) noexcept;

    BOOST_JSON_DECL
    array(pilfered<array> other) noexcept;

    BOOST_JSON_DECL
    array(
        array&& other,
        storage_ptr store);

    BOOST_JSON_DECL
    array(
        std::initializer_list<value_type> list);

    BOOST_JSON_DECL
    array(
        std::initializer_list<value_type> list,
        storage_ptr store);

    BOOST_JSON_DECL
    array&
    operator=(array&& other);

    BOOST_JSON_DECL
    array&
    operator=(array const& other);

    BOOST_JSON_DECL
    array&
    operator=(
        std::initializer_list<value_type> list);

    BOOST_JSON_DECL
    storage_ptr const&
    get_storage() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Elements
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    reference
    at(size_type pos);

    BOOST_JSON_DECL
    const_reference
    at(size_type pos) const;

    BOOST_JSON_DECL
    reference
    operator[](size_type pos);

    BOOST_JSON_DECL
    const_reference
    operator[](size_type pos) const;

    reference
    front()
    {
        return (*this)[0];
    }

    const_reference
    front() const
    {
        return (*this)[0];
    }

    reference
    back()
    {
        return (*this)[size() - 1];
    }

    const_reference
    back() const
    {
        return (*this)[size() - 1];
    }

    BOOST_JSON_DECL
    value_type*
    data() noexcept;

    BOOST_JSON_DECL
    value_type const*
    data() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Iterators
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    iterator
    begin() noexcept;

    BOOST_JSON_DECL
    const_iterator
    begin() const noexcept;

    BOOST_JSON_DECL
    const_iterator
    cbegin() noexcept
    {
        return begin();
    }

    BOOST_JSON_DECL
    iterator
    end() noexcept;

    BOOST_JSON_DECL
    const_iterator
    end() const noexcept;

    BOOST_JSON_DECL
    const_iterator
    cend() noexcept
    {
        return end();
    }

    BOOST_JSON_DECL
    reverse_iterator
    rbegin() noexcept;

    BOOST_JSON_DECL
    const_reverse_iterator
    rbegin() const noexcept;

    BOOST_JSON_DECL
    const_reverse_iterator
    crbegin() noexcept
    {
        return rbegin();
    }

    BOOST_JSON_DECL
    reverse_iterator
    rend() noexcept;

    BOOST_JSON_DECL
    const_reverse_iterator
    rend() const noexcept;

    BOOST_JSON_DECL
    const_reverse_iterator
    crend() noexcept
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

    BOOST_JSON_DECL
    size_type
    max_size() const noexcept;

    BOOST_JSON_DECL
    void
    reserve(size_type new_capacity);

    BOOST_JSON_DECL
    size_type
    capacity() const noexcept;

    BOOST_JSON_DECL
    void
    shrink_to_fit() noexcept;

    //--------------------------------------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------------------------------------

    BOOST_JSON_DECL
    void
    clear() noexcept;

    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator before,
        value_type const& v);

    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator before,
        value_type&& v);

    BOOST_JSON_DECL
    iterator
    insert (
        const_iterator before,
        size_type count,
        value_type const& v);

    template<class InputIt>
    iterator
    insert(
        const_iterator before,
        InputIt first, InputIt last);

    BOOST_JSON_DECL
    iterator
    insert(
        const_iterator before,
        std::initializer_list<value_type> list);

    template<class Arg>
    iterator
    emplace(
        const_iterator before,
        Arg&& arg);

    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos);

    BOOST_JSON_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    BOOST_JSON_DECL
    void
    push_back(value_type const& v);

    BOOST_JSON_DECL
    void
    push_back(value_type&& v);

    template<class Arg>
    reference
    emplace_back(Arg&& arg);

    BOOST_JSON_DECL
    void
    pop_back();

    BOOST_JSON_DECL
    void
    resize(size_type count);

    BOOST_JSON_DECL
    void
    resize(
        size_type count,
        value_type const& v);

    BOOST_JSON_DECL
    void
    swap(array& other) noexcept;

private:
    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    template<class InputIt>
    array(
        InputIt first, InputIt last,
        storage_ptr store,
        std::input_iterator_tag);

    template<class InputIt>
    array(
        InputIt first, InputIt last,
        storage_ptr store,
        std::forward_iterator_tag);

    template<class InputIt>
    iterator
    insert(
        const_iterator before,
        InputIt first, InputIt last,
        std::input_iterator_tag);

    template<class InputIt>
    iterator
    insert(
        const_iterator before,
        InputIt first, InputIt last,
        std::forward_iterator_tag);

    template<class Arg>
    iterator
    emplace_impl(
        const_iterator before,
        Arg&& arg);

    BOOST_JSON_DECL
    storage_ptr
    release_storage() noexcept;

    BOOST_JSON_DECL
    void
    destroy(
        value* first,
        value* last);

    BOOST_JSON_DECL
    void
    move(
        value* to,
        value* from,
        size_type n) noexcept;
};

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// headers for this file are at the bottom of value.hpp

#endif
