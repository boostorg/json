//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_HPP
#define BOOST_JSON_IMPL_OBJECT_HPP

#include <boost/json/value.hpp>
#include <boost/core/exchange.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

struct object::list_hook
{
    element* prev;
    element* next;
};

//----------------------------------------------------------

struct object::element : list_hook
{
    value v;
    element* local_next;
    impl_size_type size; // of key (excluding null)

    string_view
    key() const noexcept
    {
        return {reinterpret_cast<
            char const*>(this+1),
                size};
    }

    BOOST_JSON_DECL
    void
    destroy(
        storage_ptr const& sp) const noexcept;

    template<class Arg>
    element(
        Arg&& arg,
        storage_ptr sp)
        : v(std::forward<Arg>(arg),
            std::move(sp))
    {
    }
};

//----------------------------------------------------------

struct object::table
{
    // number of values in the object
    std::size_t size;

    // number of buckets in table
    std::size_t bucket_count;

    // insertion-order list of all objects
    element* head;

    list_hook end_element;

    inline
    void
    destroy(
        storage_ptr const& sp) noexcept;

    inline
    table(size_type bucket_count_) noexcept;

    static
    inline
    table*
    construct(
        size_type bucket_count,
        storage_ptr const& sp);

    element*
    begin() noexcept
    {
        return head;
    }

    element*
    end() noexcept
    {
        return reinterpret_cast<
            element*>(&end_element);
    }

    element*&
    bucket(std::size_t n) noexcept
    {
        return reinterpret_cast<
            element**>(this + 1)[n];
    }
};

//----------------------------------------------------------

class object::undo_range
{
    object& self_;
    element* head_ = nullptr;
    element* tail_ = nullptr;
    size_type n_ = 0;

public:
    BOOST_JSON_DECL
    explicit
    undo_range(object& self) noexcept;

    BOOST_JSON_DECL
    ~undo_range();

    BOOST_JSON_DECL
    void
    insert(element* e) noexcept;

    BOOST_JSON_DECL
    void
    commit(
        const_iterator pos,
        size_type count);
};

//----------------------------------------------------------

class object::hasher
{
    inline
    static
    std::pair<
        std::uint64_t, std::uint64_t>
    init(std::true_type) noexcept;

    inline
    static
    std::pair<
        std::uint32_t, std::uint32_t>
    init(std::false_type) noexcept;

public:
    BOOST_JSON_DECL
    std::size_t
    operator()(key_type key) const noexcept;
};

//----------------------------------------------------------

class object::key_equal
{
public:
    bool
    operator()(
        string_view lhs,
        string_view rhs) const noexcept
    {
        return lhs == rhs;
    }
};

//----------------------------------------------------------

class object::pointer
{
    reference t_;

public:
    pointer(reference t)
        : t_(t)
    {
    }

    reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//----------------------------------------------------------

class object::const_pointer
{
    const_reference t_;

public:
    const_pointer(
        const_reference t)
        : t_(t)
    {
    }

    const_reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//----------------------------------------------------------

class object::const_iterator
{
    element* e_ = nullptr;

    friend class object;

    const_iterator(element* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;
    using pointer = object::const_pointer;
    using reference = object::const_reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator& operator=(const_iterator const&) = default;

    BOOST_JSON_DECL
    const_iterator(iterator it) noexcept;

    const_iterator&
    operator++() noexcept
    {
        e_ = e_->next;
        return *this;
    }

    const_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    const_iterator&
    operator--() noexcept
    {
        e_ = e_->prev;
        return *this;
    }

    const_iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    pointer
    operator->() const noexcept
    {
        return const_reference{
            e_->key(), e_->v };
    }

    reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v };
    }

    bool
    operator==(const_iterator other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(const_iterator other) const noexcept
    {
        return e_ != other.e_;
    }
};

//----------------------------------------------------------

class object::iterator
{
    element* e_ = nullptr;

    friend class object;

    iterator(element* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;
    using pointer = object::pointer;
    using reference = object::reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    iterator() = default;
    iterator(iterator const&) = default;
    iterator& operator=(iterator const&) = default;

    iterator&
    operator++() noexcept
    {
        e_ = e_->next;
        return *this;
    }

    iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    iterator&
    operator--() noexcept
    {
        e_ = e_->prev;
        return *this;
    }

    iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    pointer
    operator->() const noexcept
    {
        return reference{
            e_->key(), e_->v };
    }

    reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v };
    }

    bool
    operator==(const_iterator other) const noexcept
    {
        return const_iterator(*this) == other;
    }

