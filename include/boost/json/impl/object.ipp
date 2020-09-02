//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_IPP
#define BOOST_JSON_IMPL_OBJECT_IPP

#include <boost/json/object.hpp>
#include <boost/json/detail/except.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <type_traits>

BOOST_JSON_NS_BEGIN

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
                    self_.impl_.bucket(it->key()), *it);
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
    if(uo.storage().is_not_counted_and_deallocate_is_trivial())
        impl_.template build<false>(std::move(uo));
    else
        impl_.template build<true>(std::move(uo));
}

object::
object(object_test const*)
{
    object_impl impl(3, 1, 0, sp_);
    impl_.swap(impl);
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
        string_view, value_ref>> init,
    std::size_t min_capacity,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(this);
    using FwdIt = std::pair<
        string_view, value_ref> const*;
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
        string_view, value_ref>> init)
{
    object tmp(init, sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

auto
object::
get_allocator() const noexcept ->
    allocator_type
{
    return sp_.get();
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
        string_view, value_ref>> init)
{
    using FwdIt = std::pair<
        string_view, value_ref> const*;
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
        detail::throw_length_error(
            "object too large",
            BOOST_CURRENT_LOCATION);
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
        impl_.bucket(p->key()), *p);
    p->~value_type();
    impl_.shrink(1);
    if(p != impl_.end())
    {
        auto pb = impl_.end();
        auto& head =
            impl_.bucket(pb->key());
        impl_.remove(head, *pb);
        // the casts silence warnings
        std::memcpy(
            static_cast<void*>(p),
            static_cast<void const*>(pb),
            sizeof(*p));
        impl_.next(*p) = head;
        head = impl_.index_of(*p);
    }
    return p;
}

auto
object::
erase(string_view key) noexcept ->
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
at(string_view key) ->
    value&
{
    auto it = find(key);
    if(it == end())
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return it->value();
}
    
auto
object::
at(string_view key) const ->
    value const&
{
    auto it = find(key);
    if(it == end())
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return it->value();
}

auto
object::
operator[](string_view key) ->
    value&
{
    auto const result =
        emplace(key, nullptr);
    return result.first->value();
}

auto
object::
count(string_view key) const noexcept ->
    std::size_t
{
    if(find(key) == end())
        return 0;
    return 1;
}

auto
object::
find(string_view key) noexcept ->
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
find(string_view key) const noexcept ->
    const_iterator
{
    auto const p =
        find_impl(key).first;
    if(p)
        return p;
    return end();
}

value const*
object::
contains(string_view key) const noexcept
{
    auto const it = find(key);
    if(it != end())
        return &it->value();
    return nullptr;
}

value*
object::
contains(string_view key) noexcept
{
    auto const it = find(key);
    if(it != end())
        return &it->value();
    return nullptr;
}

//----------------------------------------------------------
//
// (implementation)
//
//----------------------------------------------------------

auto
object::
find_impl(string_view key) const noexcept ->
    std::pair<value_type*, std::size_t>
{
    std::pair<
        value_type*,
        std::size_t> result;
    result.second = impl_.digest(key);
    if(empty())
    {
        result.first = nullptr;
        return result;
    }
    auto const& head =
        impl_.bucket(result.second);
    auto i = head;
    while(i != null_index &&
        impl_.get(i).key() != key)
        i = impl_.next(impl_.get(i));
    if(i != null_index)
        result.first = &impl_.get(i);
    else
        result.first = nullptr;
    return result;
}

// rehash to at least `n` buckets
void
object::
rehash(std::size_t new_capacity)
{
    BOOST_ASSERT(new_capacity > capacity());
    const unsigned long long* prime = 
        object_impl::bucket_sizes();
    while(new_capacity > *prime)
        ++prime;
    new_capacity = *prime;
    if(new_capacity > max_size())
        detail::throw_length_error(
            "object too large",
            BOOST_CURRENT_LOCATION);
    object_impl impl(
        new_capacity,
        prime - object_impl::bucket_sizes(),
        impl_.salt(),
        sp_);
    if(impl_.size() > 0)
        std::memcpy(
            static_cast<void*>(impl.begin()),
            static_cast<void const*>(impl_.begin()),
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
    string_view key,
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
    impl_.next(e) = head;
    head = impl_.index_of(e);
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
    impl_.next(e) = head;
    head = impl_.index_of(e);
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
    impl_.next(e) = head;
    head = impl_.index_of(e);
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
        for(auto i = head;;
            i = impl_.next(impl_.get(i)))
        {
            if(i != null_index)
            {
                if(impl_.get(i).key() != e.key())
                    continue;
                e.~value_type();
            }
            else
            {
                impl_.next(e) = head;
                head = impl_.index_of(e);
                ++u.last;
            }
            break;
        }
    }
    u.commit = true;
}

BOOST_JSON_NS_END

#endif
