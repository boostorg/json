//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_OBJECT_HPP
#define BOOST_JSON_OBJECT_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/json/storage.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace boost {
namespace beast {
namespace json {

class value;

//------------------------------------------------------------------------------

/** The native type of object values

    This copies the interface of `std::unordered_map` with
    one important distinction: the order of insertions is
    preserved.
*/
class object
{
    struct list_hook;
    struct element;
    class table;

    storage_ptr sp_;
    table* tab_ = nullptr;
    float mf_ = 1.0;

public:
    using key_type = beast::string_view;
    using mapped_type = value;
    using value_type = std::pair<key_type, value>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<key_type, value&>;
    using const_reference = std::pair<key_type, value const&>;

    class hasher;
    class key_equal;
    class pointer;
    class const_pointer;
    class iterator;
    class const_iterator;
    class local_iterator;
    class const_local_iterator;
    class node_type;
    struct insert_return_type;

    //--------------------------------------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    ~object();

    BOOST_BEAST_DECL
    object();

    BOOST_BEAST_DECL
    object(
        size_type capacity);

    BOOST_BEAST_DECL
    object(
        storage_ptr store);

    BOOST_BEAST_DECL
    object(
        size_type capacity,
        storage_ptr store);

    template<class InputIt>
    BOOST_BEAST_DECL
    object(
        InputIt first,
        InputIt last);

    template<class InputIt>
    BOOST_BEAST_DECL
    object(
        InputIt first,
        InputIt last,
        size_type capacity);

    template<class InputIt>
    BOOST_BEAST_DECL
    object(
        InputIt first,
        InputIt last,
        storage_ptr store);

    template<class InputIt>
    BOOST_BEAST_DECL
    object(
        InputIt first,
        InputIt last,
        size_type capacity,
        storage_ptr store);

    BOOST_BEAST_DECL
    object(object&& other) noexcept;

    BOOST_BEAST_DECL
    object(
        object&& other,
        storage_ptr store) noexcept;

    BOOST_BEAST_DECL
    object(
        object const& other);

    BOOST_BEAST_DECL
    object(
        object const& other,
        storage_ptr store);

    BOOST_BEAST_DECL
    object(
        std::initializer_list<value_type> list);

    BOOST_BEAST_DECL
    object(
        std::initializer_list<value_type> list,
        size_type capacity);

    BOOST_BEAST_DECL
    object(
        std::initializer_list<value_type> list,
        storage_ptr store);
        
    BOOST_BEAST_DECL
    object(
        std::initializer_list<value_type> list,
        size_type capacity,
        storage_ptr store);

    BOOST_BEAST_DECL
    object&
    operator=(object&& other);

    BOOST_BEAST_DECL
    object&
    operator=(object const& other);

    BOOST_BEAST_DECL
    object&
    operator=(
        std::initializer_list<value_type> list);

    BOOST_BEAST_DECL
    storage_ptr const&
    get_storage() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Iterators
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    iterator
    begin() noexcept;

    BOOST_BEAST_DECL
    const_iterator
    begin() const noexcept;

    BOOST_BEAST_DECL
    const_iterator
    cbegin() const noexcept;

    BOOST_BEAST_DECL
    iterator
    end() noexcept;

    BOOST_BEAST_DECL
    const_iterator
    end() const noexcept;

    BOOST_BEAST_DECL
    const_iterator
    cend() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    bool
    empty() const noexcept;

    BOOST_BEAST_DECL
    size_type
    size() const noexcept;

    BOOST_BEAST_DECL
    size_type
    max_size() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    void
    clear() noexcept;

    template<class P = value_type
#ifndef BOOST_BEAST_DOXYGEN
        ,class = typename std::enable_if<
            std::is_constructible<value_type,
                P&&>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(P&& p);

    template<class P = value_type
#ifndef BOOST_BEAST_DOXYGEN
        ,class = typename std::enable_if<
            std::is_constructible<value_type,
                P&&>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(
        const_iterator before,
        P&& p);

    template<class InputIt>
    void
    insert(InputIt first, InputIt last);

    BOOST_BEAST_DECL
    void
    insert(std::initializer_list<
        value_type> list);

    BOOST_BEAST_DECL
    insert_return_type
    insert(node_type&& nh);

    BOOST_BEAST_DECL
    insert_return_type
    insert(
        const_iterator before,
        node_type&& nh);

    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        key_type key, M&& obj);

    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        const_iterator before,
        key_type key,
        M&& obj);

    template<class Arg>
    std::pair<iterator, bool>
    emplace(key_type key, Arg&& arg);

    template<class Arg>
    std::pair<iterator, bool>
    emplace(
        const_iterator before,
        key_type key, Arg&& arg);