    bool
    operator!=(const_iterator other) const noexcept
    {
        return const_iterator(*this) != other;
    }
};

//----------------------------------------------------------
//
// Iterators
//
//----------------------------------------------------------

auto
object::
begin() noexcept ->
    iterator
{
    if(! tab_)
        return {};
    return tab_->head;
}

auto
object::
begin() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->head;
}

auto
object::
cbegin() const noexcept ->
    const_iterator
{
    return begin();
}

auto
object::
end() noexcept ->
    iterator
{
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
end() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
cend() const noexcept ->
    const_iterator
{
    return end();
}

//----------------------------------------------------------
//
// Capacity
//
//----------------------------------------------------------

bool
object::
empty() const noexcept
{
    return ! tab_ ||
        tab_->size == 0;
}

auto
object::
size() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->size;
}

auto
object::
max_size() const noexcept ->
    size_type
{
    return (std::numeric_limits<
        size_type>::max)();
}

auto
object::
capacity() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return static_cast<
        size_type>(std::ceil(
            tab_->bucket_count *
            max_load_factor()));
}

void
object::
reserve(size_type n)
{
    rehash(static_cast<
        size_type>(std::ceil(
            n / max_load_factor())));
}

//----------------------------------------------------------
//
// Observers
//
//----------------------------------------------------------

auto
object::
hash_function() const noexcept ->
    hasher
{
    return hasher{};
}

auto
object::
key_eq() const noexcept ->
    key_equal
{
    return key_equal{};
}

//----------------------------------------------------------

template<class InputIt, class>
object::
object(
    InputIt first,
    InputIt last,
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    insert_range(end(),
        first, last, count);
}

//----------------------------------------------------------

template<class P, class>
auto
object::
insert(P&& p)->
    std::pair<iterator, bool>
{
    return insert(
        end(), std::forward<P>(p));
}

template<class P, class>
auto
object::
insert(
    const_iterator pos, P&& p) ->
        std::pair<iterator, bool>
{
    // VFALCO We can do better here,
    // by constructing `v` with the
    // get_storage().
    value_type v(std::forward<P>(p));
    return emplace(pos, v.first,
        std::move(v.second));
}

template<class InputIt, class>
void
object::
insert(InputIt first, InputIt last)
{
    insert_range(end(), first, last, 0);
}

template<class InputIt, class>
void
object::
insert(
    const_iterator pos,
    InputIt first,
    InputIt last)
{
    insert_range(pos, first, last, 0);
}

template<class M>
auto
object::
insert_or_assign(
    key_type key, M&& obj) ->
        std::pair<iterator, bool>
{
    return insert_or_assign(end(), key,
        std::forward<M>(obj));
}

template<class M>
auto
object::
insert_or_assign(
    const_iterator pos,
    key_type key,
    M&& obj) ->
        std::pair<iterator, bool>
{
    auto const result =
        find_impl(key);
    if(result.first)
    {
        result.first->v = std::forward<M>(obj);
        return { iterator(result.first), false };
    }
    auto const e = allocate(
        key, std::forward<M>(obj));
    insert(pos, result.second, e);
    return { iterator(e), true };
}

template<class Arg>
auto
object::
emplace(
    key_type key,
    Arg&& arg) ->
        std::pair<iterator, bool>
{
    return emplace(
        end(),
        key,
        std::forward<Arg>(arg));
}

template<class Arg>
auto
object::
emplace(
    const_iterator pos,
    key_type key,
    Arg&& arg) ->
        std::pair<iterator, bool>
{
    auto const result =
        find_impl(key);
    if(result.first)
        return { iterator(
            result.first), false };
    auto const e = allocate(
        key, std::forward<Arg>(arg));
    insert(pos, result.second, e);
    return { iterator(e), true };
}

//----------------------------------------------------------

// type-erased constructor to
// reduce template instantiations.
struct object::construct_base
{
    virtual
    ~construct_base() = default;

    virtual
    void
    operator()(void* p) const = 0;
};

template<class Arg>
auto
object::
allocate(key_type key, Arg&& arg) ->
    element*
{
    struct place : construct_base
    {
        Arg&& arg;
        storage_ptr const& sp;
        
        place(
            Arg&& arg_,
            storage_ptr const& sp_) noexcept
            : arg(std::forward<Arg>(arg_))
            , sp(sp_)
        {
        }

        void
        operator()(void* p) const override
        {
            ::new(p) element(
                std::forward<Arg>(arg), sp);
        }
    };

    return allocate_impl(key, place(
        std::forward<Arg>(arg), sp_));
}

template<class InputIt>
void
object::
insert_range(
    const_iterator pos,
    InputIt first,
    InputIt last,
    size_type count)
{
    undo_range u(*this);
    while(first != last)
        u.insert(allocate(*first++));
    u.commit(pos, count);
}

} // json
} // boost

#endif
