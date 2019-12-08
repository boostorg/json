//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_IPP
#define BOOST_JSON_IMPL_OBJECT_IPP

#include <boost/json/object.hpp>
#include <boost/json/detail/except.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace boost {
namespace json {

class object::undo_insert
{
    object& self_;

public:
    std::size_t const first;
    std::size_t last;
    bool commit = false;

    ~undo_insert()
    {
        if(commit)
        {
            self_.impl_.grow(last - first);
        }
        else
        {
            auto p0 = self_.impl_.begin() + first;
            auto p1 = self_.impl_.begin() + last;
            for(auto it = p0; it != p1; ++it)
                self_.impl_.remove(
                    self_.impl_.bucket(it->key()), it);
            detail::destroy(p0, last - first);
        }
    }

    explicit
    undo_insert(
        object& self) noexcept
        : self_(self)
        , first(
            self_.impl_.end() -
            self_.impl_.begin())
        , last(first)
    {
    }

    value_type*
    pos() noexcept
    {
        return self_.begin() + last;
    }
};

//----------------------------------------------------------
//
// object
//
//----------------------------------------------------------

object::
object(detail::unchecked_object&& uo)
    : sp_(uo.storage())
{
    reserve(uo.size());
    uo.relocate(impl_.begin());
    impl_.grow(uo.size());
    impl_.build();
}

object::
object(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
}

object::
object(
    std::size_t min_capacity,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    reserve(min_capacity);
}

object::
object(object&& other) noexcept
    : sp_(other.sp_)
    , impl_(std::move(other.impl_))
{
}

object::
object(
    object&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    if(*sp_ == *other.sp_)
    {
        impl_.swap(other.impl_);
    }
    else
    {
        undo_construct u(this);
        insert_range(
            other.begin(),
            other.end(), 0);
        u.self = nullptr;
    }
}

object::
object(pilfered<object> other) noexcept
    : sp_(std::move(other.get().sp_))
    , impl_(std::move(other.get().impl_))
{
}

object::
object(
    object const& other)
    : sp_(other.sp_)
{
    undo_construct u(this);
    insert_range(
        other.begin(),
        other.end(), 0);
    u.self = nullptr;
}

object::
object(
    object const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(this);
    insert_range(
        other.begin(),
        other.end(), 0);
    u.self = nullptr;
}

object::
object(
    std::initializer_list<std::pair<
        key_type, value_ref>> init,
    std::size_t min_capacity,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(this);
    using FwdIt = std::pair<
        key_type, value_ref> const*;
    struct place_impl : place_range
    {
        FwdIt it;
        std::size_t n;
        storage_ptr const& sp;

        place_impl(
            FwdIt it_,
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
            ::new(dest) value_type(
                it->first,
                it->second.make_value(sp));
            ++it;
            return true;
        }
    };
    if( min_capacity < init.size())
        min_capacity = init.size();
    place_impl f(
        init.begin(), init.size(), sp_);
    insert_range_impl(min_capacity, f);
    u.self = nullptr;
}

object&
object::
operator=(object&& other)
{
    object tmp(std::move(other), sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

object&
object::
operator=(object const& other)
{
    if(this == &other)
        return *this;
    object tmp(other, sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

object&
object::
operator=(
    std::initializer_list<std::pair<
        key_type, value_ref>> init)
{
    object tmp(init, sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

void
object::
clear() noexcept
{
    impl_.clear();
}

void
object::
insert(
    std::initializer_list<std::pair<
        key_type, value_ref>> init)
{
    using FwdIt = std::pair<
        key_type, value_ref> const*;
    struct place_impl : place_range
    {
        FwdIt it;
        std::size_t n;
        storage_ptr const& sp;

        place_impl(
            FwdIt it_,
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
            ::new(dest) value_type(
                it->first,
                it->second.make_value(sp));
            ++it;
            return true;
        }
    };
    auto const n0 = size();
    if(init.size() > max_size() - n0)
        BOOST_JSON_THROW(
            detail::object_too_large_exception());
    place_impl f(
        init.begin(), init.size(), sp_);
    insert_range_impl(n0 + init.size(), f);
}

auto
object::
erase(const_iterator pos) noexcept ->
    iterator
{
    auto p = impl_.begin() +
        (pos - impl_.begin());
    impl_.remove(
        impl_.bucket(p->key()), p);
    p->~value_type();
    impl_.shrink(1);
    if(p != impl_.end())
    {
        auto pb = impl_.end();
        auto& head =
            impl_.bucket(pb->key());
        impl_.remove(head, pb);
        std::memcpy(
            reinterpret_cast<void*>(p),
            reinterpret_cast<void const*>(pb),
            sizeof(*p));
        next(*p) = head;
        head = p;
    }
    return p;
}

auto
object::
erase(key_type key) noexcept ->
    std::size_t
{
    auto it = find(key);
    if(it == end())
        return 0;
    erase(it);
    return 1;
}

void
object::
swap(object& other)
{
    BOOST_ASSERT(this != &other);
    if(*sp_ == *other.sp_)
    {
        impl_.swap(other.impl_);
        return;
    }
    object temp1(
        std::move(*this),
        other.storage());
    object temp2(
        std::move(other),
        this->storage());
    other.~object();
    ::new(&other) object(pilfer(temp1));
    this->~object();
    ::new(this) object(pilfer(temp2));
}

//----------------------------------------------------------
//
// Lookup
//
//----------------------------------------------------------

auto
object::
at(key_type key) ->
    value&
{
    auto it = find(key);
    if(it == end())
        BOOST_JSON_THROW(
         std::out_of_range(
            "key not found"));
    return it->value();
}
    
auto
object::
at(key_type key) const ->
    value const&
{
    auto it = find(key);
    if(it == end())
        BOOST_JSON_THROW(
         std::out_of_range(
            "key not found"));
    return it->value();
}

auto
object::
operator[](key_type key) ->
    value&
{
    auto const result =
        emplace(key, nullptr);
    return result.first->value();
}

auto
object::
count(key_type key) const noexcept ->
    std::size_t
{
    if(find(key) == end())
        return 0;
    return 1;
}

auto
object::
find(key_type key) noexcept ->
    iterator
{
    auto const p =
        find_impl(key).first;
    if(p)
        return p;
    return end();
}

auto
object::
find(key_type key) const noexcept ->
    const_iterator
{
    auto const p =
        find_impl(key).first;
    if(p)
        return p;
    return end();
}

bool
object::
contains(key_type key) const noexcept
{
    return find(key) != end();
}

//----------------------------------------------------------
//
// (implementation)
//
//----------------------------------------------------------

auto
object::
find_impl(key_type key) const noexcept ->
    std::pair<value_type*, std::size_t>
{
    std::pair<
        value_type*,
        std::size_t> result;
    result.second =
        object_impl::digest(key);
    if(empty())
    {
        result.first = nullptr;
        return result;
    }
    auto const& head =
        impl_.bucket(result.second);
    auto it = head;
    while(it && it->key() != key)
        it = next(*it);
    result.first = it;
    return result;
}

// rehash to at least `n` buckets
void
object::
rehash(std::size_t new_capacity)
{
    auto const next_prime =
    [](std::size_t n) noexcept
    {
        // Taken from Boost.Intrusive and Boost.MultiIndex code,
        // thanks to Ion Gaztanaga and Joaquin M Lopez Munoz.
        static unsigned long long constexpr list[] = {
            0ULL,

            3ULL,                     7ULL,
            11ULL,                    17ULL,
            29ULL,                    53ULL,
            97ULL,                    193ULL,
            389ULL,                   769ULL,
            1543ULL,                  3079ULL,
            6151ULL,                  12289ULL,
            24593ULL,                 49157ULL,
            98317ULL,                 196613ULL,
            393241ULL,                786433ULL,
            1572869ULL,               3145739ULL,
            6291469ULL,               12582917ULL,
            25165843ULL,              50331653ULL,
            100663319ULL,             201326611ULL,
            402653189ULL,             805306457ULL,
            1610612741ULL,            3221225473ULL,

            6442450939ULL,            12884901893ULL,
            25769803751ULL,           51539607551ULL,
            103079215111ULL,          206158430209ULL,
            412316860441ULL,          824633720831ULL,
            1649267441651ULL,         3298534883309ULL,
            6597069766657ULL,         13194139533299ULL,
            26388279066623ULL,        52776558133303ULL,
            105553116266489ULL,       211106232532969ULL,
            422212465066001ULL,       844424930131963ULL,
            1688849860263953ULL,      3377699720527861ULL,
            6755399441055731ULL,      13510798882111483ULL,
            27021597764222939ULL,     54043195528445957ULL,
            108086391056891903ULL,    216172782113783843ULL,
            432345564227567621ULL,    864691128455135207ULL,
            1729382256910270481ULL,   3458764513820540933ULL,
            6917529027641081903ULL,   13835058055282163729ULL,
            18446744073709551557ULL,  18446744073709551615ULL
        };
        return static_cast<std::size_t>(
            *std::lower_bound(
                &list[0],
                &list[std::extent<
                    decltype(list)>::value],
                (unsigned long long)n));
    };
    BOOST_ASSERT(new_capacity > capacity());
    auto const f = std::ceil(
        new_capacity / max_load_factor());
    BOOST_ASSERT(
        f < static_cast<std::size_t>(-1));
    auto const new_buckets = next_prime(
        static_cast<std::size_t>(f));
    BOOST_ASSERT(std::ceil(
        new_buckets * max_load_factor()) >=
            new_capacity);
    new_capacity = static_cast<std::size_t>(
        std::ceil(new_buckets * max_load_factor()));
    if(new_capacity > max_size())
        BOOST_JSON_THROW(
            detail::object_too_large_exception());
    object_impl impl(
        new_capacity, new_buckets, sp_);
    if(impl_.size() > 0)
        std::memcpy(
            reinterpret_cast<void*>(impl.begin()),
            reinterpret_cast<void const*>(impl_.begin()),
            impl_.size() * sizeof(value_type));
    impl.grow(impl_.size());
    impl_.shrink(impl_.size());
    impl_.destroy(sp_);
    impl_.swap(impl);
    impl_.rebuild();
}

auto
object::
emplace_impl(
    key_type key,
    place_one& f) ->
    std::pair<iterator, bool>
{
    auto const result = find_impl(key);
    if(result.first)
        return { result.first, false };
    reserve(size() + 1);
    auto& e = *impl_.end();
    f(&e);
    auto& head =
        impl_.bucket(result.second);
    next(e) = head;
    head = &e;
    impl_.grow(1);
    return { &e, true };
}

auto
object::
insert_impl(
    place_one& f) ->
    std::pair<iterator, bool>
{
    reserve(size() + 1);
    auto& e = *impl_.end();
    f(&e);
    auto const result =
        find_impl(e.key());
    if(result.first)
    {
        e.~value_type();
        return { result.first, false };
    }
    auto& head =
        impl_.bucket(result.second);
    next(e) = head;
    head = &e;
    impl_.grow(1);
    return { &e, true };
}

auto
object::
insert_impl(
    std::size_t hash,
    place_one& f) ->
        iterator
{
    reserve(size() + 1);
    auto& e = *impl_.end();
    f(&e);
    auto& head =
        impl_.bucket(hash);
    next(e) = head;
    head = &e;
    impl_.grow(1);
    return &e;
}

void
object::
insert_range_impl(
    std::size_t min_capacity,
    place_range& f)
{
    reserve(min_capacity);
    undo_insert u(*this);
    for(;;)
    {
        reserve(size() + 1);
        auto& e = *u.pos();
        if(! f(&e))
            break;
        auto& head =
            impl_.bucket(e.key());
        for(auto it = head;;
            it = next(*it))
        {
            if(it)
            {
                if(it->key() != e.key())
                    continue;
                e.~value_type();
            }
            else
            {
                next(e) = head;
                head = &e;
                ++u.last;
            }
            break;
        }
    }
    u.commit = true;
}

} // json
} // boost

#endif