    BOOST_BEAST_DECL
    iterator
    erase(const_iterator pos);
    
    BOOST_BEAST_DECL
    iterator
    erase(
        const_iterator first,
        const_iterator last);

    BOOST_BEAST_DECL
    size_type
    erase(key_type key);

    BOOST_BEAST_DECL
    void
    swap(object& other) noexcept;

    BOOST_BEAST_DECL
    node_type
    extract(const_iterator pos);

    BOOST_BEAST_DECL
    node_type
    extract(key_type key);

    BOOST_BEAST_DECL
    void
    merge(object& source);

    BOOST_BEAST_DECL
    void
    merge(object&& source);

    //--------------------------------------------------------------------------
    //
    // Lookup
    //
    //--------------------------------------------------------------------------
    
    BOOST_BEAST_DECL
    value&
    at(key_type key);
    
    BOOST_BEAST_DECL
    value const&
    at(key_type key) const;

    BOOST_BEAST_DECL
    value&
    operator[](key_type key);
    
    BOOST_BEAST_DECL
    value const&
    operator[](key_type key) const;

    BOOST_BEAST_DECL
    size_type
    count(key_type key) const;

    BOOST_BEAST_DECL
    size_type
    count(
        key_type key,
        std::size_t hash) const;

    BOOST_BEAST_DECL
    iterator
    find(key_type key);

    BOOST_BEAST_DECL
    iterator
    find(
        key_type key,
        std::size_t hash);

    BOOST_BEAST_DECL
    const_iterator
    find(key_type key) const;

    BOOST_BEAST_DECL
    const_iterator
    find(
        key_type key,
        std::size_t hash) const;

    BOOST_BEAST_DECL
    bool
    contains(key_type key) const;

    BOOST_BEAST_DECL
    bool
    contains(
        key_type key,
        std::size_t hash ) const;

    //--------------------------------------------------------------------------
    //
    // Bucket Interface
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    local_iterator
    begin(size_type n) noexcept;

    BOOST_BEAST_DECL
    const_local_iterator
    begin(size_type n) const noexcept;

    BOOST_BEAST_DECL
    const_local_iterator
    cbegin(size_type n) noexcept;

    BOOST_BEAST_DECL
    local_iterator
    end(size_type n)  noexcept;

    BOOST_BEAST_DECL
    const_local_iterator
    end(size_type n) const noexcept;

    BOOST_BEAST_DECL
    const_local_iterator
    cend(size_type n) noexcept;

    BOOST_BEAST_DECL
    size_type
    bucket_count() const noexcept;

    BOOST_BEAST_DECL
    size_type
    max_bucket_count() const noexcept;

    BOOST_BEAST_DECL
    size_type
    bucket_size(size_type n) const noexcept;

    BOOST_BEAST_DECL
    size_type
    bucket(key_type key) const noexcept;

    //--------------------------------------------------------------------------
    //
    // Hash Policy
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    float
    load_factor() const noexcept;

    BOOST_BEAST_DECL
    float
    max_load_factor() const;

    BOOST_BEAST_DECL
    void
    max_load_factor(float ml);

    BOOST_BEAST_DECL
    void
    rehash(size_type count);

    BOOST_BEAST_DECL
    void
    reserve(size_type count);

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    BOOST_BEAST_DECL
    hasher
    hash_function() const;

    BOOST_BEAST_DECL
    key_equal
    key_eq() const;

private:
    struct cleanup_replace;

    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    template<class InputIt>
    void
    construct(
        InputIt first,
        InputIt last,
        size_type capacity,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    construct(
        InputIt first,
        InputIt last,
        size_type capacity,
        std::input_iterator_tag);

    template<class InputIt>
    void
    insert(
        InputIt first,
        InputIt last,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    insert(
        InputIt first,
        InputIt last,
        std::input_iterator_tag);

    template<class Arg>
    std::pair<iterator, bool>
    emplace_impl(
        const_iterator before,
        key_type key,
        Arg&& arg);

    BOOST_BEAST_DECL
    static
    size_type
    constrain_hash(
        std::size_t hash,
        size_type bucket_count) noexcept;

    BOOST_BEAST_DECL
    element*
    find_element(
        key_type key,
        std::size_t hash) const noexcept;

    BOOST_BEAST_DECL
    element*
    prepare_insert(
        const_iterator* before,
        key_type key,
        std::size_t hash);

    BOOST_BEAST_DECL
    void
    finish_insert(
        const_iterator before,
        element* e,
        std::size_t hash);

    BOOST_BEAST_DECL
    void
    remove(element* e);
};

} // json
} // beast
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// headers for this file are at the bottom of value.hpp

#endif
