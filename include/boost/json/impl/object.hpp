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
#include <boost/json/detail/except.hpp>
#include <iterator>
#include <cmath>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

unchecked_object::
~unchecked_object()
{
    if(data_)
        object::value_type::destroy(
            data_, size_);
}

void
unchecked_object::
relocate(object::value_type* dest) noexcept
{
    if(size_ > 0)
        std::memcpy(
            reinterpret_cast<void*>(dest),
            data_, size_ * sizeof(object::value_type));
    data_ = nullptr;
}

//----------------------------------------------------------

void
object::
object_impl::
remove(
    value_type*& head,
    value_type* p) noexcept
{
    if(head == p)
    {
        head = head->next_;
        return;
    }
    auto prev = head;
    while(prev->next_ != p)
        prev = prev->next_;
    prev->next_ = p->next_;
}

auto
object::
object_impl::
bucket(string_view key) const noexcept ->
    value_type*&
{
    auto const hash = digest(key);
    auto const i = hash % buckets();
    return bucket_begin()[i];
}

auto
object::
object_impl::
bucket(std::size_t hash) const noexcept ->
    value_type*&
{
    return bucket_begin()[hash % buckets()];
}

auto
object::
object_impl::
begin() const noexcept ->
    value_type*
{
    if(! tab_)
        return nullptr;
    return reinterpret_cast<
        value_type*>(tab_ + 1);
}

auto
object::
object_impl::
end() const noexcept ->
    value_type*
{
    return begin() + size();
}

auto
object::
object_impl::
bucket_begin() const noexcept ->
    value_type**
{
    return reinterpret_cast<
        value_type**>(
            begin() + capacity());
}

//----------------------------------------------------------

struct object::undo_construct
{
    object* self;

    explicit
    undo_construct(
        object* self_)
        : self(self_)
    {
    }

    ~undo_construct()
    {
        if(self)
            self->impl_.destroy(
                self->sp_);
    }
};

//----------------------------------------------------------

struct object::place_one
{
    virtual
    void
    operator()(void* dest) = 0;
};

struct object::place_range
{
    virtual
    bool
    operator()(void* dest) = 0;
};

//----------------------------------------------------------
//
// object
//
//----------------------------------------------------------

template<class InputIt, class>
object::
object(
    InputIt first,
    InputIt last,
    std::size_t min_capacity,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(this);
    insert_range(
        first, last,
        min_capacity);
    u.self = nullptr;
}

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
    return impl_.begin();
}

auto
object::
begin() const noexcept ->
    const_iterator
{
    return impl_.begin();
}

auto
object::
cbegin() const noexcept ->
    const_iterator
{
    return impl_.begin();
}

auto
object::
end() noexcept ->
    iterator
{
    return impl_.end();
}

auto
object::
end() const noexcept ->
    const_iterator
{
    return impl_.end();
}

auto
object::
cend() const noexcept ->
    const_iterator
{
    return impl_.end();
}

auto
object::
rbegin() noexcept ->
    reverse_iterator
{
    return reverse_iterator(end());
}

auto
object::
rbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(end());
}

auto
object::
crbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(end());
}

auto
object::
rend() noexcept ->
    reverse_iterator
{
    return reverse_iterator(begin());
}

auto
object::
rend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(begin());
}

auto
object::
crend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(begin());
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
    return impl_.size() == 0;
}

auto
object::
size() const noexcept ->
    std::size_t
{
    return impl_.size();
}

auto
object::
capacity() const noexcept ->
    std::size_t
{
    return impl_.capacity();
}

void
object::
reserve(std::size_t new_capacity)
{
    if(new_capacity <= capacity())
        return;
    rehash(new_capacity);
}

//----------------------------------------------------------

template<class P, class>
auto
object::
insert(P&& p) ->
    std::pair<iterator, bool>
{
    struct place_impl : place_one
    {
        P&& p;
        storage_ptr const& sp;

        place_impl(
            P&& p_,
            storage_ptr const& sp_)
            : p(std::forward<P>(p_))
            , sp(sp_)
        {
        }

        void
        operator()(void* dest) override
        {
            ::new(dest) value_type(
                std::forward<P>(p), sp);
        }
    };

    place_impl f(
        std::forward<P>(p), sp_);
    return insert_impl(f);
}

template<class M>
auto
object::
insert_or_assign(
    key_type key, M&& m) ->
        std::pair<iterator, bool>
{
    struct place_impl : place_one
    {
        key_type key;
        M&& m;
        storage_ptr const& sp;

        place_impl(
            key_type key_,
            M&& m_,
            storage_ptr const& sp_)
            : key(key_)
            , m(std::forward<M>(m_))
            , sp(sp_)
        {
        }

        void
        operator()(void* dest) override
        {
            ::new(dest) value_type(key,
                std::forward<M>(m), sp);
        }
    };

    auto const result = find_impl(key);
    if(result.first)
    {
        result.first->value() =
            std::forward<M>(m);
        return { result.first, false };
    }

    place_impl f(key,
        std::forward<M>(m), sp_);
    return { insert_impl(
        result.second, f), true };
}

template<class Arg>
auto
object::
emplace(
    key_type key,
    Arg&& arg) ->
        std::pair<iterator, bool>
{
    struct place_impl : place_one
    {
        key_type key;
        Arg&& arg;
        storage_ptr const& sp;
    
        place_impl(
            key_type key_,
            Arg&& arg_,
            storage_ptr const& sp_)
            : key(key_)
            , arg(std::forward<Arg>(arg_))
            , sp(sp_)
        {
        }

        void
        operator()(void* dest) override
        {
            ::new(dest) value_type(key,
                std::forward<Arg>(arg), sp);
        }
    };

    place_impl f(key,
        std::forward<Arg>(arg), sp_);
    return emplace_impl(key, f);
}

//----------------------------------------------------------

inline
void
swap(object& lhs, object& rhs)
{
    lhs.swap(rhs);
}

//----------------------------------------------------------
//
// (implementation)
//
//----------------------------------------------------------

std::uint32_t
object::
digest(
    key_type key,
    std::false_type) noexcept
{
    std::uint32_t prime = 0x01000193UL;
    std::uint32_t hash  = 0x811C9DC5UL;
    for(auto p = key.begin(),
        end = key.end(); p != end; ++p)
        hash = (*p ^ hash) * prime;
    return hash;
}

std::uint64_t
object::
digest(
    key_type key,
    std::true_type) noexcept
{
    std::uint64_t prime = 0x100000001B3ULL;
    std::uint64_t hash  = 0xcbf29ce484222325ULL;
    for(auto p = key.begin(),
        end = key.end(); p != end; ++p)
        hash = (*p ^ hash) * prime;
    return hash;
}

std::size_t
object::
digest(key_type key) noexcept
{
    return digest(key,
        std::integral_constant<bool,
            sizeof(std::size_t) ==
            sizeof(std::uint64_t)>{});
}

template<class InputIt>
void
object::
insert_range(
    InputIt first,
    InputIt last,
    std::size_t min_capacity,
    std::input_iterator_tag)
{
    // Since input iterators cannot be rewound,
    // we keep inserted elements on an exception.
    //
    reserve(min_capacity);
    while(first != last)
    {
        insert(*first);
        ++first;
    }
}

template<class InputIt>
void
object::
insert_range(
    InputIt first,
    InputIt last,
    std::size_t min_capacity,
    std::forward_iterator_tag)
{
    struct place_impl : place_range
    {
        InputIt it;
        std::size_t n;
        storage_ptr const& sp;

        place_impl(
            InputIt it_,
            std::size_t n_,
            storage_ptr const& sp_)
            : it(it_)
            , n(n_)
            , sp(sp_)
        {
        }

        bool
        operator()(void* dest) override
        {
            if(n-- == 0)
                return false;
            ::new(dest) value_type(*it++, sp);
            return true;
        }
    };

    auto const n = static_cast<std::size_t>(
        std::distance(first, last));
    auto const n0 = size();
    if(n > max_size() - n0)
        BOOST_JSON_THROW(
            detail::object_too_large_exception());
    if( min_capacity < n0 + n)
        min_capacity = n0 + n;
    place_impl f(first, n, sp_);
    insert_range_impl(min_capacity, f);
}

} // json
} // boost

#endif
